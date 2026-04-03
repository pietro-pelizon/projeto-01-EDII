#define NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linked_list.h"
#include "../include/e_hashing.h"

#include <math.h>

typedef struct __attribute__((packed)) StBucket {
    uint16_t local_depth;
    uint16_t record_count;
} Bucket;

typedef struct  __attribute__((packed)) StEHashing {
    FILE *file;
    uint64_t *directory;
    uint32_t record_size;
    uint32_t bucket_size;
    uint8_t global_depth;
} eHashing;

typedef struct __attribute__((packed)) stFileHeader{
    uint8_t global_depth;
    uint32_t bucket_size;
    uint32_t record_size;
    uint32_t directory_offset;

} FileHeader;


#include <stdint.h>

// Função de hash alfanumérica
static uint64_t murmurhash3_64(const void *key, int len, uint32_t seed) {
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t *data = (const uint64_t *)key;
    const uint64_t *end = data + (len / 8);

    while (data != end) {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = (const unsigned char *)data;
    switch (len & 7) {
        case 7: h ^= (uint64_t)data2[6] << 48;
        case 6: h ^= (uint64_t)data2[5] << 40;
        case 5: h ^= (uint64_t)data2[4] << 32;
        case 4: h ^= (uint64_t)data2[3] << 24;
        case 3: h ^= (uint64_t)data2[2] << 16;
        case 2: h ^= (uint64_t)data2[1] << 8;
        case 1: h ^= (uint64_t)data2[0];
            h *= m;
            break;
        default:
            break;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

// Escreve um balde vazio em um determinado offset
static void write_empty_bucket(FILE *file, uint32_t bucket_size, uint16_t depth) {
    Bucket bh = { .local_depth = depth, .record_count = 0 };
    fwrite(&bh, sizeof(Bucket), 1, file);

    // Preenche o restante do espaço do balde com zeros
    char *padding = calloc(1, bucket_size - sizeof(Bucket));
    fwrite(padding, 1, bucket_size - sizeof(Bucket), file);
    free(padding);
}

static void update_file_header(eHashing *map) {
    FileHeader header = {
        .global_depth = map -> global_depth,
        .bucket_size = map -> bucket_size,
        .record_size = map -> record_size,
        .directory_offset = sizeof(FileHeader)
    };

    fseek(map -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(FileHeader), 1, map -> file);
}

static void init_new_hashfile(eHashing *map) {
    map -> global_depth = 0;
    map -> directory = malloc(sizeof(uint64_t) * 1);

    // O primeiro balde ficará após o Header + 1 entrada de diretório
    long first_bucket_offset = sizeof(FileHeader) + sizeof(uint64_t);
    map -> directory[0] = (uint64_t)first_bucket_offset;

    // Persiste a estrutura inicial
    update_file_header(map);

    fseek(map -> file, sizeof(FileHeader), SEEK_SET);
    fwrite(map -> directory, sizeof(uint64_t), 1, map -> file);

    fseek(map->file, first_bucket_offset, SEEK_SET);
    write_empty_bucket(map -> file, map -> bucket_size, 0);
}

static void load_existing_hashfile(eHashing *map) {
    FileHeader header;
    fseek(map -> file, 0, SEEK_SET);
    fread(&header, sizeof(FileHeader), 1, map -> file);

    // Sincroniza a RAM com as definições do disco
    map -> global_depth = header.global_depth;
    map -> bucket_size = header.bucket_size;
    map -> record_size = header.record_size;

    uint32_t num_entries = 1 << map -> global_depth;
    map -> directory = malloc(sizeof(uint64_t) * num_entries);

    fseek(map -> file, (long)header.directory_offset, SEEK_SET);
    fread(map -> directory, sizeof(uint64_t), num_entries, map -> file);
}

eHashing *e_hashing_init(const char *filename, uint32_t record_size, uint32_t bucket_size) {
    eHashing *map = malloc(sizeof(eHashing));
    if (!map) return NULL;

    map -> record_size = record_size;
    map -> bucket_size = bucket_size;
    map -> file = fopen(filename, "r+b");

    if (map -> file == NULL) {
        map -> file = fopen(filename, "w+b");

        if (!map -> file) { free(map); return NULL; }

        init_new_hashfile(map);
    } else {
        load_existing_hashfile(map);
    }

    return map;
}

uint64_t e_hashing_capacity(eHashing *map) {

    uint64_t C = map -> bucket_size - sizeof(bucket);
    uint64_t Q = sizeof(uint64_t) + map -> record_size;

    return C/Q;
}

bool e_hashing_search(eHashing *map, uint64_t key, void *out_data) {
    assert(map != NULL);

    uint64_t hashed_key = murmurhash3_64(&key, sizeof(uint64_t), 0);
    uint64_t idx = hashed_key & ((1 << map -> global_depth) - 1);
    uint64_t offset = map -> directory[idx];

    Bucket header;
    fseek(map -> file, (long)offset, SEEK_SET);
    fread(&header, sizeof(Bucket), 1, map -> file);

    uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

    for (uint16_t i = 0; i < header.record_count; i++) {
        uint64_t slot_key;
        uint32_t slot_offset = (long)offset + sizeof(Bucket) + (i * slot_size);

        fseek(map -> file, slot_offset, SEEK_SET);
        fread(&slot_key, sizeof(uint64_t), 1, map -> file);

        if (slot_key == key) {
            fread(out_data, map -> record_size, 1, map -> file);
            return true;
        }
    }

    return false;
}

bool e_hashing_remove(eHashing *map, uint64_t key, void *out_data);

void e_hashing_destroy(eHashing *map) {
    if (map == NULL) return;

    // 1. Prepara o cabeçalho para salvar
    FileHeader header;
    header.global_depth = map -> global_depth;
    header.bucket_size = map -> bucket_size;
    header.directory_offset = (1 << map -> global_depth);

    fseek(map -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(FileHeader), 1, map -> file);

    fwrite(map -> directory, sizeof(uint64_t), header.directory_offset, map -> file);

    fclose(map -> file);
    free(map -> directory);
    free(map);

}

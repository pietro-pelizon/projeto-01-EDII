#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/exhash.h"

typedef struct __attribute__((packed)) stExhash {
    FILE     *file;
    uint64_t *directory;
    uint32_t  record_size;
    uint32_t  bucket_size;
    uint8_t   global_depth;
} exhash_t;

typedef struct __attribute__((packed)) stBucket {
    uint16_t local_depth;
    uint16_t record_count;
} bucket_t;

typedef struct __attribute__((packed)) stFile_header {
    uint8_t  global_depth;
    uint32_t bucket_size;
    uint32_t record_size;
    uint32_t directory_offset;
} file_header_t;

static bool     exhash_contains(const exhash_t *map, const char *key);
static uint64_t murmurhash3_64(const void *key, size_t len, uint32_t seed);
static uint64_t exhash_capacity(const exhash_t *map);
static void     write_empty_bucket(FILE *file, uint32_t bucket_size, uint16_t depth);
static void     update_file_header(const exhash_t *map);
static void     load_existing_exhash(exhash_t *map);
static void     init_new_exhash(exhash_t *map);
static bool     insert_raw_into_bucket(const exhash_t *map, uint64_t offset, bucket_t *b, uint64_t numeric_key, const void *data);
static bool     insert_into_bucket(const exhash_t *map, uint64_t offset, bucket_t *b, const void *key, const void *data);
static uint64_t get_directory_index(const exhash_t *hashfile, const char *key);
static bool     expand_directory(exhash_t *hashfile);
static void     redistribute_records(const exhash_t *hashfile, uint64_t old_offset, uint64_t new_offset, bucket_t *old_bucket, bucket_t *new_bucket, uint32_t old_count);
static void     update_directory_pointers(const exhash_t *hashfile, uint64_t old_offset, uint64_t new_offset, uint8_t depth);
static void     split_bucket(exhash_t *hashfile, uint64_t old_offset, bucket_t old_bucket);

static inline uint64_t dir_size(const exhash_t *hashfile) {
    return (uint64_t)1 << hashfile -> global_depth;
}

static inline uint64_t dir_mask(const exhash_t *hashfile) {
    return dir_size(hashfile) - 1;
}

// ============== FUNÇÕES PRINCIPAIS DO MÓDULO ==============

exhash_t *exhash_init(const char *filename, uint32_t record_size, uint32_t bucket_size) {
    exhash_t *map = malloc(sizeof(exhash_t));
    if (!map) return NULL;

    map -> record_size = record_size;
    map -> bucket_size = bucket_size;
    map -> file = fopen(filename, "r+b");

    if (map -> file == NULL) {
        map -> file = fopen(filename, "w+b");

        if (!map -> file) { free(map); return NULL; }

        init_new_exhash(map);
    }
    else {
        load_existing_exhash(map);
    }

    return map;
}

bool exhash_insert(exhash_t *hashfile, const void *data, const char *key) {
    assert(hashfile != NULL && data != NULL);

    if (exhash_contains(hashfile, key)) {
        return false;
    }

    while (true) {

        uint64_t idx = get_directory_index(hashfile, key);
        uint64_t offset = hashfile -> directory[idx];

        bucket_t directory_header;

        fseek(hashfile -> file, (long)offset, SEEK_SET);
        fread(&directory_header, sizeof(bucket_t), 1, hashfile -> file);

        if (directory_header.record_count < exhash_capacity(hashfile)) {
            return insert_into_bucket(hashfile, offset, &directory_header, key, data);
        }

        if (directory_header.local_depth == hashfile -> global_depth) {
            expand_directory(hashfile);
        }

        split_bucket(hashfile, offset, directory_header);
    }
}

bool exhash_search(const exhash_t *map, const char *key, void *out_data) {
    assert(map != NULL && key != NULL && out_data != NULL);

    uint64_t hashed_key = murmurhash3_64(key, strlen(key), 0);

    uint64_t idx = hashed_key & dir_mask(map);
    uint64_t offset = map -> directory[idx];

    bucket_t header;
    fseek(map -> file, (long)offset, SEEK_SET);
    fread(&header, sizeof(bucket_t), 1, map -> file);

    uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

    for (uint16_t i = 0; i < header.record_count; i++) {
        uint64_t slot_key;

        uint64_t slot_offset = offset + sizeof(bucket_t) + (i * slot_size);

        fseek(map -> file, (long)slot_offset, SEEK_SET);
        fread(&slot_key, sizeof(uint64_t), 1, map -> file);

        if (slot_key == hashed_key) {
            fread(out_data, map -> record_size, 1, map -> file);
            return true;
        }
    }

    return false;
}

void *exhash_remove(const exhash_t *map, const char *key) {
    if (map == NULL || key == NULL) return NULL;

    uint64_t hashed_key = murmurhash3_64(key, strlen(key), 0);
    uint64_t idx = hashed_key & dir_mask(map);
    uint64_t offset = map -> directory[idx];

    bucket_t header;
    fseek(map -> file, (long)offset, SEEK_SET);
    fread(&header, sizeof(bucket_t), 1, map -> file);

    uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

    for (uint16_t i = 0; i < header.record_count; i++) {
        uint64_t slot_key;
        uint64_t slot_offset = offset + sizeof(bucket_t) + (i * slot_size);

        fseek(map -> file, (long)slot_offset, SEEK_SET);
        fread(&slot_key, sizeof(uint64_t), 1, map -> file);

        if (slot_key != hashed_key) continue;

        void *removed_data = malloc(map -> record_size);
        fread(removed_data, map -> record_size, 1, map -> file);

        if (i < header.record_count - 1) {
            uint64_t last_slot_offset = offset + sizeof(bucket_t) + ((header.record_count - 1) * slot_size);

            uint64_t last_key;
            void *last_data = malloc(map -> record_size);

            fseek(map -> file, (long)last_slot_offset, SEEK_SET);
            fread(&last_key, sizeof(uint64_t), 1, map -> file);
            fread(last_data, map -> record_size, 1, map -> file);

            fseek(map -> file, (long)slot_offset, SEEK_SET);
            fwrite(&last_key, sizeof(uint64_t), 1, map -> file);
            fwrite(last_data, map -> record_size, 1, map -> file);

            free(last_data);
        }

        header.record_count--;
        fseek(map -> file, (long)offset, SEEK_SET);
        fwrite(&header, sizeof(bucket_t), 1, map -> file);

        return removed_data;
    }

    printf("Dado não encontrado no hashfile!\n");
    return NULL;
}

void **exhash_get_all(exhash_t *map, uint64_t *out_count) {
    if (map == NULL || out_count == NULL) return NULL;

    uint64_t capacidade = 128;
    void **resultados = malloc(capacidade * sizeof(void *));
    uint64_t total_encontrados = 0;

    uint64_t tamanho_diretorio = dir_size(map);
    uint64_t *baldes_visitados = malloc(tamanho_diretorio * sizeof(uint64_t));
    uint64_t qtd_visitados = 0;

    for (uint64_t i = 0; i < tamanho_diretorio; i++) {
        uint64_t offset_atual = map -> directory[i];

        bool ja_visitado = false;
        for (uint64_t v = 0; v < qtd_visitados; v++) {
            if (baldes_visitados[v] == offset_atual) {
                ja_visitado = true;
                break;
            }
        }

        if (ja_visitado) continue;

        baldes_visitados[qtd_visitados++] = offset_atual;

        bucket_t header;
        fseek(map -> file, (long)offset_atual, SEEK_SET);
        fread(&header, sizeof(bucket_t), 1, map -> file);

        uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

        for (uint16_t j = 0; j < header.record_count; j++) {
            uint64_t slot_offset = offset_atual + sizeof(bucket_t) + (j * slot_size) + sizeof(uint64_t);

            void *registro = malloc(map -> record_size);
            fseek(map -> file, (long)slot_offset, SEEK_SET);
            fread(registro, map -> record_size, 1, map -> file);

            if (total_encontrados == capacidade) {
                capacidade *= 2;
                resultados = realloc(resultados, capacidade * sizeof(void *));
            }

            resultados[total_encontrados++] = registro;
        }
    }

    free(baldes_visitados);
    *out_count = total_encontrados;
    return resultados;
}

void exhash_destroy(exhash_t *map) {
    if (map == NULL) return;

    fseek(map -> file, 0, SEEK_END);
    uint32_t dir_offset = (uint32_t)ftell(map -> file);

    uint32_t num_entries = (uint32_t)dir_size(map);
    fwrite(map -> directory, sizeof(uint64_t), num_entries, map -> file);

    file_header_t header = {
        .global_depth     = map -> global_depth,
        .bucket_size      = map -> bucket_size,
        .record_size      = map -> record_size,
        .directory_offset = dir_offset
    };

    fseek(map -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(file_header_t), 1, map -> file);

    fclose(map -> file);
    free(map -> directory);
    free(map);
}

// ============== FUNÇÕES STATIC UTILITÁRIAS ==============

static uint64_t murmurhash3_64(const void *key, size_t len, const uint32_t seed) {
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

static uint64_t exhash_capacity(const exhash_t *map) {
    uint64_t C = map -> bucket_size - sizeof(bucket_t);
    uint64_t Q = sizeof(uint64_t) + map -> record_size;
    return C / Q;
}

static void write_empty_bucket(FILE *file, uint32_t bucket_size, uint16_t depth) {
    bucket_t bh = { .local_depth = depth, .record_count = 0 };
    fwrite(&bh, sizeof(bucket_t), 1, file);

    char *padding = calloc(1, bucket_size - sizeof(bucket_t));
    fwrite(padding, 1, bucket_size - sizeof(bucket_t), file);
    free(padding);
}

static void update_file_header(const exhash_t *map) {
    file_header_t header = {
        .global_depth     = map -> global_depth,
        .bucket_size      = map -> bucket_size,
        .record_size      = map -> record_size,
        .directory_offset = sizeof(file_header_t)
    };

    fseek(map -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(file_header_t), 1, map -> file);
}

static void load_existing_exhash(exhash_t *map) {
    file_header_t header;
    fseek(map -> file, 0, SEEK_SET);
    fread(&header, sizeof(file_header_t), 1, map -> file);

    map -> global_depth = header.global_depth;
    map -> bucket_size  = header.bucket_size;
    map -> record_size  = header.record_size;

    uint32_t num_entries = (uint32_t)dir_size(map);
    map -> directory = malloc(sizeof(uint64_t) * num_entries);

    fseek(map -> file, (long)header.directory_offset, SEEK_SET);
    fread(map -> directory, sizeof(uint64_t), num_entries, map -> file);
}

static void init_new_exhash(exhash_t *map) {
    map -> global_depth = 0;
    map -> directory = malloc(sizeof(uint64_t));

    uint64_t first_bucket_offset = sizeof(file_header_t) + sizeof(uint64_t);
    map -> directory[0] = first_bucket_offset;

    file_header_t header = {
        .global_depth     = map -> global_depth,
        .bucket_size      = map -> bucket_size,
        .record_size      = map -> record_size,
        .directory_offset = sizeof(file_header_t)
    };

    fseek(map -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(file_header_t), 1, map -> file);
    fwrite(map -> directory, sizeof(uint64_t), 1, map -> file);

    fseek(map -> file, (long)first_bucket_offset, SEEK_SET);
    write_empty_bucket(map -> file, map -> bucket_size, 0);
}

static bool insert_raw_into_bucket(const exhash_t *map, uint64_t offset, bucket_t *b, uint64_t numeric_key, const void *data) {
    uint64_t slot_offset = offset + sizeof(bucket_t) +
                           (b -> record_count * (sizeof(uint64_t) + map -> record_size));

    fseek(map -> file, (long)slot_offset, SEEK_SET);
    fwrite(&numeric_key, sizeof(uint64_t), 1, map -> file);
    fwrite(data, map -> record_size, 1, map -> file);

    b -> record_count++;
    fseek(map -> file, (long)offset, SEEK_SET);
    fwrite(b, sizeof(bucket_t), 1, map -> file);

    return true;
}

static bool insert_into_bucket(const exhash_t *map, uint64_t offset, bucket_t *b, const void *key, const void *data) {
    uint64_t numeric_key = murmurhash3_64(key, strlen((const char *)key), 0);
    return insert_raw_into_bucket(map, offset, b, numeric_key, data);
}

static uint64_t get_directory_index(const exhash_t *hashfile, const char *key) {
    assert(hashfile != NULL && key != NULL);
    return murmurhash3_64(key, strlen(key), 0) & dir_mask(hashfile);
}

static bool expand_directory(exhash_t *hashfile) {
    uint64_t old_size = dir_size(hashfile);
    uint64_t new_size = old_size * 2;

    uint64_t *new_directory = realloc(hashfile -> directory, sizeof(uint64_t) * new_size);
    assert(new_directory != NULL);
    hashfile -> directory = new_directory;

    for (uint64_t i = 0; i < old_size; i++) {
        hashfile -> directory[i + old_size] = hashfile -> directory[i];
    }

    hashfile -> global_depth++;
    update_file_header(hashfile);

    return true;
}

static void redistribute_records(const exhash_t *hashfile, uint64_t old_offset, uint64_t new_offset, bucket_t *old_bucket, bucket_t *new_bucket, uint32_t old_count) {
    void *temp_data = malloc(hashfile -> record_size);
    assert(temp_data != NULL);

    for (uint32_t i = 0; i < old_count; i++) {
        uint64_t saved_key;
        uint64_t slot_offset = old_offset + sizeof(bucket_t) + (i * (sizeof(uint64_t) + hashfile -> record_size));

        fseek(hashfile -> file, (long)slot_offset, SEEK_SET);
        fread(&saved_key, sizeof(uint64_t), 1, hashfile -> file);
        fread(temp_data, hashfile -> record_size, 1, hashfile -> file);

        uint32_t bit = (saved_key >> (old_bucket -> local_depth - 1)) & 1;

        if (bit == 0) {
            insert_raw_into_bucket(hashfile, old_offset, old_bucket, saved_key, temp_data);
        } else {
            insert_raw_into_bucket(hashfile, new_offset, new_bucket, saved_key, temp_data);
        }
    }
    free(temp_data);
}

static void update_directory_pointers(const exhash_t *hashfile, uint64_t old_offset, uint64_t new_offset, uint8_t depth) {
    uint64_t dir_size_val = dir_size(hashfile);

    for (uint64_t i = 0; i < dir_size_val; i++) {
        if (hashfile -> directory[i] == old_offset) {
            uint32_t bit_of_index = (i >> (depth - 1)) & 1;

            if (bit_of_index == 1) {
                hashfile -> directory[i] = new_offset;
            }
        }
    }
}

static void split_bucket(exhash_t *hashfile, uint64_t old_offset, bucket_t old_bucket) {
    old_bucket.local_depth++;
    bucket_t new_bucket = { .local_depth = old_bucket.local_depth, .record_count = 0 };

    fseek(hashfile -> file, 0, SEEK_END);
    uint64_t new_offset = (uint64_t)ftell(hashfile -> file);
    write_empty_bucket(hashfile -> file, hashfile -> bucket_size, new_bucket.local_depth);

    uint32_t old_count = old_bucket.record_count;
    old_bucket.record_count = 0;

    // Zera o cabeçalho do balde antigo no disco
    fseek(hashfile -> file, (long)old_offset, SEEK_SET);
    fwrite(&old_bucket, sizeof(bucket_t), 1, hashfile -> file);

    redistribute_records(hashfile, old_offset, new_offset, &old_bucket, &new_bucket, old_count);
    update_directory_pointers(hashfile, old_offset, new_offset, old_bucket.local_depth);
}

static bool exhash_contains(const exhash_t *map, const char *key) {
    assert(map != NULL && key != NULL);

    uint64_t hashed_key = murmurhash3_64(key, strlen(key), 0);
    uint64_t idx = hashed_key & dir_mask(map);
    uint64_t offset = map -> directory[idx];

    bucket_t header;
    fseek(map -> file, (long)offset, SEEK_SET);
    fread(&header, sizeof(bucket_t), 1, map -> file);

    uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

    for (uint16_t i = 0; i < header.record_count; i++) {
        uint64_t slot_key;

        uint64_t slot_offset = offset + sizeof(bucket_t) + (i * slot_size);

        fseek(map -> file, (long)slot_offset, SEEK_SET);
        fread(&slot_key, sizeof(uint64_t), 1, map -> file);

        if (slot_key == hashed_key) return true;
    }

    return false;
}
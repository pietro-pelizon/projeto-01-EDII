#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/eHashing.h"

typedef struct  __attribute__((packed)) StEHashing {
    FILE *file;
    uint64_t *directory;
    uint32_t record_size;
    uint32_t bucket_size;
    uint8_t global_depth;
} eHashing;

typedef struct __attribute__((packed)) StBucket {
    uint16_t local_depth;
    uint16_t record_count;
} Bucket;


typedef struct __attribute__((packed)) stFileHeader{
    uint8_t global_depth;
    uint32_t bucket_size;
    uint32_t record_size;
    uint32_t directory_offset;

} FileHeader;


// Verifica se uma chave (alfanumérica) já está cadastrada no disco.
static bool eHashing_contains(const eHashing *map, const char *key);

// Transforma a chave (texto) em um número de 64 bits (identidade digital).
static uint64_t murmurhash3_64(const void *key, size_t len, uint32_t seed);

// Calcula quantos registros fixos cabem num único balde.
static uint64_t e_hashfile_capacity(const eHashing *map);

// Grava a estrutura inicial de um balde vazio (com zeros) em um offset do disco.
static void write_empty_bucket(FILE *file, uint32_t bucket_size, uint16_t depth);

// Salva as configurações globais (profundidade, tamanhos) no byte 0 do arquivo.
static void update_file_header(const eHashing *map);

// Lê o cabeçalho e reconstrói o mapa do diretório na RAM a partir de um arquivo já existente.
static void load_existing_hashfile(eHashing *map);

// Monta a infraestrutura do zero em um arquivo recém-criado (Header + Diretório + 1º Balde).
static void init_new_hashfile(eHashing *map);

// Escreve fisicamente a chave e o dado na próxima vaga livre de um balde.
static bool insert_into_bucket(const eHashing *map, uint64_t offset, Bucket *b, const void *key, const void *data);

// Aplica a máscara da profundidade global sobre o hash para encontrar o índice do balde.
static uint64_t get_directory_index(const eHashing *hashfile, const char *key);

// Dobra o tamanho do diretório na RAM (efeito espelho) quando um balde lotado atinge o limite global.
static bool expand_directory(eHashing *hashfile);

// Move os registros do balde velho, dividindo-os entre o balde velho e o novo com base no novo bit.
static void redistribute_records(const eHashing *hashfile, uint64_t old_offset, uint64_t new_offset, Bucket *old_bucket, Bucket *new_bucket, uint32_t old_count);

// Corrige as "placas" do diretório na RAM que devem apontar para a nova garagem construída.
static void update_directory_pointers(const eHashing *hashfile, uint64_t old_offset, uint64_t new_offset, uint8_t depth);

// Orquestra a divisão dos buckets: cria novo balde, redistribui os dados e atualiza os ponteiros.
static void split_bucket(eHashing *hashfile, uint64_t old_offset, Bucket old_bucket);

// ============== FUNÇÕES PRINCIPAIS DO MÓDULO ==============

eHashing *eHashing_init(const char *filename, uint32_t record_size, uint32_t bucket_size) {
    eHashing *map = malloc(sizeof(eHashing));
    if (!map) return NULL;

    map -> record_size = record_size;
    map -> bucket_size = bucket_size;
    map -> file = fopen(filename, "r + b");

    if (map -> file == NULL) {
        map -> file = fopen(filename, "w + b");

        if (!map -> file) { free(map); return NULL; }

        init_new_hashfile(map);
    }

    else {
        load_existing_hashfile(map);
    }

    return map;
}

bool eHashing_insert(eHashing *hashfile, const void *data, const char *key) {
    assert(hashfile != NULL && data != NULL);

    if (eHashing_contains(hashfile, key)) {
        return false;
    }

    while (true) {

        uint64_t idx = get_directory_index(hashfile, key);
        uint64_t offset = hashfile -> directory[idx];

        Bucket directory_header;

        fseek(hashfile ->  file, (long)offset, SEEK_SET);
        fread(&directory_header, sizeof(Bucket), 1, hashfile -> file);


        if (directory_header.record_count < e_hashfile_capacity(hashfile)) {
            return insert_into_bucket(hashfile, offset, &directory_header, key, data);
        }

        if (directory_header.local_depth == hashfile -> global_depth) {
            expand_directory(hashfile);
        }

        split_bucket(hashfile, offset, directory_header);

    }
}

bool eHashing_search(const eHashing *map, const char *key, void *out_data) {
    assert(map != NULL && key != NULL && out_data != NULL);

    uint64_t hashed_key = murmurhash3_64(key, strlen(key), 0);

    uint64_t idx = hashed_key & ((1 << map -> global_depth) - 1);
    uint64_t offset = map -> directory[idx];

    Bucket header;
    fseek(map -> file, (long)offset, SEEK_SET);
    fread(&header, sizeof(Bucket), 1, map -> file);

    uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

    for (uint16_t i = 0; i < header.record_count; i++) {
        uint64_t slot_key;

        // CORREÇÃO: Mudei para long para não corromper em arquivos grandes
        uint64_t slot_offset = (long)offset + sizeof(Bucket) + (i * slot_size);

        fseek(map -> file, (long)slot_offset, SEEK_SET);
        fread(&slot_key, sizeof(uint64_t), 1, map -> file);

        // 2. Compara os hashes
        if (slot_key == hashed_key) {
            // Leu a chave e bateu? O ponteiro do disco já está exatamente
            // no início do bloco de dados! É só dar o fread no out_data.
            fread(out_data, map -> record_size, 1, map -> file);
            return true;
        }
    }

    return false;
}

void eHashing_destroy(eHashing *map) {
    if (map == NULL) return;

    // Prepara o cabeçalho para salvar
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


// ============== FUNÇÕES STATIC QUE FORAM UTILIZADAS COMO UTILITÁRIAS PARA AS FUNÇÕES PRINCIPAIS ==============

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

static uint64_t e_hashfile_capacity(const eHashing *map) {

    uint64_t C = map -> bucket_size - sizeof(Bucket);
    uint64_t Q = sizeof(uint64_t) + map -> record_size;

    return C/Q;
}

static void write_empty_bucket(FILE *file, uint32_t bucket_size,  uint16_t depth) {
    Bucket bh = { .local_depth = depth, .record_count = 0 };
    fwrite(&bh, sizeof(Bucket), 1, file);

    // Preenche o restante do espaço do balde com zeros
    char *padding = calloc(1, bucket_size - sizeof(Bucket));
    fwrite(padding, 1, bucket_size - sizeof(Bucket), file);
    free(padding);
}

static void update_file_header(const eHashing *map) {
    FileHeader header = {
        .global_depth = map -> global_depth,
        .bucket_size = map -> bucket_size,
        .record_size = map -> record_size,
        .directory_offset = sizeof(FileHeader)
    };

    fseek(map -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(FileHeader), 1, map -> file);
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

static void init_new_hashfile(eHashing *map) {
    map -> global_depth = 0;
    map -> directory = malloc(sizeof(uint64_t) * 1);

    long first_bucket_offset = sizeof(FileHeader) + sizeof(uint64_t);
    map -> directory[0] = (uint64_t)first_bucket_offset;

    update_file_header(map);

    fseek(map -> file, sizeof(FileHeader), SEEK_SET);
    fwrite(map -> directory, sizeof(uint64_t), 1, map -> file);

    fseek(map -> file, first_bucket_offset, SEEK_SET);
    write_empty_bucket(map -> file, map -> bucket_size, 0);
}

static bool insert_into_bucket(const eHashing *map, uint64_t offset, Bucket *b, const void *key, const void *data) {
    uint64_t numeric_key = murmurhash3_64(key, strlen(key), 0);

    uint64_t slot_offset = offset + sizeof(Bucket) +
                       (b -> record_count * (sizeof(uint64_t) + map -> record_size));

    // 3. Pula pro slot e grava
    fseek(map -> file, (long)slot_offset, SEEK_SET);
    fwrite(&numeric_key, sizeof(uint64_t), 1, map -> file);
    fwrite(data, map -> record_size, 1, map -> file);

    // 4. Atualiza a prancheta do guarda no disco
    b -> record_count++;
    fseek(map -> file, (long)offset, SEEK_SET);
    fwrite(b, sizeof(Bucket), 1, map->file);

    return true;
}

static uint64_t get_directory_index(const eHashing *hashfile, const char *key) {
    assert(hashfile != NULL && key != NULL);

    uint64_t hashed_key = murmurhash3_64(key, strlen(key), 0);
    return hashed_key & ((1 << hashfile -> global_depth) - 1);
}

static bool expand_directory(eHashing *hashfile) {

    // Dobramos a capacidade do diretório
    uint32_t old_size = 1 << hashfile -> global_depth;
    uint32_t new_size = old_size * 2;

    uint64_t *new_directory = realloc(hashfile -> directory, sizeof(uint64_t) * new_size);
    assert(new_directory != NULL);
    hashfile -> directory = new_directory;

    // Espelhamos os apontadores do diretório (001 e 101 apontam para o mesmo endereço)
    for (uint32_t i = 0; i < old_size; i++) {
        hashfile -> directory[i + old_size] = hashfile -> directory[i];
    }

    hashfile -> global_depth++;

    fseek(hashfile -> file, 0, SEEK_SET);

    FileHeader header;
    fread(&header, sizeof(FileHeader), 1, hashfile -> file);
    header.global_depth = hashfile -> global_depth;

    fseek(hashfile -> file, 0, SEEK_SET);
    fwrite(&header, sizeof(FileHeader), 1, hashfile -> file);

    return true;

}

static void redistribute_records(const eHashing *hashfile, uint64_t old_offset, uint64_t new_offset, Bucket *old_bucket, Bucket *new_bucket, uint32_t old_count) {
    void *temp_data = malloc(hashfile -> record_size);
    assert(temp_data != NULL);

    for (uint32_t i = 0; i < old_count; i++) {
        uint64_t saved_key;

        uint64_t slot_offset = old_offset + sizeof(Bucket) + (i * (sizeof(uint64_t) + hashfile -> record_size));

        fseek(hashfile -> file, (long)slot_offset, SEEK_SET);
        fread(&saved_key, sizeof(uint64_t), 1, hashfile->file);
        fread(temp_data, hashfile->record_size, 1, hashfile->file);

        uint64_t hashed_key = murmurhash3_64(&saved_key, sizeof(uint64_t), 0);
        uint32_t bit = (hashed_key >> (old_bucket -> local_depth - 1)) & 1;

        if (bit == 0) {
            insert_into_bucket(hashfile, old_offset, old_bucket, &saved_key, temp_data);
        }

        else {
            insert_into_bucket(hashfile, new_offset, new_bucket, &saved_key, temp_data);
        }
    }

    free(temp_data);
}

static void update_directory_pointers(const eHashing *hashfile, uint64_t old_offset, uint64_t new_offset, uint8_t depth) {
    uint32_t dir_size = 1 << hashfile -> global_depth;

    for (uint32_t i = 0; i < dir_size; i++) {
        if (hashfile -> directory[i] == old_offset) {
            uint32_t bit_of_index = (i >> (depth - 1)) & 1;

            if (bit_of_index == 1) {
                hashfile -> directory[i] = new_offset;
            }
        }
    }
}

static void split_bucket(eHashing *hashfile, uint64_t old_offset, Bucket old_bucket) {
    old_bucket.local_depth++;
    Bucket new_bucket = {.local_depth = old_bucket.local_depth, .record_count = 0};

    fseek(hashfile->file, 0, SEEK_END);
    uint64_t new_offset = ftell(hashfile->file);

    uint32_t old_count = old_bucket.record_count;
    old_bucket.record_count = 0;

    // Executa a separação física
    redistribute_records(hashfile, old_offset, new_offset, &old_bucket, &new_bucket, old_count);

    // Executa a correção do índice
    update_directory_pointers(hashfile, old_offset, new_offset, old_bucket.local_depth);
}

bool eHashing_contains(const eHashing *map, const char *key) {
    assert(map != NULL && key != NULL);

    uint64_t hashed_key = murmurhash3_64(key, strlen(key), 0);

    uint64_t idx = hashed_key & ((1 << map -> global_depth) - 1);
    uint64_t offset = map -> directory[idx];

    Bucket header;
    fseek(map -> file, (long)offset, SEEK_SET);
    fread(&header, sizeof(Bucket), 1, map -> file);

    uint64_t slot_size = sizeof(uint64_t) + map -> record_size;

    for (uint16_t i = 0; i < header.record_count; i++) {
        uint64_t slot_key;

        uint64_t slot_offset = (long)offset + sizeof(Bucket) + (i * slot_size);

        fseek(map -> file, (long)slot_offset, SEEK_SET);
        fread(&slot_key, sizeof(uint64_t), 1, map -> file);

        if (slot_key == hashed_key) {
            return true;
        }
    }

    return false;
}




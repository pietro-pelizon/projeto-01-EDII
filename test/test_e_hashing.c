#include <assert.h>

#include "../unity/Unity/src/unity.h"
#include "../include/e_hashing.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct StEHashing eHashing;
typedef struct StBucket bucket;

eHashing *map;
const char *FILENAME = "projeto_teste.dat";
uint64_t hash_string(char *string);
static uint64_t hash_64bit(uint64_t x);

void setUp() {
    map = e_hashing_init(FILENAME, 512, free);
}

void tearDown() {
    e_hashing_destroy(map);
    remove(FILENAME);
}

typedef struct {
    int id;
    char nome[30];
    double nota;
}Aluno;

void tenta_inserir_e_buscar_struct_generica(void) {
    setUp();
    Aluno a1 = {1, "Pietro", 9.5};
    uint64_t key = (uint64_t)a1.id;


    bool ok = e_hashing_insert(map, &a1, key, sizeof(Aluno));
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir o aluno corretamente");

    size_t size_out;

    Aluno *res = e_hashing_search(map, key, &size_out);

    TEST_ASSERT_NOT_NULL_MESSAGE(res, "Deve achar o aluno");
    TEST_ASSERT_EQUAL_INT_MESSAGE(sizeof(Aluno), size_out, "");
    TEST_ASSERT_EQUAL_STRING("Pietro", res -> nome);
    TEST_ASSERT_EQUAL_DOUBLE(9.5, res -> nota);

    tearDown();

}

void test_inserir_string_com_hash_customizado() {
    setUp();

    char *texto = "Uma string de tamanho dinamico bem longa...";
    uint64_t key = hash_string(texto);
    size_t len = strlen(texto) + 1;

    e_hashing_insert(map, texto, key, len);

    size_t size_out;
    char *res = (char*)e_hashing_search(map, key, &size_out);

    TEST_ASSERT_EQUAL_STRING(texto, res);
    free(res);

    tearDown();
}

void deve_realizar_split_se_o_bucket_encher(void) {
    setUp();

    for (uint64_t i = 0; i < 10000; i++) {
        uint64_t chave = i;
        uint64_t valor_para_guardar = i * 100;

        bool ok = e_hashing_insert(map, &valor_para_guardar,chave, sizeof(uint64_t));
        TEST_ASSERT_TRUE_MESSAGE(ok, "Falha na insercao massiva");
    }

    for (uint64_t i = 0; i < 100; i++) {
        size_t s;
        uint64_t *res = (uint64_t*)e_hashing_search(map, i, &s);

        TEST_ASSERT_NOT_NULL_MESSAGE(res, "Dado perdido apos multiplos splits");
        TEST_ASSERT_EQUAL_UINT64(i * 100, *res);

        free(res);
    }

    tearDown();
}

void test_insere_remove_e_tenta_buscar(void) {
    setUp();

    Aluno a1 = {31, "Fernando", 4.3};

    bool ok = e_hashing_insert(map, &a1, a1.id, sizeof(Aluno));
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir aluno corretamente");

    size_t size_out;

    Aluno *removed = (Aluno*)e_hashing_remove(map, (uint64_t)a1.id, &size_out);
    TEST_ASSERT_NOT_NULL_MESSAGE(removed, "O dado removido nao deveria ser NULL");
    TEST_ASSERT_EQUAL_UINT(sizeof(Aluno), size_out);
    TEST_ASSERT_EQUAL_STRING("Fernando", removed -> nome);

    bool ok_again = e_hashing_insert(map, &a1, (uint64_t)a1.id, sizeof(Aluno));
    TEST_ASSERT_TRUE_MESSAGE(ok_again, "Deveria ser possivel reinserir a chave apos remocao");

    Aluno *search_res = (Aluno*)e_hashing_search(map, (uint64_t)a1.id, &size_out);
    TEST_ASSERT_NOT_NULL_MESSAGE(search_res, "Deveria NÃO retornar NULL apos a remocao");

    free(removed);

    tearDown();
}

void test_inserir_chaves_duplicadas() {
    setUp();

    int x = 4;
    bool ok = e_hashing_insert(map, &x, 10, sizeof(int));
    TEST_ASSERT_TRUE_MESSAGE(ok, "deve inserir corretamente");

    int x2 = 14;
    bool ok2 = e_hashing_insert(map, &x2, 10, sizeof(int));
    TEST_ASSERT_FALSE_MESSAGE(ok2, "Deve retornar false por key repetida");

    tearDown();
}

void test_espacamentos_correto() {
    setUp();

    int x = 1, y = 2, z = 3, a = 14, b = 112, c = 131;
    e_hashing_insert(map, &x, 1, sizeof(int));
    e_hashing_insert(map, &y, 2, sizeof(int));
    e_hashing_insert(map, &z, 3, sizeof(int));
    e_hashing_insert(map, &a, 4, sizeof(int));
    e_hashing_insert(map, &b, 5, sizeof(int));
    e_hashing_insert(map, &c, 6, sizeof(int));


    size_t size_out1;
    size_t size_out2;
    size_t size_out3;

    e_hashing_remove(map, 1, &size_out1);
    e_hashing_remove(map, 3, &size_out2);
    e_hashing_remove(map, 5, &size_out3);

    char *nome = "pietrofernandopelizon";

    bool ok = e_hashing_insert(map, &nome, 3, sizeof(char*));
    TEST_ASSERT_TRUE_MESSAGE(ok, "deve inserir corretamente");

    tearDown();

}

void dado_persiste_caso_hashmap_seja_destruido() {
    setUp();

    int x = 33;
    e_hashing_insert(map, &x, 10, sizeof(int));

    e_hashing_destroy(map);

    map = e_hashing_init(FILENAME, 512, free);

    size_t size_out;

    int *found = e_hashing_search(map, 10, &size_out);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_INT_MESSAGE(33, *found, "Deve achar pois os dados persistem");

    tearDown();
}

void limite_do_bloco() {
    setUp();

    char registro[512];
    bool ok = e_hashing_insert(map, &registro, 1, sizeof(registro) - 8);
    TEST_ASSERT_TRUE_MESSAGE(ok, "deve inserir corretamente, ainda está dentro do limite do bucket");

    tearDown();
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(tenta_inserir_e_buscar_struct_generica);
    RUN_TEST(test_inserir_string_com_hash_customizado);
    RUN_TEST(deve_realizar_split_se_o_bucket_encher);
    RUN_TEST(test_insere_remove_e_tenta_buscar);
    RUN_TEST(test_inserir_chaves_duplicadas);
    RUN_TEST(test_inserir_chaves_duplicadas);
    RUN_TEST(test_espacamentos_correto);
    RUN_TEST(dado_persiste_caso_hashmap_seja_destruido);
    RUN_TEST(limite_do_bloco);
    return UNITY_END();
}
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../unity/Unity/src/unity.h"
#include "../include/exhash.h"

exhash_t *map = NULL;
const char *FILENAME = "projeto_teste.dat";

// Estrutura genérica para testes
typedef struct {
    int id;
    char nome[30];
    double nota;
} Aluno;

void setUp() {
    remove(FILENAME);
    map = NULL;
}

void tearDown() {
    if (map != NULL) {
        exhash_destroy(map);
        map = NULL;
    }
    remove(FILENAME);
}

void tenta_inserir_e_buscar_struct_generica(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno a1 = {1, "Pietro", 9.5};

    bool ok = exhash_insert(map, &a1, "1");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir o aluno corretamente");

    Aluno res;
    bool found = exhash_search(map, "1", &res);

    TEST_ASSERT_TRUE_MESSAGE(found, "Deve achar o aluno");
    TEST_ASSERT_EQUAL_STRING("Pietro", res.nome);
    TEST_ASSERT_EQUAL_DOUBLE(9.5, res.nota);
}

void test_inserir_string_com_hash_customizado() {
    char payload[64] = "Uma string de tamanho dinamico bem longa...";
    map = exhash_init(FILENAME, sizeof(payload), 1024);

    bool ok = exhash_insert(map, payload, "chave_do_texto");
    TEST_ASSERT_TRUE(ok);

    char res[64];
    bool found = exhash_search(map, "chave_do_texto", res);

    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_STRING(payload, res);
}

void deve_realizar_split_se_o_bucket_encher(void) {
    map = exhash_init(FILENAME, sizeof(uint64_t), 128);

    char chave_str[20];
    for (uint64_t i = 0; i < 5000; i++) {
        sprintf(chave_str, "%" PRIu64, i);
        uint64_t valor_para_guardar = i * 100;

        bool ok = exhash_insert(map, &valor_para_guardar, chave_str);
        TEST_ASSERT_TRUE_MESSAGE(ok, "Falha na insercao massiva");
    }

    for (uint64_t i = 0; i < 100; i++) {
        sprintf(chave_str, "%" PRIu64, i);
        uint64_t res;

        bool found = exhash_search(map, chave_str, &res);

        TEST_ASSERT_TRUE_MESSAGE(found, "Dado perdido apos multiplos splits");
        TEST_ASSERT_EQUAL_UINT64(i * 100, res);
    }
}

void test_insere_remove_e_tenta_buscar(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno a1 = {31, "Fernando", 4.3};

    bool ok = exhash_insert(map, &a1, "31");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir aluno corretamente");

    Aluno *removed = (Aluno*)exhash_remove(map, "31");
    TEST_ASSERT_NOT_NULL_MESSAGE(removed, "O dado removido nao deveria ser NULL");
    TEST_ASSERT_EQUAL_STRING("Fernando", removed->nome);
    free(removed);

    bool ok_again = exhash_insert(map, &a1, "31");
    TEST_ASSERT_TRUE_MESSAGE(ok_again, "Deveria ser possivel reinserir a chave apos remocao");

    Aluno search_res;
    bool found = exhash_search(map, "31", &search_res);
    TEST_ASSERT_TRUE_MESSAGE(found, "Deveria encontrar o dado apos a reinsercao");
}

void test_inserir_chaves_duplicadas() {
    map = exhash_init(FILENAME, sizeof(int), 1024);

    int x = 4;
    bool ok = exhash_insert(map, &x, "chave_repetida");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir a primeira vez corretamente");

    int x2 = 14;
    bool ok2 = exhash_insert(map, &x2, "chave_repetida");
    TEST_ASSERT_FALSE_MESSAGE(ok2, "Deve retornar false ao tentar inserir chave repetida");
}

void test_espacamentos_correto() {
    map = exhash_init(FILENAME, sizeof(int), 1024);

    int x = 1, y = 2, z = 3, a = 14, b = 112, c = 131;
    exhash_insert(map, &x, "1");
    exhash_insert(map, &y, "2");
    exhash_insert(map, &z, "3");
    exhash_insert(map, &a, "4");
    exhash_insert(map, &b, "5");
    exhash_insert(map, &c, "6");

    // Remove alternados para testar o Swap with Last (sem deixar buracos)
    int *rem1 = exhash_remove(map, "1"); free(rem1);
    int *rem2 = exhash_remove(map, "3"); free(rem2);
    int *rem3 = exhash_remove(map, "5"); free(rem3);

    int novo = 999;
    bool ok = exhash_insert(map, &novo, "7");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir normalmente preenchendo o espacamento");

    int res;
    bool found = exhash_search(map, "7", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(999, res);
}

void dado_persiste_caso_hashmap_seja_destruido() {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 33;
    exhash_insert(map, &x, "chave_persistente");

    // Destrói a estrutura (salvando no disco)
    exhash_destroy(map);
    map = NULL;

    // Recarrega do arquivo
    map = exhash_init(FILENAME, sizeof(int), 512);

    int res;
    bool found = exhash_search(map, "chave_persistente", &res);

    TEST_ASSERT_TRUE_MESSAGE(found, "Deve achar pois os dados persistem no arquivo");
    TEST_ASSERT_EQUAL_INT(33, res);
}

void limite_do_bloco() {
    // Teste com bucket de 1024, record limitando o espaço
    char registro[512] = "Payload pesado";
    map = exhash_init(FILENAME, sizeof(registro), 1024);

    bool ok = exhash_insert(map, &registro, "bloco_limite");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir corretamente se a matemática do record_size couber no bucket");
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(tenta_inserir_e_buscar_struct_generica);
    RUN_TEST(test_inserir_string_com_hash_customizado);
    RUN_TEST(deve_realizar_split_se_o_bucket_encher);
    RUN_TEST(test_insere_remove_e_tenta_buscar);
    RUN_TEST(test_inserir_chaves_duplicadas);
    RUN_TEST(test_espacamentos_correto);
    RUN_TEST(dado_persiste_caso_hashmap_seja_destruido);
    RUN_TEST(limite_do_bloco);
    return UNITY_END();
}
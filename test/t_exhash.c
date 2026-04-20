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
const char *FILENAME = "exhash_test.dat";

typedef struct {
    int    id;
    char   nome[30];
    double nota;
} Aluno;

void setUp(void) {
    remove(FILENAME);
    map = NULL;
}

void tearDown(void) {
    if (map != NULL) {
        exhash_destroy(map);
        map = NULL;
    }
    remove(FILENAME);
}

// ============================================================
//  INIT
// ============================================================

void test_init_cria_arquivo(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);
    TEST_ASSERT_NOT_NULL_MESSAGE(map, "exhash_init deve retornar ponteiro valido");

    FILE *f = fopen(FILENAME, "rb");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Arquivo deve ser criado no disco");
    fclose(f);
}

void test_init_reabre_arquivo_existente(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);
    TEST_ASSERT_NOT_NULL(map);
    exhash_destroy(map);
    map = NULL;

    map = exhash_init(FILENAME, sizeof(int), 512);
    TEST_ASSERT_NOT_NULL_MESSAGE(map, "Deve reabrir arquivo existente sem erro");
}

void test_init_com_record_size_diferente(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);
    TEST_ASSERT_NOT_NULL(map);
}

// ============================================================
//  INSERT
// ============================================================

void test_insert_simples(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int valor = 42;
    bool ok = exhash_insert(map, &valor, "chave");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Insercao simples deve retornar true");
}

void test_insert_chave_duplicada_retorna_false(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 1;
    exhash_insert(map, &x, "dup");

    int y = 2;
    bool ok = exhash_insert(map, &y, "dup");
    TEST_ASSERT_FALSE_MESSAGE(ok, "Insercao de chave duplicada deve retornar false");
}

void test_insert_chave_duplicada_nao_sobrescreve(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int original = 10;
    exhash_insert(map, &original, "k");

    int outro = 99;
    exhash_insert(map, &outro, "k");

    int res;
    exhash_search(map, "k", &res);
    TEST_ASSERT_EQUAL_INT_MESSAGE(10, res, "Valor original nao deve ser sobrescrito");
}

void test_insert_multiplas_chaves_distintas(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int i = 0; i < 20; i++) {
        char chave[16];
        sprintf(chave, "chave_%d", i);
        bool ok = exhash_insert(map, &i, chave);
        TEST_ASSERT_TRUE_MESSAGE(ok, "Cada chave distinta deve inserir com sucesso");
    }
}

void test_insert_struct_generica(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno a = {7, "Joao", 8.5};
    bool ok = exhash_insert(map, &a, "7");
    TEST_ASSERT_TRUE(ok);
}

void test_insert_payload_maximo_no_bucket(void) {
    char registro[512] = "Payload pesado";
    map = exhash_init(FILENAME, sizeof(registro), 1024);

    bool ok = exhash_insert(map, &registro, "bloco_limite");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve inserir mesmo com record_size grande");
}

// ============================================================
//  SEARCH
// ============================================================

void test_search_encontra_valor_inserido(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 77;
    exhash_insert(map, &x, "k77");

    int res;
    bool found = exhash_search(map, "k77", &res);
    TEST_ASSERT_TRUE_MESSAGE(found, "Deve encontrar chave inserida");
    TEST_ASSERT_EQUAL_INT(77, res);
}

void test_search_chave_inexistente_retorna_false(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int res;
    bool found = exhash_search(map, "nao_existe", &res);
    TEST_ASSERT_FALSE_MESSAGE(found, "Busca por chave inexistente deve retornar false");
}

void test_search_struct_campos_corretos(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno a = {3, "Maria", 7.0};
    exhash_insert(map, &a, "3");

    Aluno res;
    bool found = exhash_search(map, "3", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(3, res.id);
    TEST_ASSERT_EQUAL_STRING("Maria", res.nome);
    TEST_ASSERT_EQUAL_DOUBLE(7.0, res.nota);
}

void test_search_string_como_dado(void) {
    char payload[64] = "texto longo de teste";
    map = exhash_init(FILENAME, sizeof(payload), 512);

    exhash_insert(map, payload, "str_key");

    char res[64];
    bool found = exhash_search(map, "str_key", res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_STRING(payload, res);
}

void test_search_apos_multiplas_insercoes(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int i = 0; i < 50; i++) {
        char chave[16];
        sprintf(chave, "%d", i);
        exhash_insert(map, &i, chave);
    }

    for (int i = 0; i < 50; i++) {
        char chave[16];
        sprintf(chave, "%d", i);
        int res;
        bool found = exhash_search(map, chave, &res);
        TEST_ASSERT_TRUE_MESSAGE(found, "Deve encontrar todos os registros inseridos");
        TEST_ASSERT_EQUAL_INT(i, res);
    }
}

// ============================================================
//  REMOVE
// ============================================================

void test_remove_retorna_dado_correto(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno a = {5, "Carlos", 6.0};
    exhash_insert(map, &a, "5");

    Aluno *removed = (Aluno *)exhash_remove(map, "5");
    TEST_ASSERT_NOT_NULL(removed);
    TEST_ASSERT_EQUAL_STRING("Carlos", removed -> nome);
    free(removed);
}

void test_remove_chave_inexistente_retorna_null(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    void *res = exhash_remove(map, "fantasma");
    TEST_ASSERT_NULL_MESSAGE(res, "Remove de chave inexistente deve retornar NULL");
}

void test_remove_impede_busca_posterior(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 55;
    exhash_insert(map, &x, "rem_key");

    void *removed = exhash_remove(map, "rem_key");
    free(removed);

    int res;
    bool found = exhash_search(map, "rem_key", &res);
    TEST_ASSERT_FALSE_MESSAGE(found, "Dado removido nao deve ser encontrado");
}

void test_remove_permite_reinsercao(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 10;
    exhash_insert(map, &x, "r");

    void *rem = exhash_remove(map, "r");
    free(rem);

    int y = 20;
    bool ok = exhash_insert(map, &y, "r");
    TEST_ASSERT_TRUE_MESSAGE(ok, "Deve reinserir apos remocao");

    int res;
    exhash_search(map, "r", &res);
    TEST_ASSERT_EQUAL_INT(20, res);
}

void test_remove_nao_corrompe_outros_registros(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int a = 1, b = 2, c = 3;
    exhash_insert(map, &a, "a");
    exhash_insert(map, &b, "b");
    exhash_insert(map, &c, "c");

    void *rem = exhash_remove(map, "b");
    free(rem);

    int ra, rc;
    TEST_ASSERT_TRUE(exhash_search(map, "a", &ra));
    TEST_ASSERT_EQUAL_INT(1, ra);

    TEST_ASSERT_TRUE(exhash_search(map, "c", &rc));
    TEST_ASSERT_EQUAL_INT(3, rc);
}

void test_remove_alternado_e_insere(void) {
    map = exhash_init(FILENAME, sizeof(int), 1024);

    int vals[6] = {1, 2, 3, 14, 112, 131};
    for (int i = 0; i < 6; i++) {
        char chave[8];
        sprintf(chave, "%d", i + 1);
        exhash_insert(map, &vals[i], chave);
    }

    int *r1 = exhash_remove(map, "1"); free(r1);
    int *r3 = exhash_remove(map, "3"); free(r3);
    int *r5 = exhash_remove(map, "5"); free(r5);

    int novo = 999;
    bool ok = exhash_insert(map, &novo, "7");
    TEST_ASSERT_TRUE(ok);

    int res;
    bool found = exhash_search(map, "7", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(999, res);
}

// ============================================================
//  SPLIT / CRESCIMENTO
// ============================================================

void test_split_massivo_5000_insercoes(void) {
    map = exhash_init(FILENAME, sizeof(uint64_t), 128);

    char chave[20];
    for (uint64_t i = 0; i < 5000; i++) {
        sprintf(chave, "%" PRIu64, i);
        uint64_t val = i * 100;
        bool ok = exhash_insert(map, &val, chave);
        TEST_ASSERT_TRUE_MESSAGE(ok, "Falha na insercao massiva");
    }

    for (uint64_t i = 0; i < 100; i++) {
        sprintf(chave, "%" PRIu64, i);
        uint64_t res;
        bool found = exhash_search(map, chave, &res);
        TEST_ASSERT_TRUE_MESSAGE(found, "Dado perdido apos splits");
        TEST_ASSERT_EQUAL_UINT64(i * 100, res);
    }
}

void test_split_nao_perde_nenhum_dado(void) {
    map = exhash_init(FILENAME, sizeof(int), 64);

    // bucket_size=64: capacity = (64 - 4) / (8 + 4) = 5 slots
    // força vários splits com poucos registros
    for (int i = 0; i < 200; i++) {
        char chave[16];
        sprintf(chave, "k%d", i);
        exhash_insert(map, &i, chave);
    }

    for (int i = 0; i < 200; i++) {
        char chave[16];
        sprintf(chave, "k%d", i);
        int res;
        bool found = exhash_search(map, chave, &res);
        TEST_ASSERT_TRUE_MESSAGE(found, "Dado perdido apos splits com bucket pequeno");
        TEST_ASSERT_EQUAL_INT(i, res);
    }
}

// ============================================================
//  GET ALL
// ============================================================

void test_get_all_retorna_todos_os_registros(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int i = 0; i < 10; i++) {
        char chave[8];
        sprintf(chave, "k%d", i);
        exhash_insert(map, &i, chave);
    }

    uint64_t count = 0;
    void **todos = exhash_get_all(map, &count);

    TEST_ASSERT_NOT_NULL(todos);
    TEST_ASSERT_EQUAL_UINT64_MESSAGE(10, count, "get_all deve retornar 10 registros");

    for (uint64_t i = 0; i < count; i++) free(todos[i]);
    free(todos);
}

void test_get_all_mapa_vazio(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    uint64_t count = 0;
    void **todos = exhash_get_all(map, &count);

    TEST_ASSERT_EQUAL_UINT64_MESSAGE(0, count, "get_all em mapa vazio deve retornar 0");

    free(todos);
}

void test_get_all_apos_remocoes(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int i = 0; i < 6; i++) {
        char chave[8];
        sprintf(chave, "k%d", i);
        exhash_insert(map, &i, chave);
    }

    void *r = exhash_remove(map, "k2"); free(r);
    r = exhash_remove(map, "k4"); free(r);

    uint64_t count = 0;
    void **todos = exhash_get_all(map, &count);

    TEST_ASSERT_EQUAL_UINT64_MESSAGE(4, count, "get_all deve refletir remocoes");

    for (uint64_t i = 0; i < count; i++) free(todos[i]);
    free(todos);
}

void test_get_all_sem_duplicatas_apos_expand(void) {
    // Garante que get_all não retorna o mesmo registro duas vezes
    // quando o diretório tem múltiplos ponteiros pro mesmo bucket
    map = exhash_init(FILENAME, sizeof(int), 128);

    for (int i = 0; i < 30; i++) {
        char chave[16];
        sprintf(chave, "x%d", i);
        exhash_insert(map, &i, chave);
    }

    uint64_t count = 0;
    void **todos = exhash_get_all(map, &count);

    TEST_ASSERT_EQUAL_UINT64_MESSAGE(30, count, "get_all nao deve duplicar registros");

    for (uint64_t i = 0; i < count; i++) free(todos[i]);
    free(todos);
}

// ============================================================
//  PERSISTÊNCIA
// ============================================================

void test_dado_persiste_apos_destroy_e_reopen(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 33;
    exhash_insert(map, &x, "persistente");

    exhash_destroy(map);
    map = NULL;

    map = exhash_init(FILENAME, sizeof(int), 512);

    int res;
    bool found = exhash_search(map, "persistente", &res);
    TEST_ASSERT_TRUE_MESSAGE(found, "Dado deve persistir apos reopen");
    TEST_ASSERT_EQUAL_INT(33, res);
}

void test_multiplos_dados_persistem(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int i = 0; i < 20; i++) {
        char chave[16];
        sprintf(chave, "p%d", i);
        exhash_insert(map, &i, chave);
    }

    exhash_destroy(map);
    map = NULL;

    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int i = 0; i < 20; i++) {
        char chave[16];
        sprintf(chave, "p%d", i);
        int res;
        bool found = exhash_search(map, chave, &res);
        TEST_ASSERT_TRUE_MESSAGE(found, "Todos os dados devem persistir");
        TEST_ASSERT_EQUAL_INT(i, res);
    }
}

void test_struct_persiste_com_todos_campos(void) {
    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno a = {99, "Persistencia", 10.0};
    exhash_insert(map, &a, "99");

    exhash_destroy(map);
    map = NULL;

    map = exhash_init(FILENAME, sizeof(Aluno), 1024);

    Aluno res;
    bool found = exhash_search(map, "99", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(99, res.id);
    TEST_ASSERT_EQUAL_STRING("Persistencia", res.nome);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, res.nota);
}

void test_persistencia_apos_split(void) {
    map = exhash_init(FILENAME, sizeof(int), 64);

    for (int i = 0; i < 50; i++) {
        char chave[16];
        sprintf(chave, "s%d", i);
        exhash_insert(map, &i, chave);
    }

    exhash_destroy(map);
    map = NULL;

    map = exhash_init(FILENAME, sizeof(int), 64);

    for (int i = 0; i < 50; i++) {
        char chave[16];
        sprintf(chave, "s%d", i);
        int res;
        bool found = exhash_search(map, chave, &res);
        TEST_ASSERT_TRUE_MESSAGE(found, "Dado deve persistir mesmo apos splits");
        TEST_ASSERT_EQUAL_INT(i, res);
    }
}

// ============================================================
//  EDGE CASES
// ============================================================

void test_chave_com_caracteres_especiais(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 1;
    bool ok = exhash_insert(map, &x, "chave!@#123");
    TEST_ASSERT_TRUE(ok);

    int res;
    bool found = exhash_search(map, "chave!@#123", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(1, res);
}

void test_chave_numerica_como_string(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 42;
    exhash_insert(map, &x, "0001");

    int res;
    bool found = exhash_search(map, "0001", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(42, res);
}

void test_chave_longa(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    const char *chave_longa = "esta_e_uma_chave_muito_longa_para_testar_o_hash_com_strings_extensas_xxxxxxxxxxx";
    int x = 7;
    exhash_insert(map, &x, chave_longa);

    int res;
    bool found = exhash_search(map, chave_longa, &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(7, res);
}

void test_remover_unico_elemento_e_reinserir(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int x = 100;
    exhash_insert(map, &x, "unico");

    void *rem = exhash_remove(map, "unico");
    TEST_ASSERT_NOT_NULL(rem);
    free(rem);

    int res;
    bool found = exhash_search(map, "unico", &res);
    TEST_ASSERT_FALSE(found);

    int y = 200;
    bool ok = exhash_insert(map, &y, "unico");
    TEST_ASSERT_TRUE(ok);

    exhash_search(map, "unico", &res);
    TEST_ASSERT_EQUAL_INT(200, res);
}

void test_valor_zero(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int zero = 0;
    exhash_insert(map, &zero, "zero");

    int res = -1;
    bool found = exhash_search(map, "zero", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(0, res);
}

void test_valor_negativo(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    int neg = -999;
    exhash_insert(map, &neg, "neg");

    int res;
    bool found = exhash_search(map, "neg", &res);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_INT(-999, res);
}

void test_insere_remove_insere_mesma_chave_tres_vezes(void) {
    map = exhash_init(FILENAME, sizeof(int), 512);

    for (int ciclo = 0; ciclo < 3; ciclo++) {
        int val = ciclo * 10;
        bool ok = exhash_insert(map, &val, "ciclo");
        TEST_ASSERT_TRUE_MESSAGE(ok, "Reinsercao apos remocao deve funcionar");

        int res;
        exhash_search(map, "ciclo", &res);
        TEST_ASSERT_EQUAL_INT(val, res);

        void *rem = exhash_remove(map, "ciclo");
        TEST_ASSERT_NOT_NULL(rem);
        free(rem);
    }
}

// ============================================================
//  MAIN
// ============================================================

int main(void) {
    UNITY_BEGIN();

    // Init
    RUN_TEST(test_init_cria_arquivo);
    RUN_TEST(test_init_reabre_arquivo_existente);
    RUN_TEST(test_init_com_record_size_diferente);

    // Insert
    RUN_TEST(test_insert_simples);
    RUN_TEST(test_insert_chave_duplicada_retorna_false);
    RUN_TEST(test_insert_chave_duplicada_nao_sobrescreve);
    RUN_TEST(test_insert_multiplas_chaves_distintas);
    RUN_TEST(test_insert_struct_generica);
    RUN_TEST(test_insert_payload_maximo_no_bucket);

    // Search
    RUN_TEST(test_search_encontra_valor_inserido);
    RUN_TEST(test_search_chave_inexistente_retorna_false);
    RUN_TEST(test_search_struct_campos_corretos);
    RUN_TEST(test_search_string_como_dado);
    RUN_TEST(test_search_apos_multiplas_insercoes);

    // Remove
    RUN_TEST(test_remove_retorna_dado_correto);
    RUN_TEST(test_remove_chave_inexistente_retorna_null);
    RUN_TEST(test_remove_impede_busca_posterior);
    RUN_TEST(test_remove_permite_reinsercao);
    RUN_TEST(test_remove_nao_corrompe_outros_registros);
    RUN_TEST(test_remove_alternado_e_insere);

    // Split / crescimento
    RUN_TEST(test_split_massivo_5000_insercoes);
    RUN_TEST(test_split_nao_perde_nenhum_dado);

    // Get all
    RUN_TEST(test_get_all_retorna_todos_os_registros);
    RUN_TEST(test_get_all_mapa_vazio);
    RUN_TEST(test_get_all_apos_remocoes);
    RUN_TEST(test_get_all_sem_duplicatas_apos_expand);

    // Persistência
    RUN_TEST(test_dado_persiste_apos_destroy_e_reopen);
    RUN_TEST(test_multiplos_dados_persistem);
    RUN_TEST(test_struct_persiste_com_todos_campos);
    RUN_TEST(test_persistencia_apos_split);

    // Edge cases
    RUN_TEST(test_chave_com_caracteres_especiais);
    RUN_TEST(test_chave_numerica_como_string);
    RUN_TEST(test_chave_longa);
    RUN_TEST(test_remover_unico_elemento_e_reinserir);
    RUN_TEST(test_valor_zero);
    RUN_TEST(test_valor_negativo);
    RUN_TEST(test_insere_remove_insere_mesma_chave_tres_vezes);

    return UNITY_END();
}
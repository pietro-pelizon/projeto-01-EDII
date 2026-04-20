#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../unity/Unity/src/unity.h"
#include "geo_handler.h"
#include "exhash.h"
#include "quadra.h"

exhash_t    *mapa_teste         = NULL;
const char  *ARQUIVO_TESTE_TEMP = "c1_mini_teste.geo";

void setUp(void) {
    mapa_teste = exhash_init("teste_quadras.hf", quadra_get_size(), 4096);

    FILE *f = fopen(ARQUIVO_TESTE_TEMP, "w");
    if (f != NULL) {
        fprintf(f, "cq 2.5 red blue\n");
        fprintf(f, "q cep1 10.0 20.0 100.0 50.0\n");
        fprintf(f, "cq 1.0 white black\n");
        fprintf(f, "q cep2 200.0 200.0 50.0 50.0\n");
        fclose(f);
    }
}

void tearDown(void) {
    if (mapa_teste != NULL) {
        exhash_destroy(mapa_teste);
        mapa_teste = NULL;
    }
    remove(ARQUIVO_TESTE_TEMP);
    remove("teste_quadras.hf");
}

void t_geo_handler_deve_ler_arquivo_e_inserir_no_hash(void) {
    geo_processar_arquivo(ARQUIVO_TESTE_TEMP, mapa_teste);

    quadra_t *q1 = malloc(quadra_get_size());
    quadra_t *q2 = malloc(quadra_get_size());

    bool achou1 = exhash_search(mapa_teste, "cep1", q1);
    TEST_ASSERT_TRUE_MESSAGE(achou1, "Quadra cep1 deve estar no banco");
    TEST_ASSERT_EQUAL_DOUBLE(10.0,   quadra_get_x(q1));
    TEST_ASSERT_EQUAL_DOUBLE(50.0,   quadra_get_h(q1));
    TEST_ASSERT_EQUAL_DOUBLE(2.5,    quadra_get_sw(q1));
    TEST_ASSERT_EQUAL_STRING("red",  quadra_get_corp(q1));
    TEST_ASSERT_EQUAL_STRING("blue", quadra_get_corb(q1));

    bool achou2 = exhash_search(mapa_teste, "cep2", q2);
    TEST_ASSERT_TRUE_MESSAGE(achou2, "Quadra cep2 deve estar no banco");
    TEST_ASSERT_EQUAL_DOUBLE(200.0,   quadra_get_x(q2));
    TEST_ASSERT_EQUAL_DOUBLE(1.0,     quadra_get_sw(q2));
    TEST_ASSERT_EQUAL_STRING("white", quadra_get_corp(q2));
    TEST_ASSERT_EQUAL_STRING("black", quadra_get_corb(q2));

    free(q1);
    free(q2);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(t_geo_handler_deve_ler_arquivo_e_inserir_no_hash);
    return UNITY_END();
}
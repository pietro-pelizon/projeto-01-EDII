#include <stdio.h>
#include <stdlib.h>

#include "quadra.h"
#include "exhash.h"
#include "geo_handler.h"
#include "habitante.h"
#include "pm_handler.h"
#include "../unity/Unity/src/unity.h"


exhash_t *mapa_teste_hab = NULL;
exhash_t *mapa_teste_quadras = NULL;
const char *arquivo_teste_hab = "arquivo_teste.pm";
const char *arquivo_teste_quadra = "arquivo_teste.geo";

void setUp(void) {
    mapa_teste_quadras = exhash_init("teste_quadras.hf", quadra_get_size(), 4096);

    FILE *f1 = fopen(arquivo_teste_quadra, "w");
    if (f1 != NULL) {
        fprintf(f1, "cq 2.5 red blue\n");
        fprintf(f1, "q cep1 10.0 20.0 100.0 50.0\n");
        fprintf(f1, "cq 1.0 white black\n");
        fprintf(f1, "q cep2 200.0 200.0 50.0 50.0\n");
        fprintf(f1, "q b04.2 300.0 300.0 80.0 60.0\n");
        fclose(f1);
    }

    geo_processar_arquivo(arquivo_teste_quadra, mapa_teste_quadras);


    mapa_teste_hab = exhash_init("teste_habitantes.hf", habitante_get_size(), 4096);

    FILE *f2 = fopen(arquivo_teste_hab, "w");
    if (f2 == NULL) return;

    fprintf(f2, "p 000.000.001-91 Karina_Ernestina Daalieff_Martino F 22/04/2020\n");
    fprintf(f2, "p 000.000.011-63 Valdir_Kees Abanasoff_Caputo M 26/12/2003\n");

    fprintf(f2, "m 000.000.011-63 b04.2 S 20 ap-1176\n");
    fclose(f2);
}

void tearDown(void) {
    if (mapa_teste_hab != NULL) {
        exhash_destroy(mapa_teste_quadras);
        exhash_destroy(mapa_teste_hab);
        mapa_teste_hab = NULL;
        mapa_teste_quadras = NULL;
    }

    remove(arquivo_teste_hab);
    remove(arquivo_teste_quadra);
    remove("teste_habitantes.hf");
    remove("teste_quadras.hf");
}

void test_ler_arquivo_e_modificar_habitante() {
    pm_processa_arquivo(arquivo_teste_hab, mapa_teste_hab, mapa_teste_quadras);

    habitante_t *hab1 = malloc(habitante_get_size());
    habitante_t *hab2 = malloc(habitante_get_size());

    bool achou1 = exhash_search(mapa_teste_hab, "000.000.001-91", hab1);
    TEST_ASSERT_TRUE_MESSAGE(achou1, "CPF 000.000.001-91 deve estar no hashfile");
    TEST_ASSERT_EQUAL_STRING("000.000.001-91", habitante_get_cpf(hab1));
    TEST_ASSERT_EQUAL_STRING("Karina_Ernestina", habitante_get_nome(hab1));
    TEST_ASSERT_EQUAL_STRING("Daalieff_Martino", habitante_get_sobrenome(hab1));
    TEST_ASSERT_EQUAL_CHAR('F', habitante_get_sexo(hab1));
    TEST_ASSERT_EQUAL_STRING("22/04/2020", habitante_get_data_nascimento(hab1));
    TEST_ASSERT_TRUE(habitante_is_sem_teto(hab1));

    bool achou2 = exhash_search(mapa_teste_hab, "000.000.011-63", hab2);
    TEST_ASSERT_TRUE_MESSAGE(achou2, "CPF 000.000.011-63 deve estar no hashfile");
    TEST_ASSERT_EQUAL_STRING("000.000.011-63", habitante_get_cpf(hab2));
    TEST_ASSERT_EQUAL_STRING("Valdir_Kees", habitante_get_nome(hab2));
    TEST_ASSERT_EQUAL_STRING("Abanasoff_Caputo", habitante_get_sobrenome(hab2));
    TEST_ASSERT_EQUAL_CHAR('M', habitante_get_sexo(hab2));
    TEST_ASSERT_EQUAL_STRING("26/12/2003", habitante_get_data_nascimento(hab2));
    TEST_ASSERT_FALSE(habitante_is_sem_teto(hab2));

    TEST_ASSERT_EQUAL_STRING("b04.2", habitante_get_cep(hab2));
    TEST_ASSERT_EQUAL_CHAR('S', habitante_get_face(hab2));
    TEST_ASSERT_EQUAL_INT(20, habitante_get_numero_casa(hab2));
    TEST_ASSERT_EQUAL_STRING("ap-1176", habitante_get_complemento(hab2));
    TEST_ASSERT_FALSE(habitante_is_sem_teto(hab2));
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ler_arquivo_e_modificar_habitante);
    return UNITY_END();
}






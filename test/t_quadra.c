#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../unity/Unity/src/unity.h"
#include "../include/quadra.h"

void setUp(void) {}
void tearDown(void) {}

void test_init_retorna_ponteiro_valido(void) {
    quadra_t *q = quadra_init("cep1", 10.0, 20.0, 100.0, 50.0);
    TEST_ASSERT_NOT_NULL_MESSAGE(q, "quadra_init deve retornar ponteiro valido");
    quadra_destroy(q);
}

void test_init_campos_geometricos_corretos(void) {
    quadra_t *q = quadra_init("cep1", 10.0, 20.0, 100.0, 50.0);
    TEST_ASSERT_EQUAL_DOUBLE(10.0,  quadra_get_x(q));
    TEST_ASSERT_EQUAL_DOUBLE(20.0,  quadra_get_y(q));
    TEST_ASSERT_EQUAL_DOUBLE(100.0, quadra_get_w(q));
    TEST_ASSERT_EQUAL_DOUBLE(50.0,  quadra_get_h(q));
    quadra_destroy(q);
}

void test_init_cep_gravado_corretamente(void) {
    quadra_t *q = quadra_init("abc123", 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_STRING("abc123", quadra_get_cep(q));
    quadra_destroy(q);
}

void test_init_contadores_zerados(void) {
    quadra_t *q = quadra_init("cep1", 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT(0, quadra_get_N(q));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_S(q));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_L(q));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_O(q));
    quadra_destroy(q);
}

void test_init_cep_no_limite(void) {
    quadra_t *q = quadra_init("123456789012345", 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_STRING("123456789012345", quadra_get_cep(q));
    quadra_destroy(q);
}

void test_set_x(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_x(q, 99.9);
    TEST_ASSERT_EQUAL_DOUBLE(99.9, quadra_get_x(q));
    quadra_destroy(q);
}

void test_set_y(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_y(q, 55.5);
    TEST_ASSERT_EQUAL_DOUBLE(55.5, quadra_get_y(q));
    quadra_destroy(q);
}

void test_set_w(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_w(q, 200.0);
    TEST_ASSERT_EQUAL_DOUBLE(200.0, quadra_get_w(q));
    quadra_destroy(q);
}

void test_set_h(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_h(q, 75.0);
    TEST_ASSERT_EQUAL_DOUBLE(75.0, quadra_get_h(q));
    quadra_destroy(q);
}

void test_set_cep(void) {
    quadra_t *q = quadra_init("velho", 0, 0, 0, 0);
    quadra_set_cep(q, "novo_cep");
    TEST_ASSERT_EQUAL_STRING("novo_cep", quadra_get_cep(q));
    quadra_destroy(q);
}

void test_set_sw(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_sw(q, 3.14);
    TEST_ASSERT_EQUAL_DOUBLE(3.14, quadra_get_sw(q));
    quadra_destroy(q);
}

void test_set_corp(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_corp(q, "red");
    TEST_ASSERT_EQUAL_STRING("red", quadra_get_corp(q));
    quadra_destroy(q);
}

void test_set_corb(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_corb(q, "blue");
    TEST_ASSERT_EQUAL_STRING("blue", quadra_get_corb(q));
    quadra_destroy(q);
}

void test_set_cq_aplica_todos_os_campos(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_cq(q, 2.5, "red", "blue");
    TEST_ASSERT_EQUAL_DOUBLE(2.5,    quadra_get_sw(q));
    TEST_ASSERT_EQUAL_STRING("red",  quadra_get_corp(q));
    TEST_ASSERT_EQUAL_STRING("blue", quadra_get_corb(q));
    quadra_destroy(q);
}

void test_set_cq_nao_confunde_corp_e_corb(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_cq(q, 1.0, "white", "black");
    TEST_ASSERT_EQUAL_STRING("white", quadra_get_corp(q));
    TEST_ASSERT_EQUAL_STRING("black", quadra_get_corb(q));
    // garante que não estão trocados
    TEST_ASSERT_NOT_EQUAL(0, strcmp(quadra_get_corp(q), quadra_get_corb(q)));
    quadra_destroy(q);
}

void test_set_cq_sobrescreve_valores_anteriores(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_cq(q, 1.0, "green", "yellow");
    quadra_set_cq(q, 5.0, "red",   "blue");
    TEST_ASSERT_EQUAL_DOUBLE(5.0,    quadra_get_sw(q));
    TEST_ASSERT_EQUAL_STRING("red",  quadra_get_corp(q));
    TEST_ASSERT_EQUAL_STRING("blue", quadra_get_corb(q));
    quadra_destroy(q);
}

void test_plus_count_side_norte(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'N');
    quadra_plus_count_side(q, 'N');
    TEST_ASSERT_EQUAL_INT(2, quadra_get_N(q));
    quadra_destroy(q);
}

void test_plus_count_side_sul(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'S');
    TEST_ASSERT_EQUAL_INT(1, quadra_get_S(q));
    quadra_destroy(q);
}

void test_plus_count_side_leste(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'L');
    TEST_ASSERT_EQUAL_INT(1, quadra_get_L(q));
    quadra_destroy(q);
}

void test_plus_count_side_oeste(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'O');
    TEST_ASSERT_EQUAL_INT(1, quadra_get_O(q));
    quadra_destroy(q);
}

void test_plus_count_side_minusculo(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'n');
    quadra_plus_count_side(q, 's');
    quadra_plus_count_side(q, 'l');
    quadra_plus_count_side(q, 'o');
    TEST_ASSERT_EQUAL_INT(1, quadra_get_N(q));
    TEST_ASSERT_EQUAL_INT(1, quadra_get_S(q));
    TEST_ASSERT_EQUAL_INT(1, quadra_get_L(q));
    TEST_ASSERT_EQUAL_INT(1, quadra_get_O(q));
    quadra_destroy(q);
}

void test_plus_count_side_nao_afeta_outros_lados(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'N');
    quadra_plus_count_side(q, 'N');
    quadra_plus_count_side(q, 'N');
    TEST_ASSERT_EQUAL_INT(3, quadra_get_N(q));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_S(q));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_L(q));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_O(q));
    quadra_destroy(q);
}

void test_set_hab_faces_define_todos_os_lados(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_set_hab_faces(q, 10, 20, 30, 40);
    TEST_ASSERT_EQUAL_INT(10, quadra_get_N(q));
    TEST_ASSERT_EQUAL_INT(20, quadra_get_S(q));
    TEST_ASSERT_EQUAL_INT(30, quadra_get_L(q));
    TEST_ASSERT_EQUAL_INT(40, quadra_get_O(q));
    quadra_destroy(q);
}

void test_set_hab_faces_sobrescreve_contadores_anteriores(void) {
    quadra_t *q = quadra_init("c", 0, 0, 0, 0);
    quadra_plus_count_side(q, 'N');
    quadra_plus_count_side(q, 'N');
    quadra_set_hab_faces(q, 5, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT(5, quadra_get_N(q));
    quadra_destroy(q);
}

void test_contadores_independentes_entre_instancias(void) {
    quadra_t *q1 = quadra_init("c1", 0, 0, 0, 0);
    quadra_t *q2 = quadra_init("c2", 0, 0, 0, 0);
    quadra_plus_count_side(q1, 'N');
    quadra_plus_count_side(q1, 'N');
    TEST_ASSERT_EQUAL_INT(2, quadra_get_N(q1));
    TEST_ASSERT_EQUAL_INT(0, quadra_get_N(q2));
    quadra_destroy(q1);
    quadra_destroy(q2);
}

void test_get_size_retorna_valor_positivo(void) {
    TEST_ASSERT_GREATER_THAN(0, (int)quadra_get_size());
}

void test_get_size_comporta_todos_os_campos(void) {
    TEST_ASSERT_GREATER_OR_EQUAL(96, (int)quadra_get_size());
}

int main(void) {
    UNITY_BEGIN();

    // Init
    RUN_TEST(test_init_retorna_ponteiro_valido);
    RUN_TEST(test_init_campos_geometricos_corretos);
    RUN_TEST(test_init_cep_gravado_corretamente);
    RUN_TEST(test_init_contadores_zerados);
    RUN_TEST(test_init_cep_no_limite);

    // Set/get geométrico
    RUN_TEST(test_set_x);
    RUN_TEST(test_set_y);
    RUN_TEST(test_set_w);
    RUN_TEST(test_set_h);
    RUN_TEST(test_set_cep);

    // Set/get aparência
    RUN_TEST(test_set_sw);
    RUN_TEST(test_set_corp);
    RUN_TEST(test_set_corb);
    RUN_TEST(test_set_cq_aplica_todos_os_campos);
    RUN_TEST(test_set_cq_nao_confunde_corp_e_corb);
    RUN_TEST(test_set_cq_sobrescreve_valores_anteriores);

    // Contadores
    RUN_TEST(test_plus_count_side_norte);
    RUN_TEST(test_plus_count_side_sul);
    RUN_TEST(test_plus_count_side_leste);
    RUN_TEST(test_plus_count_side_oeste);
    RUN_TEST(test_plus_count_side_minusculo);
    RUN_TEST(test_plus_count_side_nao_afeta_outros_lados);
    RUN_TEST(test_set_hab_faces_define_todos_os_lados);
    RUN_TEST(test_set_hab_faces_sobrescreve_contadores_anteriores);
    RUN_TEST(test_contadores_independentes_entre_instancias);

    // Size
    RUN_TEST(test_get_size_retorna_valor_positivo);
    RUN_TEST(test_get_size_comporta_todos_os_campos);

    return UNITY_END();
}
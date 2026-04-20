#include <stddef.h>
#include <stdbool.h>
#include "../unity/Unity/src/unity.h"
#include "../include/habitante.h"

habitante_t *h = NULL;

void setUp(void) {
    h = habitante_init("122.334.319-12", "pietro", "pelizon", 'M', "05/03/2006");
}

void tearDown(void) {
    if (h != NULL) {
        habitante_destroy(h);
        h = NULL;
    }
}

void t_habitante_init() {
    TEST_ASSERT_EQUAL_STRING("pietro", habitante_get_nome(h));
    TEST_ASSERT_EQUAL_STRING("pelizon", habitante_get_sobrenome(h));
    TEST_ASSERT_EQUAL_STRING("122.334.319-12", habitante_get_cpf(h));
    TEST_ASSERT_EQUAL_STRING("05/03/2006", habitante_get_data_nascimento(h));
    TEST_ASSERT_EQUAL_CHAR('M', habitante_get_sexo(h));

    TEST_ASSERT_TRUE(habitante_is_sem_teto(h));
}

void t_criou_endereco_nulo() {
    TEST_ASSERT_NULL(habitante_get_cep(h));
    TEST_ASSERT_NULL(habitante_get_complemento(h));
}

void t_habitante_setters_pessoais_devem_atualizar_dados() {
    habitante_set_nome(h, "Joao");
    habitante_set_sobrenome(h, "Silva");
    habitante_set_cpf(h, "999.888.777-66");
    habitante_set_data_nascimento(h, "10/10/2010");
    habitante_set_sexo(h, 'F');

    TEST_ASSERT_EQUAL_STRING("Joao", habitante_get_nome(h));
    TEST_ASSERT_EQUAL_STRING("Silva", habitante_get_sobrenome(h));
    TEST_ASSERT_EQUAL_STRING("999.888.777-66", habitante_get_cpf(h));
    TEST_ASSERT_EQUAL_STRING("10/10/2010", habitante_get_data_nascimento(h));
    TEST_ASSERT_EQUAL_CHAR('F', habitante_get_sexo(h));
}

void t_habitante_set_endereco_deve_atualizar_dados_e_remover_sem_teto() {
    habitante_set_endereco(h, "cep15", 'S', 45.5, "Apto 2");

    TEST_ASSERT_EQUAL_STRING("cep15", habitante_get_cep(h));
    TEST_ASSERT_EQUAL_CHAR('S', habitante_get_face(h));
    TEST_ASSERT_EQUAL_DOUBLE(45.5, habitante_get_numero_casa(h));
    TEST_ASSERT_EQUAL_STRING("Apto 2", habitante_get_complemento(h));

    TEST_ASSERT_FALSE(habitante_is_sem_teto(h));
}

void t_habitante_set_sem_teto_deve_funcionar() {
    habitante_set_endereco(h, "cep15", 'S', 45.5, "");
    TEST_ASSERT_FALSE(habitante_is_sem_teto(h));

    // Desaloja ele
    habitante_set_sem_teto(h, true);
    TEST_ASSERT_TRUE(habitante_is_sem_teto(h));
}

void t_habitante_setters_individuais_endereco() {
    habitante_set_sem_teto(h, false);

    habitante_set_cep(h, "cep99");
    habitante_set_face(h, 'L');
    habitante_set_numero_casa(h, 100.0);
    habitante_set_complemento(h, "Fundos");

    TEST_ASSERT_EQUAL_STRING("cep99", habitante_get_cep(h));
    TEST_ASSERT_EQUAL_CHAR('L', habitante_get_face(h));
    TEST_ASSERT_EQUAL_DOUBLE(100.0, habitante_get_numero_casa(h));
    TEST_ASSERT_EQUAL_STRING("Fundos", habitante_get_complemento(h));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(t_habitante_init);
    RUN_TEST(t_criou_endereco_nulo);
    RUN_TEST(t_habitante_setters_pessoais_devem_atualizar_dados);
    RUN_TEST(t_habitante_set_endereco_deve_atualizar_dados_e_remover_sem_teto);
    RUN_TEST(t_habitante_set_sem_teto_deve_funcionar);
    RUN_TEST(t_habitante_setters_individuais_endereco);
    return UNITY_END();

}
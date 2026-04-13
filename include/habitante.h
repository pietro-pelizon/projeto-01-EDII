#ifndef PROJETO_01_EDII_HABITANTE_H
#define PROJETO_01_EDII_HABITANTE_H
#include <stdbool.h>

typedef struct stHabitante habitante_t;

typedef struct stEndereco endereco_t;

habitante_t *habitante_init(char *cpf, char *nome, char *sobrenome, char sexo, char *data_nascimento);

void habitante_set_endereco(habitante_t *hab, char *cep, char face, double numero, char *complemento);

void habitante_set_cpf(habitante_t *hab, const char *cpf);

void habitante_set_nome(habitante_t *hab, const char *nome);

void habitante_set_sobrenome(habitante_t *hab, const char *sobrenome);

void habitante_set_sexo(habitante_t *hab, char sexo);

void habitante_set_data_nascimento(habitante_t *hab, const char *data);

void habitante_set_sem_teto(habitante_t *hab, bool status);

const char *habitante_get_cpf(const habitante_t *hab);

const char *habitante_get_nome(const habitante_t *hab);

const char *habitante_get_sobrenome(const habitante_t *hab);

char habitante_get_sexo(const habitante_t *hab);

const char *habitante_get_data_nascimento(const habitante_t *hab);

bool habitante_is_sem_teto(const habitante_t *hab);

void habitante_set_cep(habitante_t *hab, const char *cep);

void habitante_set_face(habitante_t *hab, char face);

void habitante_set_numero_casa(habitante_t *hab, double numero);

void habitante_set_complemento(habitante_t *hab, const char *complemento);

const char *habitante_get_cep(const habitante_t *hab);

char habitante_get_face(const habitante_t *hab);

double habitante_get_numero_casa(const habitante_t *hab);

const char *habitante_get_complemento(const habitante_t *hab);

#endif //PROJETO_01_EDII_HABITANTE_H

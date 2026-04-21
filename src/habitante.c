#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char cep[16];
    char face;
    double numero;
    char complemento[20];
} endereco_t;


typedef struct stHabitante {
    char cpf[16];
    char nome[20];
    char sobrenome[20];
    char sexo;
    char data_nascimento[12];
    bool sem_teto;

    endereco_t endereco;
} habitante_t;


size_t habitante_get_size() {
    return sizeof(habitante_t);
}

habitante_t *habitante_init(const char *cpf, const char *nome,
    const char *sobrenome, const char sexo, const char *data_de_nascimento) {

    habitante_t *novo = calloc(1,   sizeof(habitante_t));
    assert(novo != NULL);

    strncpy(novo -> cpf, cpf, 16);
    strncpy(novo -> nome, nome, 20);
    strncpy(novo -> sobrenome, sobrenome, 20);
    strncpy(novo -> data_nascimento, data_de_nascimento, 12);

    novo -> sexo = sexo;
    novo -> sem_teto = true;

    return novo;
}

void habitante_destroy(habitante_t *h) {
    assert(h != NULL);

    free(h);
}

void habitante_set_endereco(habitante_t *hab, char *cep, char face, double numero, char *complemento) {

    strncpy(hab -> endereco.cep, cep, 16);
    strncpy(hab -> endereco.complemento, complemento, 20);

    hab -> endereco.face = face;
    hab -> endereco.numero = numero;
    hab -> sem_teto = false;
}

// ==========================================
// SETTERS - HABITANTE
// ==========================================

void habitante_set_cpf(habitante_t *hab, const char *cpf) {
    if (hab == NULL || cpf == NULL) return;

    strncpy(hab -> cpf, cpf, 16);
    hab -> cpf[15] = '\0';
}

void habitante_set_nome(habitante_t *hab, const char *nome) {
    if (hab == NULL || nome == NULL) return;

    strncpy(hab -> nome, nome, 20);
    hab -> nome[19] = '\0';
}

void habitante_set_sobrenome(habitante_t *hab, const char *sobrenome) {
    if (hab == NULL || sobrenome == NULL) return;

    strncpy(hab -> sobrenome, sobrenome, 20);
    hab -> sobrenome[19] = '\0';
}

void habitante_set_sexo(habitante_t *hab, char sexo) {
    if (hab == NULL) return;
    hab -> sexo = sexo;
}

void habitante_set_data_nascimento(habitante_t *hab, const char *data) {
    if (hab == NULL || data == NULL) return;

    strncpy(hab -> data_nascimento, data, 12);
    hab -> data_nascimento[11] = '\0';
}

void habitante_set_sem_teto(habitante_t *hab, bool status) {
    if (hab == NULL) return;
    hab->sem_teto = status;
}

// ==========================================
// GETTERS - HABITANTE
// ==========================================

const char* habitante_get_cpf(const habitante_t *hab) {
    return (hab != NULL) ? hab -> cpf : NULL;
}

const char* habitante_get_nome(const habitante_t *hab) {
    return (hab != NULL) ? hab -> nome : NULL;
}

const char* habitante_get_sobrenome(const habitante_t *hab) {
    return (hab != NULL) ? hab -> sobrenome : NULL;
}

char habitante_get_sexo(const habitante_t *hab) {
    return (hab != NULL) ? hab -> sexo : '\0';
}

const char* habitante_get_data_nascimento(const habitante_t *hab) {
    return (hab != NULL) ? hab -> data_nascimento : NULL;
}

bool habitante_is_sem_teto(const habitante_t *hab) {
    return (hab != NULL) ? hab -> sem_teto : true;
}

// ==========================================
// SETTERS INDIVIDUAIS - ENDEREÇO
// ==========================================

void habitante_set_cep(habitante_t *hab, const char *cep) {
    if (hab == NULL || cep == NULL) return;

    strncpy(hab -> endereco.cep, cep, 16);
    hab -> endereco.cep[15] = '\0';
}

void habitante_set_face(habitante_t *hab, char face) {
    if (hab == NULL) return;
    hab -> endereco.face = face;
}

void habitante_set_numero_casa(habitante_t *hab, double numero) {
    if (hab == NULL) return;

    hab -> endereco.numero = numero;
}

void habitante_set_complemento(habitante_t *hab, const char *complemento) {
    if (hab == NULL || complemento == NULL) return;

    strncpy(hab -> endereco.complemento, complemento, 20);
    hab -> endereco.complemento[19] = '\0';
}

// ==========================================
// GETTERS - ENDEREÇO
// ==========================================

const char* habitante_get_cep(const habitante_t *hab) {
    if (hab == NULL || hab -> sem_teto == true) return NULL;
    return hab -> endereco.cep;
}

char habitante_get_face(const habitante_t *hab) {
    if (hab == NULL || hab -> sem_teto == true) return '\0';
    return hab -> endereco.face;
}

double habitante_get_numero_casa(const habitante_t *hab) {
    if (hab == NULL || hab -> sem_teto) return -1.0;
    return hab -> endereco.numero;
}

const char* habitante_get_complemento(const habitante_t *hab) {
    if (hab == NULL || hab -> sem_teto) return NULL;
    return hab -> endereco.complemento;
}

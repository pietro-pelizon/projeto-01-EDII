
#include <stddef.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "../include/exhash.h"

#include <stdlib.h>


typedef struct __attribute__((packed)) stQuadra {
    char cep[16];
    char corb[16];
    char corp[16];
    double x, y, w, h, sw;

    int qtd_moradores_N;
    int qtd_moradores_S;
    int qtd_moradores_L;
    int qtd_moradores_O;
} quadra_t;

quadra_t *quadra_init(const char *cep, double x, double y, double w, double h) {
    assert(cep != NULL);

    quadra_t *nova_quadra = calloc(1, sizeof(quadra_t));
    assert(nova_quadra != NULL);

    strncpy(nova_quadra -> cep, cep, 16);
    nova_quadra -> x = x;
    nova_quadra -> y = y;
    nova_quadra -> w = w;
    nova_quadra -> h = h;

    return nova_quadra;

}

void quadra_plus_count_side(quadra_t *q, char face) {

    switch (face) {
        case 'S':
        case 's': q -> qtd_moradores_S++; break;

        case 'N':
        case 'n': q -> qtd_moradores_N++; break;

        case 'O':
        case 'o': q -> qtd_moradores_O++; break;

        case 'L':
        case 'l': q -> qtd_moradores_L++; break;

        default: printf("lado inválido!\n"); break;
    }
}

void quadra_minus_count_side(quadra_t *q, char face) {

    switch (face) {
        case 'S':
        case 's': q -> qtd_moradores_S--; break;

        case 'N':
        case 'n': q -> qtd_moradores_N--; break;

        case 'O':
        case 'o': q -> qtd_moradores_O--; break;

        case 'L':
        case 'l': q -> qtd_moradores_L--; break;

        default: printf("lado inválido!\n"); break;
    }

}

void quadra_destroy(quadra_t *q) {

    free(q);
}

void quadra_set_hab_faces(quadra_t *q, int n, int s, int l, int o) {
    q -> qtd_moradores_N = n;
    q -> qtd_moradores_S = s;
    q -> qtd_moradores_L = l;
    q -> qtd_moradores_O = o;
}

void quadra_set_cep(quadra_t *q, const char *novo_cep) {
    strncpy(q -> cep, novo_cep, 16);
}

void quadra_set_x(quadra_t *q, double novo_x) {
    q -> x = novo_x;
}

void quadra_set_y(quadra_t *q, double novo_y) {
    q -> y = novo_y;
}

void quadra_set_w(quadra_t *q, double novo_w) {
    q -> w = novo_w;
}

void quadra_set_h(quadra_t *q, double novo_h) {
    q -> h = novo_h;
}

void quadra_set_corp(quadra_t *q, const char *nova_corp) {
    assert(q != NULL);

    strncpy(q -> corp, nova_corp, 16);
}

void quadra_set_corb(quadra_t *q, const char *nova_corb) {
    assert(q != NULL);

    strncpy(q -> corb, nova_corb, 16);
}

void quadra_set_sw(quadra_t *q, double novo_sw) {
    assert(q != NULL);

    q -> sw = novo_sw;
}

int quadra_get_N(const quadra_t *q) {
    assert(q != NULL);

    return q -> qtd_moradores_N;
}

int quadra_get_S(const quadra_t *q) {
    assert(q != NULL);

    return q -> qtd_moradores_S;
}

int quadra_get_L(const quadra_t *q) {
    assert(q != NULL);

    return q -> qtd_moradores_L;
}

int quadra_get_O(const quadra_t *q) {
    assert(q != NULL);

    return q -> qtd_moradores_O;
}

double quadra_get_x(const quadra_t *q) {
    assert(q != NULL);

    return q -> x;
}


double quadra_get_y(const quadra_t *q) {
    assert(q != NULL);

    return q -> y;
}

double quadra_get_w(const quadra_t *q) {
    assert(q != NULL);

    return q -> w;
}

double quadra_get_h(const quadra_t *q) {
    assert(q != NULL);

    return q -> h;
}

const char *quadra_get_corp(const quadra_t *q) {
    assert(q != NULL);

    return q -> corp;
}

const char *quadra_get_corb(const quadra_t *q) {
    assert(q != NULL);

    return q -> corb;
}

double quadra_get_sw(const quadra_t *q) {
    assert(q != NULL);

    return q -> sw;
}

const char *quadra_get_cep(const quadra_t *q) {
    assert(q != NULL);

    return q -> cep;
}

void quadra_set_cq(quadra_t *q, double sw, const char *corp, const char *corb) {
    assert(q != NULL);

    quadra_set_sw(q, sw);
    quadra_set_corb(q, corb);
    quadra_set_corp(q, corp);

}

size_t quadra_get_size() {
    return sizeof(quadra_t);
}


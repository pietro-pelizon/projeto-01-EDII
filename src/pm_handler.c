#include "../include/pm_handler.h"
#include "habitante.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "quadra.h"
#include <stdlib.h>

static void processa_comando_p(exhash_t *map, const char *linha_lida) {
    char cpf[16], nome[20], sobrenome[20], data_nascimento[12];
    char sexo;

    sscanf(linha_lida, "%*s %15s %19s %19s %c %11s", cpf, nome, sobrenome, &sexo, data_nascimento);

    habitante_t *novo = habitante_init(cpf, nome, sobrenome, sexo, data_nascimento);
    if (novo == NULL) return;

    if (!exhash_insert(map, novo, cpf)) {
        printf("Habitante CPF %s nao inserido! (Ja existe no banco)\n", habitante_get_cpf(novo));
    }

    habitante_destroy(novo);
}

static void processa_comando_m(exhash_t *mapa_quadras, exhash_t *mapa_habitantes, const char *linha_lida) {
    assert(mapa_habitantes != NULL);

    char cpf[16], cep[16], complemento[20] = "";
    char face;
    double num;



    sscanf(linha_lida, "%*s %15s %15s %c %lf %19[^\n]", cpf, cep, &face, &num, complemento);

    habitante_t *morador = (habitante_t *) exhash_remove(mapa_habitantes, cpf);
    quadra_t *quadra_buscada = malloc(quadra_get_size());

    if (!exhash_search(mapa_quadras, cep, quadra_buscada)) {
        free(quadra_buscada);
        return;
    }

    if (morador != NULL) {
        habitante_set_endereco(morador, cep, face, num, complemento);
        habitante_set_sem_teto(morador, false);

        exhash_insert(mapa_habitantes, morador, cpf);

        exhash_remove(mapa_quadras, cep);
        quadra_plus_count_side(quadra_buscada, face);
        exhash_insert(mapa_quadras, quadra_buscada, cep);

        habitante_destroy(morador);

        printf("Morador CPF %s mudou-se para o CEP %s!\n", cpf, cep);
    }

    else {
        printf("Aviso: Comando 'm' falhou. CPF %s não encontrado no banco.\n", cpf);
    }

    free(quadra_buscada);
}

void pm_processa_arquivo(const char *caminho_arquivo, exhash_t *mapa_habitantes, exhash_t *mapa_quadras) {
    assert(mapa_habitantes != NULL);

    FILE *pm = fopen(caminho_arquivo, "r");
    if (pm == NULL) {
        printf("Aviso: Erro ao abrir o arquivo .pm\n");
        return;
    }

    char linha[256];

    while (fgets(linha, sizeof(linha), pm)) {
        char comandos[3];

        sscanf(linha,"%2s", comandos);

        if (strcmp(comandos, "p") == 0) {
            processa_comando_p(mapa_habitantes, linha);
        }

        else if (strcmp(comandos, "m") == 0) {
            processa_comando_m(mapa_quadras, mapa_habitantes, linha);
        }
    }

    fclose(pm);
}
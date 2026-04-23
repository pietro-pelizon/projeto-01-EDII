#include "../include/qry_handler.h"

#include <assert.h>

#include "../include/habitante.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "exhash.h"
#include "quadra.h"
#include <stdlib.h>

#include "svg_handler.h"

static void coordenada_casa_habitante(char face, double *x, double *y, quadra_t *quadra, double numero);

// Varre todo o mapa de habitantes e encontra quem foi despejado em determinado CEP
static void full_table_scan_rq(exhash_t *mapa_pessoas, FILE *txt, const char *cep) {
    uint64_t total_pessoas;

    void **habitantes_gerais = exhash_get_all(mapa_pessoas, &total_pessoas);
    if (habitantes_gerais == NULL) return;


    for (uint64_t i = 0; i < total_pessoas; i++) {
        habitante_t *hab = habitantes_gerais[i];
        const char *cep_morador_atual = habitante_get_cep(hab);

        if (cep_morador_atual != NULL && strcmp(cep_morador_atual, cep) == 0) {
            fprintf(txt, "CPF: %15s | Nome: %19s\n",
                    habitante_get_cpf(hab), habitante_get_nome(hab));

            void *lixo = exhash_remove(mapa_pessoas, habitante_get_cpf(hab));
            free(lixo);

            habitante_set_endereco(hab, "", '\0', 0.0, "");
            habitante_set_sem_teto(hab, true);

            exhash_insert(mapa_pessoas, hab, habitante_get_cpf(hab));
        }

        habitante_destroy(hab);
    }

    free(habitantes_gerais);

}

// Processa comando rq - Remove quadra de CEP especificado
static void comando_rq(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg) {
    assert(mapa_pessoas != NULL);
    assert(mapa_quadras != NULL);
    assert(txt != NULL);
    assert(svg != NULL);

    char cep[20] = "";

    sscanf(linha_lida, "%*s %15s", cep);
    fprintf(txt, "[*] rq %s\n", cep);

    quadra_t *quadra_removida = exhash_remove(mapa_quadras, cep);
    if (quadra_removida == NULL) {
        printf("Aviso: quadra de CEP %s não encontrada pra remoção no comando rq\n", cep);
        fprintf(txt, "Quadra não encontrada!\n");
        return;
    }

    double x_quadra = quadra_get_x(quadra_removida);
    double y_quadra = quadra_get_y(quadra_removida);

    svg_x_vermelho(svg, x_quadra, y_quadra);

    printf("Quadra %s demolida com sucesso.\n", cep);
    quadra_destroy(quadra_removida);

    full_table_scan_rq(mapa_pessoas, txt, cep);
    fprintf(txt, "\n");
}

// Faz a contagem de do número de moradores por face
static void full_table_scan_pq(exhash_t *mapa_pessoas, const char *cep, int *n, int *s, int *l, int *o) {
    uint64_t total_pessoas;
    void **habitantes = exhash_get_all(mapa_pessoas, &total_pessoas);


    if (habitantes != NULL) {
        for (uint64_t i = 0; i < total_pessoas; i++) {
            habitante_t *hab = (habitante_t *) habitantes[i];
            const char *cep_morador = habitante_get_cep(hab);

            if (cep_morador != NULL && strcmp(cep_morador, cep) == 0) {
                char face = habitante_get_face(hab);
                if (face == 'N') *n++;
                else if (face == 'S') *s++;
                else if (face == 'L') *l++;
                else if (face == 'O') *o++;
            }
            habitante_destroy(hab);
        }
        free(habitantes);
    }
}

// Calcula o número de moradores de uma quadra com o CEP especificado (por face e total)
static void comando_pq(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg) {
    assert(mapa_pessoas != NULL);
    assert(mapa_pessoas != NULL);
    assert(svg != NULL);
    assert(txt != NULL);

    char cep[20] = "";

    sscanf(linha_lida, "%*s %15s", cep);

    fprintf(txt, "[*] pq %s\n", cep);

    quadra_t* quadra_procurada = malloc(quadra_get_size());

    if (!exhash_search(mapa_quadras, cep, quadra_procurada)) {
        fprintf(txt, "Quadra nao encontrada.\n\n");
        free(quadra_procurada);
        return;
    }

    int *n =0, *s = 0, *l = 0, *o = 0;

    full_table_scan_pq(mapa_pessoas, cep, n, s, l, o);

    quadra_set_hab_faces(quadra_procurada, n, s, l, o);

    svg_escrever_populacao_pq(svg, quadra_procurada);

    quadra_destroy(quadra_procurada);
    fprintf(txt, "\n");
}

// Produz diversas estatísticas sobre os habitantes da cidade
static void comando_censo(exhash_t *mapa_pessoas, FILE *txt) {
    assert(mapa_pessoas != NULL);
    assert(txt != NULL);

    fprintf(txt, "[*] censo\n");

    uint64_t total_habitantes;
    void **habitantes = exhash_get_all(mapa_pessoas, &total_habitantes);

    if (habitantes == NULL || total_habitantes == 0) {
        fprintf(txt, "A cidade esta vazia. Nao ha dados para o censo.\n\n");
        if (habitantes) free(habitantes);
        return;
    }

    int moradores = 0;
    int sem_teto = 0;
    int homens = 0;
    int mulheres = 0;

    int moradores_homens = 0;
    int moradores_mulheres = 0;
    int sem_teto_homens = 0;
    int sem_teto_mulheres = 0;

    for (uint64_t i = 0; i < total_habitantes; i++) {
        habitante_t *hab = (habitante_t *) habitantes[i];

        bool eh_sem_teto = habitante_is_sem_teto(hab);
        char sexo = habitante_get_sexo(hab);


        if (sexo == 'M') homens++;
        else if (sexo == 'F') mulheres++;

        if (eh_sem_teto) {
            sem_teto++;
            if (sexo == 'M') sem_teto_homens++;
            else if (sexo == 'F') sem_teto_mulheres++;
        } else {
            moradores++;
            if (sexo == 'M') moradores_homens++;
            else if (sexo == 'F') moradores_mulheres++;
        }

        habitante_destroy(hab);
    }
    free(habitantes);


    double prop_moradores_habitantes = ((double)moradores / total_habitantes);;

    double perc_hab_homens = ((double)homens / total_habitantes) * 100.0;
    double perc_hab_mulheres = ((double)mulheres / total_habitantes) * 100.0;

    // Proteção contra divisão por zero! (E se não tiver nenhum morador?)
    double perc_mor_homens = moradores > 0 ? ((double)moradores_homens / moradores) * 100.0 : 0.0;
    double perc_mor_mulheres = moradores > 0 ? ((double)moradores_mulheres / moradores) * 100.0 : 0.0;

    double perc_st_homens = sem_teto > 0 ? ((double)sem_teto_homens / sem_teto) * 100.0 : 0.0;
    double perc_st_mulheres = sem_teto > 0 ? ((double)sem_teto_mulheres / sem_teto) * 100.0 : 0.0;


    fprintf(txt, "Total de Habitantes: %lu\n", total_habitantes);
    fprintf(txt, "Total de Moradores: %d\n", moradores);
    fprintf(txt, "Proporção Moradores/Habitantes: %lf\n", prop_moradores_habitantes);
    fprintf(txt, "Total de Sem-Tetos: %d\n", sem_teto);
    fprintf(txt, "Total de homens: %d\n", homens);
    fprintf(txt, "Total de mulheres: %d\n", mulheres);
    fprintf(txt, "Porcentagem de habitantes homens: %lf\n", perc_hab_homens);
    fprintf(txt, "Porcentagem de habitantes mulheres: %lf\n", perc_hab_mulheres);
    fprintf(txt, "Porcentagem de moradores homens: %lf\n", perc_mor_homens);
    fprintf(txt, "Porcentagem de moradores mulheres: %lf\n", perc_mor_mulheres);
    fprintf(txt, "Porcentagem de sem-tetos homens: %lf\n", perc_st_homens);
    fprintf(txt, "Porcentagem de sem-tetos mulheres: %lf\n", perc_st_mulheres);
    fprintf(txt, "\n");
}

static void comando_h(const char *linha_lida, const exhash_t *mapa_pessoa, FILE *txt) {
    assert(mapa_pessoa != NULL);

    char cpf[20] = "";

    sscanf(linha_lida, "%*s %15s", cpf);

    habitante_t *hab_procurado = malloc(habitante_get_size());

    fprintf(txt, "[*] h? %s\n", cpf);

    bool found = exhash_search(mapa_pessoa, cpf, hab_procurado);
    if (!found) {
        fprintf(txt, "Habitante de CPF %s não encontrado no hashfile!\n", cpf);
        free(hab_procurado);
        return;
    }

    habitante_print_info(txt, hab_procurado);

    fprintf(txt, " -- Endereço --\n");
    bool sem_teto = habitante_is_sem_teto(hab_procurado);
    if (sem_teto) {
        fprintf(txt, "Habitante é sem-teto - nenhum endereço a exbir\n");
        fprintf(txt, "\n");
        habitante_destroy(hab_procurado);
        return;
    }

    habitante_endereco_print_info(txt, hab_procurado);
    fprintf(txt, "\n");

    habitante_destroy(hab_procurado);
}

void comando_nasc(const char *linha_lida, exhash_t *mapa_pessoas) {
    assert(mapa_pessoas != NULL);

    char nome[20] = "";
    char sobrenome[20] = "";
    char cpf[20] = "";
    char data_nascimento[20] = "";
    char sexo;

    sscanf(linha_lida, "%*s %15s %19s %19s %c %11s", cpf, nome, sobrenome, &sexo, data_nascimento);

    habitante_t *recem_nascido = habitante_init(cpf, nome, sobrenome, sexo, data_nascimento);
    if (recem_nascido == NULL) {
        printf("Erro Crítico: Falta de memoria ao tentar criar o habitante %s.\n", cpf);
        return;
    }

    bool inserido = exhash_insert(mapa_pessoas, recem_nascido, cpf);
    if (!inserido) {
        printf("Aviso: Falha no nascimento. O CPF %s ja existe no banco de dados!\n", cpf);
    } else {
        printf("Bem-vindo a Bitnopolis, %s %s!\n", nome, sobrenome);
    }

    habitante_destroy(recem_nascido);
}

static void comando_rip(const char *linha_lida, exhash_t *mapa_quadras, exhash_t *mapa_pessoas, FILE *txt, FILE *svg) {
    assert(mapa_quadras != NULL && mapa_pessoas != NULL && txt != NULL && svg != NULL);

    char cpf[20] = "";

    sscanf(linha_lida, "%*s %15s", cpf);

    fprintf(txt, "[*] rip %s\n", cpf);

    habitante_t *falecido = exhash_remove(mapa_pessoas, cpf);
    if (falecido == NULL) {
        printf("Habitante com CPF de número %s não encontrado no hashfile!\n", cpf);
        return;
    }

    habitante_print_info(txt, falecido);

    fprintf(txt, "-- Endereço --\n");
    bool sem_teto = habitante_is_sem_teto(falecido);
    if (sem_teto) {
        fprintf(txt, "Habitante sem-teto! Nenhuma informação a exibir\n");
        habitante_destroy(falecido);
        fprintf(txt, "\n");
        return;
    }

    habitante_endereco_print_info(txt, falecido);

    fprintf(txt, "\n");

    const char *cep = habitante_get_cep(falecido);
    char face = habitante_get_face(falecido);
    double numero = habitante_get_numero_casa(falecido);


    quadra_t *quadra_falecido = malloc(quadra_get_size());
    if (quadra_falecido == NULL) {
        printf("erro ao alocar memória para quadra em rip\n");
        return;
    }

    bool achou_quadra = exhash_search(mapa_quadras, cep, quadra_falecido);
    if (!achou_quadra) {
        return;
    }

    double cruz_x = 0;
    double cruz_y = 0;

    coordenada_casa_habitante(face, &cruz_x, &cruz_y, quadra_falecido, numero);
    svg_cruz_insert(svg, cruz_x, cruz_y);

    quadra_destroy(quadra_falecido);
    habitante_destroy(falecido);
}

static void comando_mud(const char *linha_lida, exhash_t *mapa_quadras, exhash_t *mapa_pessoas, FILE *svg) {
    assert (mapa_pessoas != NULL && mapa_quadras != NULL && svg != NULL);

    char cpf[20] = "";
    char cep[20] = "";
    char complemento[20] = "";
    char face;
    double numero = 0;

    sscanf(linha_lida, "%*s %15s %15s %c %lf %19s[^\n]", cpf, cep, &face, &numero, complemento);

    quadra_t *quadra_mudanca = exhash_remove(mapa_quadras, cep);
    if (quadra_mudanca == NULL) {
        printf("ERRO: Quadra de CEP %s não encontrada\n", cep);
        return;
    }

    habitante_t *hab_mudanca = exhash_remove(mapa_pessoas, cpf);
    if (hab_mudanca == NULL) {
        printf("ERRO: habitante de cpf %s não encontrado no hashfile!\n", cpf);
        exhash_insert(mapa_quadras, quadra_mudanca, cep);
        quadra_destroy(quadra_mudanca);
        return;
    }

    habitante_set_endereco(hab_mudanca, cep, face, numero, complemento);
    habitante_set_sem_teto(hab_mudanca, false);

    quadra_plus_count_side(quadra_mudanca, face);

    double ret_x = 0;
    double ret_y = 0;

    coordenada_casa_habitante(face, &ret_x, &ret_y, quadra_mudanca, numero);

    svg_marcador_mudanca(svg, ret_x, ret_y, cpf);

    exhash_insert(mapa_quadras, quadra_mudanca, cep);
    exhash_insert(mapa_pessoas, hab_mudanca, cpf);

    quadra_destroy(quadra_mudanca);
    habitante_destroy(hab_mudanca);
}

static void comando_dspj(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *svg, FILE *txt) {
    assert(mapa_pessoas != NULL && mapa_quadras != NULL && svg != NULL && txt != NULL);

    char cpf[20] = "";

    sscanf(linha_lida, "%*s %15s", cpf);
    fprintf(txt, "[*] dspj %s\n", cpf);



    habitante_t *despejado = exhash_remove(mapa_pessoas, cpf);
    if (despejado == NULL) {
        printf("ERRO: habitante de CPF %s não encontrado no hashfile!\n", cpf);
        return;
    }

    habitante_print_info(txt, despejado);

    fprintf(txt, "-- Endereço --\n");

    habitante_endereco_print_info(txt, despejado);
    fprintf(txt, "\n");

    const char *cep = habitante_get_cep(despejado);
    char face = habitante_get_face(despejado);
    double numero = habitante_get_numero_casa(despejado);

    quadra_t *quadra_despejado = exhash_remove(mapa_quadras, cep);
    if (quadra_despejado == NULL) {
        printf("ERRO: Quadra de CEP %s não encontrada no hashfile!\n", cep);
        exhash_insert(mapa_pessoas, despejado, cpf);
        habitante_destroy(despejado);
        return;
    }

    habitante_set_endereco(despejado, "", '\0', 0.0, "");
    habitante_set_sem_teto(despejado, true);

    double circ_x = 0;
    double circ_y = 0;

    coordenada_casa_habitante(face, &circ_x, &circ_y, quadra_despejado, numero);

    svg_circulo_despejo(svg, circ_x, circ_y);

    exhash_insert(mapa_pessoas, despejado, cpf);
    exhash_insert(mapa_quadras, quadra_despejado, cep);

    quadra_destroy(quadra_despejado);
    habitante_destroy(despejado);

}

void processa_qry(const char *caminho_qry, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg) {
    FILE *arquivo_qry = fopen(caminho_qry, "r");
    if (arquivo_qry == NULL) {
        printf("Erro ao abrir o %s.qry\n", caminho_qry);
        return;
    }

    char linha_leitura[256];

    while (fgets(linha_leitura, sizeof(linha_leitura), arquivo_qry)) {

        char comando[10];
        sscanf(linha_leitura, "%6s", comando);

        if (strcmp(comando, "rq") == 0) {
            comando_rq(linha_leitura, mapa_pessoas, mapa_quadras, txt, svg);
        }

        else if (strcmp(comando, "pq") == 0) {
            comando_pq(linha_leitura, mapa_pessoas, mapa_quadras, txt, svg);
        }

        else if (strcmp(comando, "censo") == 0) {
            comando_censo(mapa_pessoas, txt);
        }

        else if (strcmp(comando, "h?") == 0) {
            comando_h(linha_leitura, mapa_pessoas, txt);
        }

        else if (strcmp(comando, "nasc") == 0) {
            comando_nasc(linha_leitura, mapa_pessoas);
        }

        else if (strcmp(comando, "rip") == 0) {
            comando_rip(linha_leitura, mapa_quadras, mapa_pessoas, txt, svg);
        }

        else if (strcmp(comando, "mud") == 0) {
            comando_mud(linha_leitura, mapa_quadras, mapa_pessoas, svg);
        }

        else if (strcmp(comando, "dspj") == 0) {
            comando_dspj(linha_leitura, mapa_pessoas, mapa_quadras, svg, txt);
        }
    }

    fclose(arquivo_qry);
}

void coordenada_casa_habitante(char face, double *x, double *y, quadra_t *quadra, double numero) {
    double qx = quadra_get_x(quadra);
    double qy = quadra_get_y(quadra);
    double qw = quadra_get_w(quadra);
    double qh = quadra_get_h(quadra);


    switch (face) {
        case 'N': {
            *x = qx + numero;
            *y = qy;
            break;
        }
        case 'S': {
            *x = qx + numero;
            *y = qy + qh;
            break;
        }

        case 'O': {
            *x = qx;
            *y = qy + numero;
            break;
        }

        case 'L': {
            *x = qx + qw;
            *y = qy + numero;
            break;
        }

        default: printf("Face %c inválida!\n", face);

    }
}
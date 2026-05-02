#include "../include/qry_handler.h"
#include "../include/habitante.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "exhash.h"
#include "quadra.h"
#include "svg_handler.h"

typedef struct stCensoStats {
    uint64_t total_habitantes;
    int moradores;
    int sem_teto;
    int homens;
    int mulheres;
    int moradores_homens;
    int moradores_mulheres;
    int sem_teto_homens;
    int sem_teto_mulheres;
} censo_stats_t;

// ============================================================================
//                          DECLARAÇÕES (PROTÓTIPOS)
// ============================================================================

// --- Comandos ---
static void comando_rq(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg);
static void comando_pq(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg);
static void comando_censo(exhash_t *mapa_pessoas, FILE *txt);
static void comando_h(const char *linha_lida, const exhash_t *mapa_pessoa, FILE *txt);
static void comando_nasc(const char *linha_lida, exhash_t *mapa_pessoas, FILE *txt);
static void comando_rip(const char *linha_lida, exhash_t *mapa_quadras, exhash_t *mapa_pessoas, FILE *txt, FILE *svg);
static void comando_mud(const char *linha_lida, exhash_t *mapa_quadras, exhash_t *mapa_pessoas, FILE *svg, FILE *txt);
static void comando_dspj(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *svg, FILE *txt);

// --- Auxiliares ---
static void coordenada_casa_habitante(char face, double *x, double *y, quadra_t *quadra, double numero);
static bool full_table_scan_rq(exhash_t *mapa_pessoas, FILE *txt, const char *cep);
static void full_table_scan_pq(exhash_t *mapa_pessoas, const char *cep, int *n, int *s, int *l, int *o);
static bool calcular_estatisticas_censo(exhash_t *mapa_pessoas, censo_stats_t *stats);
static void imprimir_relatorio_censo(FILE *txt, const censo_stats_t *stats);
static void relatorio_obito_txt(FILE *txt, habitante_t *falecido);
static void desenhar_cruz_obito_svg(FILE *svg, exhash_t *mapa_quadras, habitante_t *falecido);
static void desenhar_mudanca_svg(FILE *svg, quadra_t *quadra_destino, char face, double numero, const char *cpf);
static void remover_habitante_quadra_antiga(exhash_t *mapa_quadras, habitante_t *hab);
static void relatorio_despejo_txt(FILE *txt, habitante_t *despejado);
static void desenhar_despejo_svg(FILE *svg, quadra_t *quadra_despejo, char face, double numero);


// ============================================================================
//                             FUNÇÃO PRINCIPAL
// ============================================================================

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
        } else if (strcmp(comando, "pq") == 0) {
            comando_pq(linha_leitura, mapa_pessoas, mapa_quadras, txt, svg);
        } else if (strcmp(comando, "censo") == 0) {
            comando_censo(mapa_pessoas, txt);
        } else if (strcmp(comando, "h?") == 0) {
            comando_h(linha_leitura, mapa_pessoas, txt);
        } else if (strcmp(comando, "nasc") == 0) {
            comando_nasc(linha_leitura, mapa_pessoas, txt);
        } else if (strcmp(comando, "rip") == 0) {
            comando_rip(linha_leitura, mapa_quadras, mapa_pessoas, txt, svg);
        } else if (strcmp(comando, "mud") == 0) {
            comando_mud(linha_leitura, mapa_quadras, mapa_pessoas, svg, txt);
        } else if (strcmp(comando, "dspj") == 0) {
            comando_dspj(linha_leitura, mapa_pessoas, mapa_quadras, svg, txt);
        }
    }

    fclose(arquivo_qry);
}


// ============================================================================
//                                COMANDOS
// ============================================================================

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
    double w_quadra = quadra_get_w(quadra_removida);
    double h_quadra = quadra_get_h(quadra_removida);

    svg_x_vermelho(svg, x_quadra, y_quadra, w_quadra, h_quadra);

    quadra_destroy(quadra_removida);

    bool quadra_tinha_moradores = full_table_scan_rq(mapa_pessoas, txt, cep);

    if (!quadra_tinha_moradores) {
        fprintf(txt, "Quadra sem moradores.\n");
    }

    fprintf(txt, "\n");
}

static void comando_pq(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg) {
    assert(mapa_pessoas != NULL);
    assert(mapa_quadras != NULL);
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

    int n = 0, s = 0, l = 0, o = 0;

    full_table_scan_pq(mapa_pessoas, cep, &n, &s, &l, &o);
    quadra_set_hab_faces(quadra_procurada, &n, &s, &l, &o);
    svg_escrever_populacao_pq(svg, quadra_procurada);

    quadra_destroy(quadra_procurada);
    fprintf(txt, "\n");
}

static void comando_censo(exhash_t *mapa_pessoas, FILE *txt) {
    assert(mapa_pessoas != NULL);
    assert(txt != NULL);

    fprintf(txt, "[*] censo\n");

    censo_stats_t stats = {0};

    if (!calcular_estatisticas_censo(mapa_pessoas, &stats)) {
        fprintf(txt, "A cidade esta vazia. Nao ha dados para o censo.\n\n");
        return;
    }

    imprimir_relatorio_censo(txt, &stats);
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
    habitante_endereco_print_info(txt, hab_procurado);

    fprintf(txt, "\n");

    habitante_destroy(hab_procurado);
}

static void comando_nasc(const char *linha_lida, exhash_t *mapa_pessoas, FILE *txt) {
    assert(mapa_pessoas != NULL);

    char nome[64] = "";
    char sobrenome[64] = "";
    char cpf[20] = "";
    char data_nascimento[20] = "";
    char sexo;

    sscanf(linha_lida, "%*s %15s %63s %63s %c %11s", cpf, nome, sobrenome, &sexo, data_nascimento);



    habitante_t *recem_nascido = habitante_init(cpf, nome, sobrenome, sexo, data_nascimento);
    if (recem_nascido == NULL) {
        printf("Erro Crítico: Falta de memoria ao tentar criar o habitante %s.\n", cpf);
        return;
    }

    fprintf(txt, "[*] nasc %s\n\n", habitante_get_cpf(recem_nascido));

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

    relatorio_obito_txt(txt, falecido);
    desenhar_cruz_obito_svg(svg, mapa_quadras, falecido);

    habitante_destroy(falecido);
}

static void comando_mud(const char *linha_lida, exhash_t *mapa_quadras, exhash_t *mapa_pessoas, FILE *svg, FILE *txt) {
    assert(mapa_pessoas != NULL && mapa_quadras != NULL && svg != NULL);

    char cpf[20] = "";
    char cep[20] = "";
    char complemento[20] = "";
    char face;
    double numero = 0;

    sscanf(linha_lida, "%*s %15s %15s %c %lf %19[^\n]", cpf, cep, &face, &numero, complemento);

    habitante_t *hab_mudanca = exhash_remove(mapa_pessoas, cpf);
    if (hab_mudanca == NULL) {
        printf("ERRO: Habitante de cpf %s não encontrado no hashfile!\n", cpf);
        return;
    }

    fprintf(txt, "[*] mud %s\n\n", habitante_get_cpf(hab_mudanca));

    quadra_t *quadra_destino = exhash_remove(mapa_quadras, cep);
    if (quadra_destino == NULL) {
        exhash_insert(mapa_pessoas, hab_mudanca, cpf);
        habitante_destroy(hab_mudanca);
        return;
    }

    remover_habitante_quadra_antiga(mapa_quadras, hab_mudanca);

    habitante_set_endereco(hab_mudanca, cep, face, numero, complemento);
    habitante_set_sem_teto(hab_mudanca, false);

    quadra_plus_count_side(quadra_destino, face);

    desenhar_mudanca_svg(svg, quadra_destino, face, numero, cpf);

    exhash_insert(mapa_quadras, quadra_destino, cep);
    exhash_insert(mapa_pessoas, hab_mudanca, cpf);

    quadra_destroy(quadra_destino);
    habitante_destroy(hab_mudanca);
}

static void comando_dspj(const char *linha_lida, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *svg, FILE *txt) {
    assert(mapa_pessoas != NULL && mapa_quadras != NULL && svg != NULL && txt != NULL);

    char cpf[20] = "";
    sscanf(linha_lida, "%*s %15s", cpf);

    fprintf(txt, "[*] dspj %s\n", cpf);

    habitante_t *despejado = exhash_remove(mapa_pessoas, cpf);
    if (despejado == NULL) {
        return;
    }

    if (habitante_is_sem_teto(despejado)) {
        habitante_print_info(txt, despejado);
        habitante_endereco_print_info(txt, despejado);
        exhash_insert(mapa_pessoas, despejado, cpf);
        habitante_destroy(despejado);
        return;
    }

    relatorio_despejo_txt(txt, despejado);

    const char *cep = habitante_get_cep(despejado);
    char face = habitante_get_face(despejado);
    double numero = habitante_get_numero_casa(despejado);

    quadra_t *quadra_despejo = exhash_remove(mapa_quadras, cep);
    if (quadra_despejo == NULL) {
        printf("ERRO: Quadra de CEP %s não encontrada no hashfile!\n", cep);
        exhash_insert(mapa_pessoas, despejado, cpf);
        habitante_destroy(despejado);
        return;
    }

    desenhar_despejo_svg(svg, quadra_despejo, face, numero);
    quadra_minus_count_side(quadra_despejo, face);

    habitante_set_endereco(despejado, "", '\0', 0.0, "");
    habitante_set_sem_teto(despejado, true);

    exhash_insert(mapa_pessoas, despejado, cpf);
    exhash_insert(mapa_quadras, quadra_despejo, cep);

    quadra_destroy(quadra_despejo);
    habitante_destroy(despejado);
}


// ============================================================================
//                             FUNÇÕES AUXILIARES
// ============================================================================

static void coordenada_casa_habitante(char face, double *x, double *y, quadra_t *quadra, double numero) {
    double qx = quadra_get_x(quadra);
    double qy = quadra_get_y(quadra);
    double qw = quadra_get_w(quadra);
    double qh = quadra_get_h(quadra);

    switch (face) {
        case 'S': {
            *x = qx + numero;
            *y = qy;
            break;
        }
        case 'N': {
            *x = qx + numero;
            *y = qy + qh;
            break;
        }
        case 'L': {
            *x = qx;
            *y = qy + numero;
            break;
        }
        case 'O': {
            *x = qx + qw;
            *y = qy + numero;
            break;
        }
        default: printf("Face %c inválida!\n", face);
    }
}

static bool full_table_scan_rq(exhash_t *mapa_pessoas, FILE *txt, const char *cep) {
    uint64_t total_pessoas;
    void **habitantes_gerais = exhash_get_all(mapa_pessoas, &total_pessoas);
    if (habitantes_gerais == NULL) return false;

        bool encontrou_alguem = false;


    for (uint64_t i = 0; i < total_pessoas; i++) {
        habitante_t *hab = habitantes_gerais[i];
        const char *cep_morador_atual = habitante_get_cep(hab);


        if (cep_morador_atual != NULL && strcmp(cep_morador_atual, cep) == 0) {
            fprintf(txt, "CPF: %s | Nome: %s\n",
                    habitante_get_cpf(hab), habitante_get_nome(hab));

            encontrou_alguem = true;

            void *lixo = exhash_remove(mapa_pessoas, habitante_get_cpf(hab));
            if (lixo) {
                habitante_destroy((habitante_t *)lixo);
            }

            habitante_set_endereco(hab, "", '\0', 0.0, "");
            habitante_set_sem_teto(hab, true);

            exhash_insert(mapa_pessoas, hab, habitante_get_cpf(hab));
        }
        habitante_destroy(hab);
    }
    free(habitantes_gerais);

    return encontrou_alguem;
}

static void full_table_scan_pq(exhash_t *mapa_pessoas, const char *cep, int *n, int *s, int *l, int *o) {
    uint64_t total_pessoas;
    void **habitantes = exhash_get_all(mapa_pessoas, &total_pessoas);

    if (habitantes != NULL) {
        for (uint64_t i = 0; i < total_pessoas; i++) {
            habitante_t *hab = (habitante_t *) habitantes[i];
            const char *cep_morador = habitante_get_cep(hab);

            if (cep_morador != NULL && strcmp(cep_morador, cep) == 0) {
                char face = habitante_get_face(hab);
                if (face == 'N') (*n)++;
                else if (face == 'S') (*s)++;
                else if (face == 'L') (*l)++;
                else if (face == 'O') (*o)++;
            }
            habitante_destroy(hab);
        }
        free(habitantes);
    }
}

static bool calcular_estatisticas_censo(exhash_t *mapa_pessoas, censo_stats_t *stats) {
    void **habitantes = exhash_get_all(mapa_pessoas, &stats->total_habitantes);

    if (habitantes == NULL || stats->total_habitantes == 0) {
        if (habitantes) free(habitantes);
        return false;
    }

    for (uint64_t i = 0; i < stats->total_habitantes; i++) {
        habitante_t *hab = (habitante_t *) habitantes[i];

        bool eh_sem_teto = habitante_is_sem_teto(hab);
        char sexo = habitante_get_sexo(hab);

        if (sexo == 'M') stats->homens++;
        else if (sexo == 'F') stats->mulheres++;

        if (eh_sem_teto) {
            stats->sem_teto++;
            if (sexo == 'M') stats->sem_teto_homens++;
            else if (sexo == 'F') stats->sem_teto_mulheres++;
        } else {
            stats->moradores++;
            if (sexo == 'M') stats->moradores_homens++;
            else if (sexo == 'F') stats->moradores_mulheres++;
        }

        habitante_destroy(hab);
    }

    free(habitantes);
    return true;
}

static void imprimir_relatorio_censo(FILE *txt, const censo_stats_t *stats) {
    // Cast para (double) para evitar o Narrowing Conversion do uint64_t e divisao correta
    double prop_moradores = ((double)stats->moradores / (double)stats->total_habitantes);

    double perc_hab_homens = ((double)stats->homens / (double)stats->total_habitantes) * 100.0;
    double perc_hab_mulheres = ((double)stats->mulheres / (double)stats->total_habitantes) * 100.0;

    double perc_mor_homens = stats->moradores > 0 ? ((double)stats->moradores_homens / stats->moradores) * 100.0 : 0.0;
    double perc_mor_mulheres = stats->moradores > 0 ? ((double)stats->moradores_mulheres / stats->moradores) * 100.0 : 0.0;

    double perc_st_homens = stats->sem_teto > 0 ? ((double)stats->sem_teto_homens / stats->sem_teto) * 100.0 : 0.0;
    double perc_st_mulheres = stats->sem_teto > 0 ? ((double)stats->sem_teto_mulheres / stats->sem_teto) * 100.0 : 0.0;

    fprintf(txt, "Total de Habitantes: %lu\n", stats->total_habitantes);
    fprintf(txt, "Total de Moradores: %d\n", stats->moradores);
    fprintf(txt, "Proporção Moradores/Habitantes: %.4lf\n", prop_moradores);
    fprintf(txt, "Total de Sem-Tetos: %d\n", stats->sem_teto);
    fprintf(txt, "Total de homens: %d\n", stats->homens);
    fprintf(txt, "Total de mulheres: %d\n", stats->mulheres);
    fprintf(txt, "Porcentagem de habitantes homens: %.2lf%%\n", perc_hab_homens);
    fprintf(txt, "Porcentagem de habitantes mulheres: %.2lf%%\n", perc_hab_mulheres);
    fprintf(txt, "Porcentagem de moradores homens: %.2lf%%\n", perc_mor_homens);
    fprintf(txt, "Porcentagem de moradores mulheres: %.2lf%%\n", perc_mor_mulheres);
    fprintf(txt, "Porcentagem de sem-tetos homens: %.2lf%%\n", perc_st_homens);
    fprintf(txt, "Porcentagem de sem-tetos mulheres: %.2lf%%\n", perc_st_mulheres);
    fprintf(txt, "\n");
}

static void relatorio_obito_txt(FILE *txt, habitante_t *falecido) {
    habitante_print_info(txt, falecido);
    habitante_endereco_print_info(txt, falecido);
    fprintf(txt, "\n");
}

static void desenhar_cruz_obito_svg(FILE *svg, exhash_t *mapa_quadras, habitante_t *falecido) {
    if (habitante_is_sem_teto(falecido)) {
        return;
    }

    const char *cep = habitante_get_cep(falecido);
    char face = habitante_get_face(falecido);
    double numero = habitante_get_numero_casa(falecido);

    quadra_t *quadra_falecido = calloc(1, quadra_get_size());
    if (quadra_falecido == NULL) {
        printf("Erro ao alocar memória para quadra em rip\n");
        return;
    }

    if (exhash_search(mapa_quadras, cep, quadra_falecido)) {
        double cruz_x = 0.0;
        double cruz_y = 0.0;
        coordenada_casa_habitante(face, &cruz_x, &cruz_y, quadra_falecido, numero);
        svg_cruz_insert(svg, cruz_x, cruz_y);

        quadra_minus_count_side(quadra_falecido, face);
        exhash_insert(mapa_quadras, quadra_falecido, cep);
    }

    quadra_destroy(quadra_falecido);
}

static void desenhar_mudanca_svg(FILE *svg, quadra_t *quadra_destino, char face, double numero, const char *cpf) {
    double ret_x = 0;
    double ret_y = 0;

    coordenada_casa_habitante(face, &ret_x, &ret_y, quadra_destino, numero);
    svg_marcador_mudanca(svg, ret_x, ret_y, cpf);
}

static void remover_habitante_quadra_antiga(exhash_t *mapa_quadras, habitante_t *hab) {
    if (habitante_is_sem_teto(hab)) {
        return;
    }

    const char *cep_antigo = habitante_get_cep(hab);
    char face_antiga = habitante_get_face(hab);

    quadra_t *quadra_antiga = exhash_remove(mapa_quadras, cep_antigo);
    if (quadra_antiga != NULL) {
        quadra_minus_count_side(quadra_antiga, face_antiga);
        exhash_insert(mapa_quadras, quadra_antiga, cep_antigo);
        quadra_destroy(quadra_antiga);
    }
}

static void relatorio_despejo_txt(FILE *txt, habitante_t *despejado) {
    habitante_print_info(txt, despejado);
    habitante_endereco_print_info(txt, despejado);
    fprintf(txt, "\n");
}

static void desenhar_despejo_svg(FILE *svg, quadra_t *quadra_despejo, char face, double numero) {
    double circ_x = 0;
    double circ_y = 0;

    coordenada_casa_habitante(face, &circ_x, &circ_y, quadra_despejo, numero);
    svg_circulo_despejo(svg, circ_x, circ_y);
}
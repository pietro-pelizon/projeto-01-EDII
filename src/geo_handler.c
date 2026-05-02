#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "exhash.h"
#include "quadra.h"

#include "geo_handler.h"


// se lê 'cq', preenche as variáveis corretas de espessura, cor de preenchimento e cor de borda
static void processa_comando_cq(const char *linha_lida, char *sw_atual, char *corp_atual, char *corb_atual) {
    sscanf(linha_lida, "%*s %11s %19s %19s", sw_atual, corp_atual, corb_atual);
}

// se lê 'q', adiciona nova quadra com determinados atributos ao seu respectivo hashfile.hf
static void processa_comando_q(const char *linha_lida, exhash_t *mapa_quadras,
                        char *sw_atual, const char *corp_atual, const char *corb_atual) {
    char cep[16];
    double x, y, w, h;

    sscanf(linha_lida, "%*s %15s %lf %lf %lf %lf", cep, &x, &y, &w, &h);

    quadra_t *nova_quadra = quadra_init(cep, x, y, w, h);
    if (nova_quadra == NULL) return;

    quadra_set_cq(nova_quadra, sw_atual, corp_atual, corb_atual);

    if (!exhash_insert(mapa_quadras, nova_quadra, cep)) {
        printf("Aviso: Quadra CEP %s ignorada (ja existe).\n", cep);
    }

    quadra_destroy(nova_quadra);
}

// função principal que processa o arquivo (.geo) e retorna seu hashfile.hf preenchido
exhash_t *processa_geo(const char *caminho_geo) {
    FILE *arquivo_geo = fopen(caminho_geo, "r");
    if (!arquivo_geo) {
        printf("Erro ao abrir %s\n", caminho_geo);
        return NULL;
    }

    exhash_t *mapa_quadras = exhash_init("hashfile_quadras.hf", quadra_get_size(), 4096);

    char cor_preenchimento[20] = "white";
    char cor_borda[20] = "black";
    char espessura_borda[20] = "1.0px";

    char linha_leitura[256];

    while (fgets(linha_leitura, sizeof(linha_leitura), arquivo_geo)) {

        char comando[3];
        sscanf(linha_leitura, "%2s", comando);

        if (strcmp(comando, "cq") == 0) {
            processa_comando_cq(linha_leitura, espessura_borda, cor_preenchimento, cor_borda);
        }
        else if (strcmp(comando, "q") == 0) {
            processa_comando_q(linha_leitura, mapa_quadras, espessura_borda, cor_preenchimento, cor_borda);
        }
    }

    fclose(arquivo_geo);

    return mapa_quadras;
}
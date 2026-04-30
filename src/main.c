#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exhash.h"
#include "geo_handler.h"
#include "pm_handler.h"
#include "qry_handler.h"
#include "svg_handler.h"

#define PATH_SIZE 520

static void monta_caminho(char* path_completo, const char* base_dir, const char* nome_arquivo) {
    if (base_dir != NULL && strlen(base_dir) > 0) {
        sprintf(path_completo, "%s/%s", base_dir, nome_arquivo);
    } else {
        strcpy(path_completo, nome_arquivo);
    }
}

static void extrair_nome_base(const char *caminho, char *nome_base) {
    const char *ultimo_slash = strrchr(caminho, '/');
    strcpy(nome_base, ultimo_slash ? ultimo_slash + 1 : caminho);

    char* ponto_ext = strrchr(nome_base, '.');
    if (ponto_ext) {
        *ponto_ext = '\0';
    }
}

int main(int argc, char *argv[]) {
    char *path_entrada = NULL;
    char *path_saida = NULL;
    char *path_pm = NULL;
    char *path_geo = NULL;
    char *path_qry = NULL;

    remove("hashfile_quadras.hf");       // Deleta o arquivo velho (se existir) evitando conflito entre dados dos testes
    remove("hashfile_habitantes.hf");

    // 1. Leitura dos argumentos
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) path_entrada = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) path_saida = argv[++i];
        else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) path_qry = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) path_geo = argv[++i];
        else if (strcmp(argv[i], "-pm") == 0 && i + 1 < argc) path_pm = argv[++i];
    }

    // 3. Checagem de parâmetros obrigatórios
    if (path_geo == NULL || path_saida == NULL) {
        fprintf(stderr, "ERRO: Parâmetros obrigatórios -f <arquivo.geo> ou -o <dir_saida> faltando!\n");
        return 1;
    }

    // 4. Processamento do arquivo (.geo)
    char path_geo_completo[PATH_SIZE];
    monta_caminho(path_geo_completo, path_entrada, path_geo);

    char nome_base_geo[256];
    extrair_nome_base(path_geo, nome_base_geo);

    printf("[*] Processando mapa base: %s\n", path_geo_completo);
    exhash_t *hashfile_quadras = processa_geo(path_geo_completo);

    if (hashfile_quadras == NULL) {
        fprintf(stderr, "ERRO: falha crítica ao processar .geo\n");
        return 1;
    }

    // 5. Geração do arquivo (.svg) inicial após ler apenas o (.geo)
    char path_svg_geo[PATH_SIZE];
    sprintf(path_svg_geo, "%s/%s.svg", path_saida, nome_base_geo);

    FILE *svg_geo_file = svg_init(path_svg_geo);
    if (svg_geo_file) {
        // Percorre o hash e desenha as quadras
        svg_desenha_mapa_base(svg_geo_file, hashfile_quadras);
        fecha_svg(svg_geo_file);
        printf("[*] Primeiro SVG gerado com sucesso: %s\n", path_svg_geo);
    }

    // 6. Processamento do arquivo (.pm)
    exhash_t *hashfile_habitantes = NULL;
    if (path_pm != NULL) {
        char path_pm_completo[PATH_SIZE];
        monta_caminho(path_pm_completo, path_entrada, path_pm);

        printf("[*] Inserindo populacao: %s\n", path_pm_completo);
        hashfile_habitantes = pm_processa_arquivo(path_pm_completo, hashfile_quadras);
    }

    // 7. Processamento do arquivo (.qry)
    if (path_qry != NULL && hashfile_habitantes != NULL) {
        char path_qry_completo[PATH_SIZE];
        monta_caminho(path_qry_completo, path_entrada, path_qry);

        char nome_base_qry[256];
        extrair_nome_base(path_qry, nome_base_qry);

        char path_txt_out[PATH_SIZE];
        char path_svg_out[PATH_SIZE];
        snprintf(path_txt_out, PATH_SIZE, "%s/%s-%s.txt", path_saida, nome_base_geo, nome_base_qry);
        snprintf(path_svg_out, PATH_SIZE, "%s/%s-%s.svg", path_saida, nome_base_geo, nome_base_qry);

        FILE *txt_file = fopen(path_txt_out, "w");
        FILE *svg_qry_file = svg_init(path_svg_out);

        if (txt_file && svg_qry_file) {
            printf("[*] Executando consultas do QRY: %s\n", path_qry_completo);
            svg_desenha_mapa_base(svg_qry_file, hashfile_quadras);
            processa_qry(path_qry_completo, hashfile_habitantes, hashfile_quadras, txt_file, svg_qry_file);

            fecha_svg(svg_qry_file);
            fclose(txt_file);
            printf("[*] Relatorios finais gerados: %s e %s\n", path_txt_out, path_svg_out);
        }
    }

    if (hashfile_quadras) exhash_destroy(hashfile_quadras);
    if (hashfile_habitantes) exhash_destroy(hashfile_habitantes);

    printf("Programa encerrado com sucesso!.\n");
    return 0;
}
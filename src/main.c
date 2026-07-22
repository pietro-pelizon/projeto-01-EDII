#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exhash.h"
#include "geo_handler.h"
#include "pm_handler.h"
#include "qry_handler.h"
#include "svg_handler.h"

#define PATH_SIZE 530

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

static void print_help();


int main(int argc, char *argv[]) {
    char *path_entrada = NULL;
    char *path_saida = NULL;
    char *path_pm = NULL;
    char *path_geo = NULL;
    char *path_qry = NULL;

    // 1. Leitura dos argumentos
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) path_entrada = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) path_saida = argv[++i];
        else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) path_qry = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) path_geo = argv[++i];
        else if (strcmp(argv[i], "-pm") == 0 && i + 1 < argc) path_pm = argv[++i];
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        }
    }

    // 2. Checagem de parâmetros obrigatórios
    if (path_geo == NULL || path_saida == NULL) {
        fprintf(stderr, "ERRO: Parametros obrigatorios -f <arquivo.geo> ou -o <dir_saida> faltando!\n");
        return 1;
    }

    // 3. Extração dos nomes base (Trazemos o (.qry) pra cá para usar nos nomes dos arquivos)
    char nome_base_geo[256] = "";
    extrair_nome_base(path_geo, nome_base_geo);

    char nome_base_qry[256] = "";
    if (path_qry != NULL) {
        extrair_nome_base(path_qry, nome_base_qry);
    }

    // 4. Monta os caminhos exatos dos Bancos de Dados (.hf)
    char hf_quadras[PATH_SIZE];
    char hf_hab[PATH_SIZE];

    if (path_qry != NULL) {
        snprintf(hf_quadras, PATH_SIZE, "%s/%s-%s-quadras.hf", path_saida, nome_base_geo, nome_base_qry);
        snprintf(hf_hab, PATH_SIZE, "%s/%s-%s-habitantes.hf", path_saida, nome_base_geo, nome_base_qry);
    } else {
        snprintf(hf_quadras, PATH_SIZE, "%s/%s-quadras.hf", path_saida, nome_base_geo);
        snprintf(hf_hab, PATH_SIZE, "%s/%s-habitantes.hf", path_saida, nome_base_geo);
    }

    // 5. Processamento do arquivo (.geo)
    char path_geo_completo[PATH_SIZE];
    monta_caminho(path_geo_completo, path_entrada, path_geo);

    printf("[*] Processando mapa base: %s\n", path_geo_completo);

    exhash_t *hashfile_quadras = processa_geo(path_geo_completo, hf_quadras);

    if (hashfile_quadras == NULL) {
        fprintf(stderr, "ERRO: falha critica ao processar .geo\n");
        return 1;
    }

    // 6. Geração do arquivo (.svg) inicial após ler apenas o (.geo)
    char path_svg_geo[PATH_SIZE];
    sprintf(path_svg_geo, "%s/%s.svg", path_saida, nome_base_geo);

    FILE *svg_geo_file = svg_init(path_svg_geo);
    if (svg_geo_file) {
        svg_desenha_mapa_base(svg_geo_file, hashfile_quadras);
        fecha_svg(svg_geo_file);
        printf("[*] Primeiro (.svg) gerado com sucesso: %s\n", path_svg_geo);
    }

    // 7. Processamento do arquivo (.pm)
    exhash_t *hashfile_habitantes = NULL;
    if (path_pm != NULL) {
        char path_pm_completo[PATH_SIZE];
        monta_caminho(path_pm_completo, path_entrada, path_pm);

        printf("[*] Inserindo populacao: %s\n", path_pm_completo);

        hashfile_habitantes = pm_processa_arquivo(path_pm_completo, hashfile_quadras, hf_hab);
    }

    // 8. Processamento do arquivo (.qry)
    if (path_qry != NULL && hashfile_habitantes != NULL) {
        char path_qry_completo[PATH_SIZE];
        monta_caminho(path_qry_completo, path_entrada, path_qry);

        char path_txt_out[PATH_SIZE];
        char path_svg_out[PATH_SIZE];
        snprintf(path_txt_out, PATH_SIZE, "%s/%s-%s.txt", path_saida, nome_base_geo, nome_base_qry);
        snprintf(path_svg_out, PATH_SIZE, "%s/%s-%s.svg", path_saida, nome_base_geo, nome_base_qry);

        FILE *txt_file = fopen(path_txt_out, "w");
        FILE *svg_qry_file = svg_init(path_svg_out);

        if (txt_file && svg_qry_file) {
            printf("[*] Executando consultas do (.qry): %s\n", path_qry_completo);
            processa_qry(path_qry_completo, hashfile_habitantes, hashfile_quadras, txt_file, svg_qry_file);

            fecha_svg(svg_qry_file);
            fclose(txt_file);
        }
    }

    // 9. DUMP FINAL (.hfd)
    char nome_dump_hab[PATH_SIZE];
    char nome_dump_quadras[PATH_SIZE];

    if (path_qry != NULL) {
        snprintf(nome_dump_hab, PATH_SIZE, "%s/%s-%s-habitantes.hfd", path_saida, nome_base_geo, nome_base_qry);
        snprintf(nome_dump_quadras, PATH_SIZE, "%s/%s-%s-quadras.hfd", path_saida, nome_base_geo, nome_base_qry);
    } else {
        snprintf(nome_dump_hab, PATH_SIZE, "%s/%s-habitantes.hfd", path_saida, nome_base_geo);
        snprintf(nome_dump_quadras, PATH_SIZE, "%s/%s-quadras.hfd", path_saida, nome_base_geo);
    }

    exhash_dump(hashfile_habitantes, nome_dump_hab);
    exhash_dump(hashfile_quadras, nome_dump_quadras);

    // 10. Limpeza
    if (hashfile_quadras) exhash_destroy(hashfile_quadras);
    if (hashfile_habitantes) exhash_destroy(hashfile_habitantes);

    printf("Programa encerrado com sucesso!\n");
    return 0;
}

static void print_help() {
    printf("Uso: ted [OPÇÕES]\n\n");
    printf("Opções:\n");
    printf("  -f <arquivo.geo>   (obrigatório) Arquivo com as quadras da cidade\n");
    printf("  -o <dir>           (obrigatório) Diretório de saída\n");
    printf("  -e <dir>           (opcional)    Diretório base dos testes\n");
    printf("  -q <arquivo.qry>   (opcional)    Arquivo de consultas\n");
    printf("  -pm <arquivo.pm>   (opcional)    Arquivo com a população da cidade\n");
    printf("  -h, --help                       Exibe esta mensagem\n\n");
    printf("Exemplo:\n");
    printf("  ted -e entrada/ -f formas.geo -q consultas.qry -o saida/ -pm habitante.pm\n");
}
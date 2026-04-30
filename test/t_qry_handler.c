#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity_internals.h"
#include "unity.h"
#include "exhash.h"
#include "quadra.h"
#include "habitante.h"
#include "geo_handler.h"
#include "pm_handler.h"
#include "qry_handler.h"

static const char *ARQ_GEO   = "t_qry.geo";
static const char *ARQ_PM    = "t_qry.pm";
static const char *ARQ_QRY   = "t_qry.qry";
static const char *ARQ_TXT   = "t_qry_out.txt";
static const char *ARQ_SVG   = "t_qry_out.svg";
static const char *HF_QUAD   = "hashfile_quadras.hf";
static const char *HF_HAB    = "hashfile_habitantes.hf";

static exhash_t *mapa_quadras  = NULL;
static exhash_t *mapa_pessoas  = NULL;
static FILE     *txt           = NULL;
static FILE     *svg           = NULL;


static void escreve_geo_basico(void) {
    FILE *f = fopen(ARQ_GEO, "w");
    fprintf(f, "cq 2.0 orange black\n");
    fprintf(f, "q cep1 10.0 20.0 100.0 50.0\n");
    fprintf(f, "q cep2 200.0 200.0 80.0 60.0\n");
    fprintf(f, "q cep3 400.0 100.0 60.0 40.0\n");
    fclose(f);
}

static void escreve_pm_basico(void) {
    FILE *f = fopen(ARQ_PM, "w");
    /* habitantes */
    fprintf(f, "p 111.111.111-11 Joao Silva M 01/01/1990\n");
    fprintf(f, "p 222.222.222-22 Maria Souza F 15/06/1985\n");
    fprintf(f, "p 333.333.333-33 Carlos Lima M 20/03/2000\n");
    fprintf(f, "p 444.444.444-44 Ana Costa F 10/10/1995\n");
    /* moradores */
    fprintf(f, "m 111.111.111-11 cep1 N 10 ap1\n");
    fprintf(f, "m 222.222.222-22 cep1 S 20 ap2\n");
    fprintf(f, "m 333.333.333-33 cep2 L 15 ap3\n");
    fclose(f);
}

static void carrega_mapas(void) {
    mapa_quadras = processa_geo(ARQ_GEO);
    mapa_pessoas = pm_processa_arquivo(ARQ_PM, mapa_quadras);
}

static void abre_saidas(void) {
    txt = fopen(ARQ_TXT, "w");
    svg = fopen(ARQ_SVG, "w");
    fprintf(svg, "<svg xmlns=\"http://www.w3.org/2000/svg\">\n</svg>\n");
    fclose(svg);
    svg = fopen(ARQ_SVG, "r+");
}

static void escreve_qry(const char *conteudo) {
    FILE *f = fopen(ARQ_QRY, "w");
    fprintf(f, "%s", conteudo);
    fclose(f);
}


void setUp(void) {
    remove(HF_QUAD);
    remove(HF_HAB);

    escreve_geo_basico();
    escreve_pm_basico();
    carrega_mapas();
    abre_saidas();
}
void tearDown(void) {
    if (txt) { fclose(txt); txt = NULL; }
    if (svg) { fclose(svg); svg = NULL; }
    if (mapa_quadras) { exhash_destroy(mapa_quadras); mapa_quadras = NULL; }
    if (mapa_pessoas) { exhash_destroy(mapa_pessoas); mapa_pessoas = NULL; }
    remove(ARQ_GEO);  remove(ARQ_PM);   remove(ARQ_QRY);
    remove(ARQ_TXT);  remove(ARQ_SVG);
    remove(HF_QUAD);  remove(HF_HAB);
}

void test_h_encontra_habitante_existente(void) {
    escreve_qry("h? 111.111.111-11\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "111.111.111-11"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Joao"));
}

void test_h_nao_encontra_cpf_inexistente(void) {
    escreve_qry("h? 999.999.999-99\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "999.999.999-99"));
}

void test_h_morador_exibe_endereco(void) {
    escreve_qry("h? 111.111.111-11\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "cep1"));
}

void test_h_sem_teto_nao_exibe_endereco(void) {
    escreve_qry("h? 444.444.444-44\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "sem-teto"));
}

void test_nasc_insere_novo_habitante(void) {
    escreve_qry("nasc 555.555.555-55 Pedro Alves M 05/05/2005\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    bool found = exhash_search(mapa_pessoas, "555.555.555-55", hab);
    TEST_ASSERT_TRUE_MESSAGE(found, "Recem-nascido deve estar no banco");
    TEST_ASSERT_EQUAL_STRING("Pedro", habitante_get_nome(hab));
    free(hab);
}

void test_nasc_cpf_duplicado_nao_insere(void) {
    // 111 já existe
    escreve_qry("nasc 111.111.111-11 Outro Nome M 01/01/2000\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    exhash_search(mapa_pessoas, "111.111.111-11", hab);
    TEST_ASSERT_EQUAL_STRING("Joao", habitante_get_nome(hab));
    free(hab);
}

void test_nasc_novo_habitante_e_sem_teto(void) {
    escreve_qry("nasc 666.666.666-66 Laura Vaz F 10/10/2010\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    exhash_search(mapa_pessoas, "666.666.666-66", hab);
    TEST_ASSERT_TRUE_MESSAGE(habitante_is_sem_teto(hab), "Recem-nascido deve ser sem-teto");
    free(hab);
}

void test_rip_remove_habitante_do_banco(void) {
    escreve_qry("rip 111.111.111-11\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    bool found = exhash_search(mapa_pessoas, "111.111.111-11", hab);
    TEST_ASSERT_FALSE_MESSAGE(found, "Falecido nao deve estar no banco");
    free(hab);
}

void test_rip_escreve_dados_no_txt(void) {
    escreve_qry("rip 222.222.222-22\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "222.222.222-22"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Maria"));
}

void test_rip_cpf_inexistente_nao_crasha(void) {
    escreve_qry("rip 999.999.999-99\n");
    // só não pode crashar
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}

void test_rip_morador_escreve_endereco_no_txt(void) {
    escreve_qry("rip 111.111.111-11\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "cep1"));
}


void test_rq_remove_quadra_do_banco(void) {
    escreve_qry("rq cep3\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    quadra_t *q = malloc(quadra_get_size());
    bool found = exhash_search(mapa_quadras, "cep3", q);
    TEST_ASSERT_FALSE_MESSAGE(found, "Quadra removida nao deve estar no banco");
    free(q);
}

void test_rq_moradores_viram_sem_teto(void) {
    escreve_qry("rq cep1\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *h1 = malloc(habitante_get_size());
    habitante_t *h2 = malloc(habitante_get_size());

    exhash_search(mapa_pessoas, "111.111.111-11", h1);
    exhash_search(mapa_pessoas, "222.222.222-22", h2);

    TEST_ASSERT_TRUE_MESSAGE(habitante_is_sem_teto(h1), "111 deve ser sem-teto apos rq");
    TEST_ASSERT_TRUE_MESSAGE(habitante_is_sem_teto(h2), "222 deve ser sem-teto apos rq");

    free(h1); free(h2);
}

void test_rq_escreve_moradores_no_txt(void) {
    escreve_qry("rq cep1\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[2048] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "111.111.111-11"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "222.222.222-22"));
}

void test_rq_nao_morador_nao_vira_sem_teto(void) {
    escreve_qry("rq cep1\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *h = malloc(habitante_get_size());
    exhash_search(mapa_pessoas, "333.333.333-33", h);
    TEST_ASSERT_FALSE_MESSAGE(habitante_is_sem_teto(h), "333 nao mora em cep1, nao vira sem-teto");
    free(h);
}

void test_rq_cep_inexistente_nao_crasha(void) {
    escreve_qry("rq cep_fake\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}

void test_mud_atualiza_endereco_do_habitante(void) {
    escreve_qry("mud 111.111.111-11 cep2 S 30 ap9\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    exhash_search(mapa_pessoas, "111.111.111-11", hab);
    TEST_ASSERT_EQUAL_STRING("cep2", habitante_get_cep(hab));
    TEST_ASSERT_EQUAL_CHAR('S', habitante_get_face(hab));
    TEST_ASSERT_EQUAL_INT(30, habitante_get_numero_casa(hab));
    TEST_ASSERT_FALSE(habitante_is_sem_teto(hab));
    free(hab);
}

void test_mud_incrementa_contador_face_da_quadra(void) {
    escreve_qry("mud 444.444.444-44 cep2 L 10 ap_x\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    quadra_t *q = malloc(quadra_get_size());
    exhash_search(mapa_quadras, "cep2", q);
    TEST_ASSERT_EQUAL_INT(2, quadra_get_L(q));
    free(q);
}

void test_mud_cpf_inexistente_nao_crasha(void) {
    escreve_qry("mud 999.999.999-99 cep1 N 5 ap0\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}

void test_mud_cep_inexistente_nao_crasha(void) {
    escreve_qry("mud 111.111.111-11 cep_fake N 5 ap0\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}


void test_dspj_habitante_vira_sem_teto(void) {
    escreve_qry("dspj 111.111.111-11\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    exhash_search(mapa_pessoas, "111.111.111-11", hab);
    TEST_ASSERT_TRUE_MESSAGE(habitante_is_sem_teto(hab), "Despejado deve ser sem-teto");
    free(hab);
}

void test_dspj_habitante_permanece_no_banco(void) {
    escreve_qry("dspj 111.111.111-11\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    bool found = exhash_search(mapa_pessoas, "111.111.111-11", hab);
    TEST_ASSERT_TRUE_MESSAGE(found, "Despejado deve continuar no banco");
    free(hab);
}

void test_dspj_escreve_dados_no_txt(void) {
    escreve_qry("dspj 222.222.222-22\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "222.222.222-22"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Maria"));
}

void test_dspj_cpf_inexistente_nao_crasha(void) {
    escreve_qry("dspj 999.999.999-99\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}

void test_censo_escreve_total_habitantes(void) {
    escreve_qry("censo\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[2048] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "4"));
}

void test_censo_distingue_moradores_e_sem_teto(void) {
    escreve_qry("censo\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[2048] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "3"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "1"));
}

void test_censo_nao_crasha_com_banco_vazio(void) {
    exhash_destroy(mapa_pessoas);
    exhash_destroy(mapa_quadras);
    mapa_pessoas  = exhash_init(HF_HAB,  habitante_get_size(), 4096);
    mapa_quadras  = exhash_init(HF_QUAD, quadra_get_size(),    4096);

    escreve_qry("censo\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}

void test_pq_cep_inexistente_nao_crasha(void) {
    escreve_qry("pq cep_fake\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    TEST_ASSERT_TRUE(true);
}

void test_pq_escreve_cep_no_txt(void) {
    escreve_qry("pq cep1\n");
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[1024] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "cep1"));
}

void test_sequencia_nasc_mud_rip(void) {
    escreve_qry(
        "nasc 777.777.777-77 Lucas Melo M 01/01/2001\n"
        "mud 777.777.777-77 cep3 N 5 ap7\n"
        "rip 777.777.777-77\n"
    );
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    bool found = exhash_search(mapa_pessoas, "777.777.777-77", hab);
    TEST_ASSERT_FALSE_MESSAGE(found, "Falecido nao deve estar no banco");
    free(hab);
}

void test_sequencia_rq_vira_sem_teto_depois_h(void) {
    escreve_qry(
        "rq cep1\n"
        "h? 111.111.111-11\n"
    );
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);
    fclose(txt); txt = NULL;

    FILE *f = fopen(ARQ_TXT, "r");
    char buf[2048] = "";
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);

    TEST_ASSERT_NOT_NULL(strstr(buf, "sem-teto"));
}

void test_sequencia_dspj_depois_mud(void) {
    escreve_qry(
        "dspj 111.111.111-11\n"
        "mud 111.111.111-11 cep2 O 25 ap_novo\n"
    );
    processa_qry(ARQ_QRY, mapa_pessoas, mapa_quadras, txt, svg);

    habitante_t *hab = malloc(habitante_get_size());
    exhash_search(mapa_pessoas, "111.111.111-11", hab);
    TEST_ASSERT_FALSE_MESSAGE(habitante_is_sem_teto(hab), "Deve ter novo endereco apos mud");
    TEST_ASSERT_EQUAL_STRING("cep2", habitante_get_cep(hab));
    free(hab);
}

int main(void) {
    UNITY_BEGIN();

    // h?
    RUN_TEST(test_h_encontra_habitante_existente);
    RUN_TEST(test_h_nao_encontra_cpf_inexistente);
    RUN_TEST(test_h_morador_exibe_endereco);
    RUN_TEST(test_h_sem_teto_nao_exibe_endereco);

    // nasc
    RUN_TEST(test_nasc_insere_novo_habitante);
    RUN_TEST(test_nasc_cpf_duplicado_nao_insere);
    RUN_TEST(test_nasc_novo_habitante_e_sem_teto);

    // rip
    RUN_TEST(test_rip_remove_habitante_do_banco);
    RUN_TEST(test_rip_escreve_dados_no_txt);
    RUN_TEST(test_rip_cpf_inexistente_nao_crasha);
    RUN_TEST(test_rip_morador_escreve_endereco_no_txt);

    // rq
    RUN_TEST(test_rq_remove_quadra_do_banco);
    RUN_TEST(test_rq_moradores_viram_sem_teto);
    RUN_TEST(test_rq_escreve_moradores_no_txt);
    RUN_TEST(test_rq_nao_morador_nao_vira_sem_teto);
    RUN_TEST(test_rq_cep_inexistente_nao_crasha);

    // mud
    RUN_TEST(test_mud_atualiza_endereco_do_habitante);
    RUN_TEST(test_mud_incrementa_contador_face_da_quadra);
    RUN_TEST(test_mud_cpf_inexistente_nao_crasha);
    RUN_TEST(test_mud_cep_inexistente_nao_crasha);

    // dspj
    RUN_TEST(test_dspj_habitante_vira_sem_teto);
    RUN_TEST(test_dspj_habitante_permanece_no_banco);
    RUN_TEST(test_dspj_escreve_dados_no_txt);
    RUN_TEST(test_dspj_cpf_inexistente_nao_crasha);

    // censo
    RUN_TEST(test_censo_escreve_total_habitantes);
    RUN_TEST(test_censo_distingue_moradores_e_sem_teto);
    RUN_TEST(test_censo_nao_crasha_com_banco_vazio);

    // pq
    RUN_TEST(test_pq_cep_inexistente_nao_crasha);
    RUN_TEST(test_pq_escreve_cep_no_txt);

    // sequências
    RUN_TEST(test_sequencia_nasc_mud_rip);
    RUN_TEST(test_sequencia_rq_vira_sem_teto_depois_h);
    RUN_TEST(test_sequencia_dspj_depois_mud);

    return UNITY_END();
}
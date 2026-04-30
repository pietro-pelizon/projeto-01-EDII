#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "svg_handler.h"
#include "exhash.h"
#include "quadra.h"

static const char *ARQ_SVG = "t_svg_test.svg";

void setUp(void) {
    remove(ARQ_SVG);
}

void tearDown(void) {
    remove(ARQ_SVG);
}

// ============================================================
//  Helpers
// ============================================================

static char *le_arquivo(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);
    char *buf = malloc(tam + 1);
    fread(buf, 1, tam, f);
    buf[tam] = '\0';
    fclose(f);
    return buf;
}

// ============================================================
//  svg_init
// ============================================================

void test_svg_init_cria_arquivo(void) {
    FILE *svg = svg_init(ARQ_SVG);
    TEST_ASSERT_NOT_NULL_MESSAGE(svg, "svg_init deve retornar ponteiro valido");
    fecha_svg(svg);

    FILE *f = fopen(ARQ_SVG, "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Arquivo SVG deve existir no disco");
    fclose(f);
}

void test_svg_init_contem_cabecalho_xml(void) {
    FILE *svg = svg_init(ARQ_SVG);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<?xml"));
    free(buf);
}

void test_svg_init_contem_tag_svg(void) {
    FILE *svg = svg_init(ARQ_SVG);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<svg"));
    free(buf);
}

void test_svg_init_contem_tag_g(void) {
    FILE *svg = svg_init(ARQ_SVG);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<g>"));
    free(buf);
}

void test_svg_init_caminho_invalido_retorna_null(void) {
    FILE *svg = svg_init("/caminho/que/nao/existe/arquivo.svg");
    TEST_ASSERT_NULL_MESSAGE(svg, "Caminho invalido deve retornar NULL");
}

// ============================================================
//  fecha_svg
// ============================================================

void test_fecha_svg_escreve_fechamento(void) {
    FILE *svg = svg_init(ARQ_SVG);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "</g>"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "</svg>"));
    free(buf);
}

void test_fecha_svg_null_nao_crasha(void) {
    fecha_svg(NULL);
    TEST_ASSERT_TRUE(true);
}

void test_fecha_svg_arquivo_valido_apos_fechamento(void) {
    FILE *svg = svg_init(ARQ_SVG);
    fecha_svg(svg);

    // o arquivo deve ser legível e conter conteúdo válido
    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_GREATER_THAN(0, (int)strlen(buf));
    free(buf);
}

// ============================================================
//  svg_quadra_insert
// ============================================================

void test_svg_quadra_insert_escreve_rect(void) {
    FILE *svg = svg_init(ARQ_SVG);

    quadra_t *q = quadra_init("cep1", 10.0, 20.0, 100.0, 50.0);
    quadra_set_cq(q, 2.0, "orange", "black");
    svg_quadra_insert(svg, q);
    quadra_destroy(q);

    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "cep1"));
    free(buf);
}

void test_svg_quadra_insert_contem_coordenadas(void) {
    FILE *svg = svg_init(ARQ_SVG);

    quadra_t *q = quadra_init("cep2", 55.0, 77.0, 80.0, 40.0);
    quadra_set_cq(q, 1.0, "white", "blue");
    svg_quadra_insert(svg, q);
    quadra_destroy(q);

    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "55."));
    TEST_ASSERT_NOT_NULL(strstr(buf, "77."));
    TEST_ASSERT_NOT_NULL(strstr(buf, "80."));
    TEST_ASSERT_NOT_NULL(strstr(buf, "40."));
    free(buf);
}

void test_svg_quadra_insert_contem_cores(void) {
    FILE *svg = svg_init(ARQ_SVG);

    quadra_t *q = quadra_init("cepX", 0, 0, 10, 10);
    quadra_set_cq(q, 1.5, "red", "green");
    svg_quadra_insert(svg, q);
    quadra_destroy(q);

    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "red"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "green"));
    free(buf);
}

// ============================================================
//  svg_x_vermelho
// ============================================================

void test_svg_x_vermelho_escreve_duas_linhas(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_x_vermelho(svg, 100.0, 200.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    // deve haver duas tags <line>
    char *p1 = strstr(buf, "<line");
    TEST_ASSERT_NOT_NULL_MESSAGE(p1, "Deve ter pelo menos uma linha");
    char *p2 = strstr(p1 + 1, "<line");
    TEST_ASSERT_NOT_NULL_MESSAGE(p2, "Deve ter duas linhas para o X");
    free(buf);
}

void test_svg_x_vermelho_cor_vermelha(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_x_vermelho(svg, 50.0, 50.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "red"));
    free(buf);
}

// ============================================================
//  svg_cruz_insert
// ============================================================

void test_svg_cruz_insert_escreve_duas_linhas(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_cruz_insert(svg, 150.0, 250.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    char *p1 = strstr(buf, "<line");
    TEST_ASSERT_NOT_NULL(p1);
    char *p2 = strstr(p1 + 1, "<line");
    TEST_ASSERT_NOT_NULL(p2);
    free(buf);
}

void test_svg_cruz_insert_cor_vermelha(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_cruz_insert(svg, 0.0, 0.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "red"));
    free(buf);
}

// ============================================================
//  svg_circulo_despejo
// ============================================================

void test_svg_circulo_despejo_escreve_circle(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_circulo_despejo(svg, 300.0, 400.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<circle"));
    free(buf);
}

void test_svg_circulo_despejo_cor_preta(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_circulo_despejo(svg, 0.0, 0.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "black"));
    free(buf);
}

void test_svg_circulo_despejo_contem_coordenadas(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_circulo_despejo(svg, 123.0, 456.0);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "123."));
    TEST_ASSERT_NOT_NULL(strstr(buf, "456."));
    free(buf);
}

// ============================================================
//  svg_marcador_mudanca
// ============================================================

void test_svg_marcador_mudanca_escreve_rect_e_text(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_marcador_mudanca(svg, 200.0, 300.0, "123.456.789-00");
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "<text"));
    free(buf);
}

void test_svg_marcador_mudanca_contem_cpf(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_marcador_mudanca(svg, 0.0, 0.0, "999.888.777-66");
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "999.888.777-66"));
    free(buf);
}

void test_svg_marcador_mudanca_cor_vermelha(void) {
    FILE *svg = svg_init(ARQ_SVG);
    svg_marcador_mudanca(svg, 0.0, 0.0, "000.000.000-00");
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "red"));
    free(buf);
}

// ============================================================
//  svg_escrever_populacao_pq
// ============================================================

void test_svg_populacao_pq_escreve_textos(void) {
    FILE *svg = svg_init(ARQ_SVG);

    quadra_t *q = quadra_init("cep1", 10.0, 20.0, 100.0, 50.0);
    int n = 3, s = 2, l = 1, o = 4;

    quadra_set_hab_faces(q, &n, &s, &l, &o);
    svg_escrever_populacao_pq(svg, q);
    quadra_destroy(q);

    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<text"));
    free(buf);
}

void test_svg_populacao_pq_contem_total(void) {
    FILE *svg = svg_init(ARQ_SVG);

    int n = 1, s = 1, l = 1, o = 1;
    quadra_t *q = quadra_init("cep1", 0.0, 0.0, 100.0, 50.0);
    quadra_set_hab_faces(q, &n, &s, &l, &o); // total = 4
    svg_escrever_populacao_pq(svg, q);
    quadra_destroy(q);

    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "4"));
    free(buf);
}

// ============================================================
//  svg_desenha_mapa_base
// ============================================================

void test_svg_desenha_mapa_base_com_quadras(void) {
    exhash_t *mapa = exhash_init("t_svg_quadras.hf", quadra_get_size(), 4096);

    quadra_t *q1 = quadra_init("c1", 10.0, 10.0, 50.0, 30.0);
    quadra_set_cq(q1, 1.0, "orange", "black");
    exhash_insert(mapa, q1, "c1");
    quadra_destroy(q1);

    quadra_t *q2 = quadra_init("c2", 100.0, 100.0, 60.0, 40.0);
    quadra_set_cq(q2, 2.0, "white", "blue");
    exhash_insert(mapa, q2, "c2");
    quadra_destroy(q2);

    FILE *svg = svg_init(ARQ_SVG);
    svg_desenha_mapa_base(svg, mapa);
    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "c1"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "c2"));
    free(buf);

    exhash_destroy(mapa);
    remove("t_svg_quadras.hf");
}

void test_svg_desenha_mapa_base_vazio_nao_crasha(void) {
    exhash_t *mapa = exhash_init("t_svg_quadras_vazio.hf", quadra_get_size(), 4096);

    FILE *svg = svg_init(ARQ_SVG);
    svg_desenha_mapa_base(svg, mapa);
    fecha_svg(svg);

    // só não pode crashar e o SVG deve ser válido
    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_NOT_NULL(strstr(buf, "</svg>"));
    free(buf);

    exhash_destroy(mapa);
    remove("t_svg_quadras_vazio.hf");
}

// ============================================================
//  Múltiplos elementos no mesmo SVG
// ============================================================

void test_multiplos_elementos_no_mesmo_svg(void) {
    FILE *svg = svg_init(ARQ_SVG);

    quadra_t *q = quadra_init("cepA", 10.0, 10.0, 80.0, 40.0);
    quadra_set_cq(q, 1.0, "red", "black");
    svg_quadra_insert(svg, q);
    quadra_destroy(q);

    svg_x_vermelho(svg, 50.0, 30.0);
    svg_cruz_insert(svg, 200.0, 200.0);
    svg_circulo_despejo(svg, 300.0, 300.0);
    svg_marcador_mudanca(svg, 400.0, 400.0, "111.111.111-11");

    fecha_svg(svg);

    char *buf = le_arquivo(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "<line"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "<circle"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "<text"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "</svg>"));
    free(buf);
}

// ============================================================
//  main
// ============================================================

int main(void) {
    UNITY_BEGIN();

    // svg_init
    RUN_TEST(test_svg_init_cria_arquivo);
    RUN_TEST(test_svg_init_contem_cabecalho_xml);
    RUN_TEST(test_svg_init_contem_tag_svg);
    RUN_TEST(test_svg_init_contem_tag_g);
    RUN_TEST(test_svg_init_caminho_invalido_retorna_null);

    // fecha_svg
    RUN_TEST(test_fecha_svg_escreve_fechamento);
    RUN_TEST(test_fecha_svg_null_nao_crasha);
    RUN_TEST(test_fecha_svg_arquivo_valido_apos_fechamento);

    // svg_quadra_insert
    RUN_TEST(test_svg_quadra_insert_escreve_rect);
    RUN_TEST(test_svg_quadra_insert_contem_coordenadas);
    RUN_TEST(test_svg_quadra_insert_contem_cores);

    // svg_x_vermelho
    RUN_TEST(test_svg_x_vermelho_escreve_duas_linhas);
    RUN_TEST(test_svg_x_vermelho_cor_vermelha);

    // svg_cruz_insert
    RUN_TEST(test_svg_cruz_insert_escreve_duas_linhas);
    RUN_TEST(test_svg_cruz_insert_cor_vermelha);

    // svg_circulo_despejo
    RUN_TEST(test_svg_circulo_despejo_escreve_circle);
    RUN_TEST(test_svg_circulo_despejo_cor_preta);
    RUN_TEST(test_svg_circulo_despejo_contem_coordenadas);

    // svg_marcador_mudanca
    RUN_TEST(test_svg_marcador_mudanca_escreve_rect_e_text);
    RUN_TEST(test_svg_marcador_mudanca_contem_cpf);
    RUN_TEST(test_svg_marcador_mudanca_cor_vermelha);

    // svg_escrever_populacao_pq
    RUN_TEST(test_svg_populacao_pq_escreve_textos);
    RUN_TEST(test_svg_populacao_pq_contem_total);

    // svg_desenha_mapa_base
    RUN_TEST(test_svg_desenha_mapa_base_com_quadras);
    RUN_TEST(test_svg_desenha_mapa_base_vazio_nao_crasha);

    // integração
    RUN_TEST(test_multiplos_elementos_no_mesmo_svg);

    return UNITY_END();
}
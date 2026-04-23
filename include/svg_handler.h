#ifndef PROJETO_01_EDII_SVG_HANDLER_H
#define PROJETO_01_EDII_SVG_HANDLER_H


#include <stdio.h>

#include "exhash.h"
#include "quadra.h"

/*
 * ------- MÓDULO DE SAÍDA SVG -------
 * Este módulo funciona como uma biblioteca de funções utilitárias
 * responsável por gerar e escrever o arquivo de saída no formato SVG.
 * Ele provê funcionalidades para:
 *
 * Gerenciamento de Arquivo: Controlar o ciclo de vida do arquivo SVG,
 * incluindo sua criação ('abreSvgEscrita'), a escrita do cabeçalho
 * padrão ('printaCabecalhoSvg') e o seu fechamento ('fechaSVG');
 *
 * Desenho de Formas: Converter os dados de TADs de formas específicas
 * (círculo, retângulo, etc.) em suas respectivas tags de texto no
 * padrão SVG, desenhando-as no arquivo;
 *
 * Abstração da Sintaxe SVG: Esconder a complexidade da sintaxe SVG,
 * permitindo que o resto do programa desenhe formas simplesmente
 * chamando uma função, sem precisar conhecer os detalhes das tags
 * '<rect>', '<circle>', etc.
 */


/// @brief: Cria, abre e inicializa um arquivo SVG com o cabeçalho padrão.
/// @param caminho_arquivo: O nome/caminho do arquivo a ser criado.
/// @return: Retorna um ponteiro para o arquivo (FILE*) pronto para escrita, ou NULL se ocorrer um erro.
FILE* svg_init(const char* caminho_arquivo);

/// @brief: Desenha um retângulo no arquivo SVG.
/// @param svg: Ponteiro para o arquivo SVG aberto para escrita.
/// @param q: Ponteiro para o tipo 'quadra_t' com os dados a serem desenhados.
void svg_quadra_insert(FILE *svg, const quadra_t *q);

/// @brief
/// @param svg
/// @param x
/// @param y
void svg_x_vermelho(FILE *svg, double x, double y);

void svg_escrever_populacao_pq(FILE *svg, quadra_t *quadra);

void fecha_svg(FILE *svg);

void svg_cruz_insert(FILE *svg, double cx, double cy);

void svg_marcador_mudanca(FILE *svg, double cx, double cy, const char *cpf);

void svg_circulo_despejo(FILE *svg, double cx, double cy);

void svg_desenha_mapa_base(FILE *svg, exhash_t *mapa_quadras);

#endif //PROJETO_01_EDII_SVG_HANDLER_H

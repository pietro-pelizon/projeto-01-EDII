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


/// @brief Cria, abre e inicializa um arquivo SVG com o cabeçalho padrão.
/// @param caminho_arquivo O nome/caminho do arquivo a ser criado.
/// @return Retorna um ponteiro para o arquivo (FILE*) pronto para escrita, ou NULL se ocorrer um erro.
FILE* svg_init(const char* caminho_arquivo);

/// @brief Desenha um retângulo representando uma quadra no arquivo SVG.
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param q Ponteiro para a quadra com as dimensões e coordenadas a serem desenhadas.
void svg_quadra_insert(FILE *svg, const quadra_t *q);

/// @brief Desenha um 'X' vermelho que cobra a quadra toda para simbolizar sua remoção (comando rq).
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param x Coordenada X (eixo horizontal) do centro da quadra removida.
/// @param y Coordenada Y (eixo vertical) do centro da quadra removida.
/// @param w Largura da quadra removida.
/// @param h Altura da quadra removida.
void svg_x_vermelho(FILE *svg, double x, double y, double w, double h);

/// @brief Escreve os dados populacionais (censo) de cada face no interior do desenho da quadra.
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param quadra Ponteiro para a quadra contendo os dados populacionais a serem exibidos.
void svg_escrever_populacao_pq(FILE *svg, quadra_t *quadra);

/// @brief Insere a tag de fechamento `</svg>` e encerra o fluxo do arquivo com segurança.
/// @param svg Ponteiro para o arquivo SVG a ser fechado.
void fecha_svg(FILE *svg);

/// @brief Desenha uma cruz (lápide) para indicar o óbito de um habitante (comando rip).
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param cx Coordenada X (eixo horizontal) correspondente à casa do falecido.
/// @param cy Coordenada Y (eixo vertical) correspondente à casa do falecido.
void svg_cruz_insert(FILE *svg, double cx, double cy);

/// @brief Insere um marcador visual com o CPF para registrar o novo endereço após uma mudança (comando mud).
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param cx Coordenada X (eixo horizontal) do novo endereço.
/// @param cy Coordenada Y (eixo vertical) do novo endereço.
/// @param cpf String contendo o CPF do habitante que se mudou.
void svg_marcador_mudanca(FILE *svg, double cx, double cy, const char *cpf);

/// @brief Desenha um círculo delimitador indicando o despejo de um habitante (comando dspj).
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param cx Coordenada X (eixo horizontal) do endereço do qual o habitante foi despejado.
/// @param cy Coordenada Y (eixo vertical) do endereço do qual o habitante foi despejado.
void svg_circulo_despejo(FILE *svg, double cx, double cy);

/// @brief Percorre o hashfile de quadras e desenha a estrutura base de toda a cidade no SVG.
/// @param svg Ponteiro para o arquivo SVG aberto para escrita.
/// @param mapa_quadras Ponteiro para o hashfile contendo todas as quadras carregadas na memória.
void svg_desenha_mapa_base(FILE *svg, exhash_t *mapa_quadras);

#endif //PROJETO_01_EDII_SVG_HANDLER_H

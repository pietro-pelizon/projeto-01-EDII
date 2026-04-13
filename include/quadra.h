#ifndef PROJETO_01_EDII_QUADRA_H
#define PROJETO_01_EDII_QUADRA_H

/* O módulo a seguir define a entidade Quadra (retângulo)
 * para o Sistema de Informações Geográficas (SIG) de Bitnópolis.
 *
 * A estrutura encapsula os dados espaciais fundamentais da cidade,
 * incluindo seu identificador único (CEP), suas coordenadas de ancoragem (x, y),
 * suas dimensões (largura e altura) e suas propriedades visuais para
 * renderização em SVG (cores de preenchimento, borda e espessura).
 * * O módulo foi projetado utilizando a técnica de Ponteiros Opacos (Opaque Pointers)
 * com alocação dinâmica (Heap). Isso garante o encapsulamento total dos dados
 * e compatibilidade para serialização pelo Hash Extensível.
 *
 * Funcionalidades principais:
 * - Criação segura: aloca quadras na memória dinâmica (Heap).
 * - Encapsulamento total: uso rigoroso de Getters e Setters para proteger
 * as propriedades visuais e espaciais contra alterações indevidas.
 * - Atualização em lote: suporte nativo para o comando 'cq', permitindo
 * alterar múltiplas propriedades visuais (cores e espessura) com uma única chamada.
 */

typedef struct stQuadra quadra_t;

/// @brief Inicializa uma nova quadra com CEP, coordenadas, altura e largura.
/// @param cep CEP identificador da quadra (Chave principal).
/// @param x Coordenada X do ponto de ancoragem da quadra.
/// @param y Coordenada Y do ponto de ancoragem da quadra.
/// @param w Largura (width) da quadra.
/// @param h Altura (height) da quadra.
/// @return Retorna um ponteiro para a quadra alocada dinamicamente (Heap). O usuário DEVE chamar quadra_destroy() após o uso.
quadra_t *quadra_init(const char *cep, double x, double y, double w, double h);

/// @brief Destrói a quadra, liberando a memória alocada (Evita Memory Leaks).
/// @param q Ponteiro para a quadra a ser destruída.
void quadra_destroy(quadra_t *q);

/// @brief Define a coordenada X do ponto de ancoragem da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param novo_x Novo valor da coordenada X no eixo horizontal.
void quadra_set_x(quadra_t *q, double novo_x);

/// @brief Define a coordenada Y do ponto de ancoragem da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param novo_y Novo valor da coordenada Y no eixo vertical.
void quadra_set_y(quadra_t *q, double novo_y);

/// @brief Define a largura (width) do retângulo da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param novo_w Novo valor numérico da largura.
void quadra_set_w(quadra_t *q, double novo_w);

/// @brief Define a altura (height) do retângulo da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param novo_h Novo valor numérico da altura.
void quadra_set_h(quadra_t *q, double novo_h);

/// @brief Define a cor de preenchimento (cfill) do retângulo da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param nova_corp String contendo a cor (ex: "red", "#FF0000").
void quadra_set_corp(quadra_t *q, const char *nova_corp);

/// @brief Define a cor da borda (cstrk) do retângulo da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param nova_corb String contendo a cor da borda.
void quadra_set_corb(quadra_t *q, const char *nova_corb);

/// @brief Define a espessura da borda (stroke width) do retângulo da quadra.
/// @param q Ponteiro para a quadra que será alterada.
/// @param novo_sw Novo valor numérico da espessura.
void quadra_set_sw(quadra_t *q, double novo_sw);

/// @brief Obtém a cor de preenchimento atual da quadra.
/// @param q Ponteiro para a quadra.
/// @return Retorna a string inalterável contendo a cor de preenchimento.
const char *quadra_get_corp(const quadra_t *q);

/// @brief Obtém a cor da borda atual da quadra.
/// @param q Ponteiro para a quadra.
/// @return Retorna a string inalterável contendo a cor da borda.
const char *quadra_get_corb(const quadra_t *q);

/// @brief Obtém a espessura da borda (stroke width) atual da quadra.
/// @param q Ponteiro para a quadra.
/// @return Retorna o valor numérico da espessura.
double quadra_get_sw(const quadra_t *q);

/// @brief Aplica em lote as propriedades visuais da quadra (Equivalente ao comando 'cq').
/// @param q Ponteiro para a quadra que será alterada.
/// @param sw Espessura da borda (stroke width).
/// @param corb Cor da borda (cstrk).
/// @param corp Cor de preenchimento (cfill).
void quadra_set_cq(quadra_t *q, double sw, const char *corb, const char *corp);

#endif //PROJETO_01_EDII_QUADRA_H
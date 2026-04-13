#ifndef PROJETO_01_EDII_HABITANTE_H
#define PROJETO_01_EDII_HABITANTE_H

#include <stdbool.h>

/* O módulo a seguir define a entidade Habitante e seu Endereço (casa)
 * para o Sistema de Informações Geográficas (SIG) de Bitnópolis.
 *
 * A estrutura encapsula os dados pessoais de um cidadão (CPF, nome, sexo, etc.)
 * e sua localização espacial na cidade (CEP da quadra, face, número e complemento).
 * O módulo foi projetado utilizando a técnica de Ponteiros Opacos (Opaque Pointers)
 * com alocação dinâmica (Heap). Isso garante o encapsulamento total dos dados
 * e compatibilidade para serialização pelo Hash Extensível.
 *
 * Funcionalidades principais:
 * - Criação segura: aloca habitantes na memória Heap, padronizando
 * o status inicial como "sem-teto" até que um endereço seja fornecido.
 * - Encapsulamento total: uso rigoroso de Getters e Setters para proteger
 * os dados contra estouro de buffer (buffer overflow) e alterações indevidas.
 * A struct real fica escondida no arquivo .c.
 * - Desnormalização de moradia: abstrai a entidade "Casa", vinculando
 * os atributos espaciais diretamente ao morador para buscas em tempo O(1).
 */

typedef struct stHabitante habitante_t;
typedef struct stEndereco endereco_t;

/// @brief Cria e inicializa um novo habitante alocado dinamicamente (Heap). Por padrão, nasce como sem-teto.
/// @param cpf String com o CPF do habitante (Chave principal).
/// @param nome String com o primeiro nome.
/// @param sobrenome String com o sobrenome.
/// @param sexo Caractere representando o sexo ('M' ou 'F').
/// @param data_nascimento String com a data de nascimento (ex: "dd/mm/aaaa").
/// @return Retorna um ponteiro para a estrutura habitante_t. O usuário DEVE chamar habitante_destroy() após o uso.
habitante_t *habitante_init(const char *cpf, const char *nome,
    const char *sobrenome, char sexo, const char *data_nascimento);

/// @brief Libera a memória alocada para o habitante (Evita Memory Leaks).
/// @param h Ponteiro para o habitante a ser destruído.
void habitante_destroy(habitante_t *h);

/// @brief Atualiza todos os dados de moradia de uma vez. Automaticamente tira o status de sem-teto.
/// @param hab Ponteiro para o habitante que será atualizado.
/// @param cep CEP da quadra onde a casa está localizada.
/// @param face Face da quadra ('N', 'S', 'L' ou 'O').
/// @param numero Distância (número) da casa na face da quadra.
/// @param complemento Complemento do endereço (ex: "Apto 42").
void habitante_set_endereco(habitante_t *hab, char *cep, char face, double numero, char *complemento);

/// @brief Atualiza o CPF do habitante.
/// @param hab Ponteiro para o habitante.
/// @param cpf Novo CPF.
void habitante_set_cpf(habitante_t *hab, const char *cpf);

/// @brief Atualiza o nome do habitante.
/// @param hab Ponteiro para o habitante.
/// @param nome Novo nome.
void habitante_set_nome(habitante_t *hab, const char *nome);

/// @brief Atualiza o sobrenome do habitante.
/// @param hab Ponteiro para o habitante.
/// @param sobrenome Novo sobrenome.
void habitante_set_sobrenome(habitante_t *hab, const char *sobrenome);

/// @brief Atualiza o sexo do habitante.
/// @param hab Ponteiro para o habitante.
/// @param sexo Novo caractere de sexo.
void habitante_set_sexo(habitante_t *hab, char sexo);

/// @brief Atualiza a data de nascimento do habitante.
/// @param hab Ponteiro para o habitante.
/// @param data Nova data de nascimento.
void habitante_set_data_nascimento(habitante_t *hab, const char *data);

/// @brief Força a mudança do status de moradia (sem-teto).
/// @param hab Ponteiro para o habitante.
/// @param status True para sem-teto, false se possuir moradia.
void habitante_set_sem_teto(habitante_t *hab, bool status);

/// @brief Obtém o CPF do habitante.
/// @param hab Ponteiro constante para o habitante.
/// @return String inalterável (const char*) com o CPF.
const char *habitante_get_cpf(const habitante_t *hab);

/// @brief Obtém o nome do habitante.
/// @param hab Ponteiro constante para o habitante.
/// @return String inalterável (const char*) com o nome.
const char *habitante_get_nome(const habitante_t *hab);

/// @brief Obtém o sobrenome do habitante.
/// @param hab Ponteiro constante para o habitante.
/// @return String inalterável (const char*) com o sobrenome.
const char *habitante_get_sobrenome(const habitante_t *hab);

/// @brief Obtém o sexo do habitante.
/// @param hab Ponteiro constante para o habitante.
/// @return Caractere representando o sexo.
char habitante_get_sexo(const habitante_t *hab);

/// @brief Obtém a data de nascimento do habitante.
/// @param hab Ponteiro constante para o habitante.
/// @return String inalterável (const char*) com a data de nascimento.
const char *habitante_get_data_nascimento(const habitante_t *hab);

/// @brief Verifica se o habitante é um sem-teto.
/// @param hab Ponteiro constante para o habitante.
/// @return Retorna true se for sem-teto, ou false caso possua moradia.
bool habitante_is_sem_teto(const habitante_t *hab);

/// @brief Atualiza individualmente o CEP da moradia.
/// @param hab Ponteiro para o habitante.
/// @param cep Novo CEP.
void habitante_set_cep(habitante_t *hab, const char *cep);

/// @brief Atualiza individualmente a face da moradia.
/// @param hab Ponteiro para o habitante.
/// @param face Nova face ('N', 'S', 'L' ou 'O').
void habitante_set_face(habitante_t *hab, char face);

/// @brief Atualiza individualmente a distância/número da casa.
/// @param hab Ponteiro para o habitante.
/// @param numero Novo valor numérico da casa.
void habitante_set_numero_casa(habitante_t *hab, double numero);

/// @brief Atualiza individualmente o complemento da moradia.
/// @param hab Ponteiro para o habitante.
/// @param complemento Novo complemento.
void habitante_set_complemento(habitante_t *hab, const char *complemento);

/// @brief Obtém o CEP onde o habitante mora.
/// @param hab Ponteiro constante para o habitante.
/// @return String inalterável com o CEP, ou NULL se for sem-teto.
const char *habitante_get_cep(const habitante_t *hab);

/// @brief Obtém a face da quadra onde o habitante mora.
/// @param hab Ponteiro constante para o habitante.
/// @return Caractere da face, ou '\0' se for sem-teto.
char habitante_get_face(const habitante_t *hab);

/// @brief Obtém o número da casa onde o habitante mora.
/// @param hab Ponteiro constante para o habitante.
/// @return Valor numérico da casa, ou -1.0 se for sem-teto.
double habitante_get_numero_casa(const habitante_t *hab);

/// @brief Obtém o complemento do endereço do habitante.
/// @param hab Ponteiro constante para o habitante.
/// @return String inalterável com o complemento, ou NULL se for sem-teto.
const char *habitante_get_complemento(const habitante_t *hab);

#endif //PROJETO_01_EDII_HABITANTE_H
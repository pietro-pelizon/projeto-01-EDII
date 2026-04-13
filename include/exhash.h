#ifndef E_HASHING
#define E_HASHING

#include <inttypes.h>
#include <stdbool.h>

/* O módulo a seguir apresenta uma estrutura de dados de Hash Extensível (Extendible Hashing)
 * com persistência em disco. Feita para lidar com quantidades densas de dados,
 * possibilita inserção, busca e remoção com pouquíssimos acessos ao disco (O(1) na média).
 *
 * A estrutura gerencia um arquivo binário (.hf) como se fosse um banco de dados.
 * O usuário fornece uma chave alfanumérica (string) e o dado em si. O módulo aplica
 * uma função de Hash internamente para mapear essa chave a um "balde" (bucket) físico no disco.
 * * Funcionalidades principais:
 * - Persistência automática: os dados inseridos sobrevivem ao fechamento do programa.
 * - Expansão dinâmica: o diretório cresce automaticamente conforme a necessidade de espaço.
 * - Operações diretas: inserir, buscar a existência, resgatar dados para a RAM e remover.
 */

typedef struct stExhash exhash_t;
typedef struct stBucket bucket_t;
typedef struct stFile_header header_t;

/// @brief Inicializa o Hash Map em disco.
/// Se o arquivo já existir, ele carrega o estado anterior. Se não, cria um banco de dados novo.
/// @param filename Nome do arquivo físico (ex: "dados.hf") que será utilizado.
/// @param record_size Tamanho exato em bytes da struct que será guardada (ex: sizeof(Habitante)).
/// @param bucket_size Tamanho total de cada balde no disco (em bytes).
/// @return Retorna um ponteiro para o manipulador do Hash Map, ou NULL em caso de erro.
exhash_t *eHashing_init(const char *filename, uint32_t record_size, uint32_t bucket_size);

/// @brief Insere um dado no Hash Map.
/// @param map Ponteiro para o manipulador do Hash Map.
/// @param data O dado que será inserido (usamos "const void *" para aceitar qualquer struct).
/// @param key Chave alfanumérica (string) única vinculada ao dado (ex: CPF ou CEP).
/// @return Retorna true caso a inserção ocorra com sucesso, ou false caso a chave já exista.
bool eHashing_insert(exhash_t *map, const void *data, const char *key);

/// @brief Busca e copia um dado do disco para a memória RAM.
/// @param map Ponteiro para o manipulador do Hash Map.
/// @param key Chave alfanumérica do dado a ser buscado.
/// @param out_data Buffer pré-alocado pelo usuário (com tamanho mínimo de 'record_size').
/// Os dados encontrados serão copiados diretamente para este endereço.
/// @return Retorna true se o dado for encontrado e copiado, ou false caso não exista.
bool eHashing_search(const exhash_t *map, const char *key, void *out_data);

/// @brief Remove permanentemente um dado do disco e o retorna para a RAM.
/// @param map Ponteiro para o manipulador do Hash Map.
/// @param key Chave alfanumérica do dado a ser removido.
/// @return Retorna um ponteiro alocado com os dados removidos, ou NULL se não encontrar.
/// @attention O usuário é responsável por liberar (free) essa memória após o uso!
void *eHashing_remove(exhash_t *map, const char *key);

/// @brief Finaliza as operações, garante o salvamento seguro do diretório no disco e libera a RAM.
/// @attention Deve ser chamada obrigatoriamente antes do programa encerrar para evitar corrupção de dados.
/// @param map Ponteiro para o manipulador do Hash Map.
void eHashing_destroy(exhash_t *map);

#endif
#ifndef E_HASHING
#define E_HASHING

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct StEHashing eHashing;
typedef struct StBucket bucket;

/// @brief Inicializa o Hash Map
/// @param filename Nome do arquivo que será utilizado pelo hash extensível
/// @param bucket_size Tamanho total do bucket (em bytes)
/// @param destructor Ponteiro para a função que tratará a destruição do tipo de dado inserido no hash map
/// @return Retorna um ponteiro para o hash map já criado
eHashing *e_hashing_init(const char *filename, size_t bucket_size, void (*destructor)(void *));

/// @brief Insere um dado no Hash Map, pra isso, precisamos ter sua chave da função de hashing
/// e o seu tamanho (em bytes)
/// @param map Ponteiro para o Hash Map
/// @param data O dado que será inserido - usamos "Void *" para podermos inserir qualquer tipo de dado
/// @param key Chave criada pela função de hashing para o dado a ser inserido
/// @param data_size Tamanho do dado a ser inserido (em bytes)
/// @return Retorna true caso a inserção ocorra com sucesso e false caso o contrário
bool e_hashing_insert(eHashing *map, void *data, uint64_t key, size_t data_size);

/// @brief Busca um dado no Hash Map
/// @param map Ponteiro para o Hash Map
/// @param key Chave do dado a ser buscado
/// @param data_size PPonteiro de SAÍDA que receberá o tamanho do dado encontrado (em bytes)
/// @return Retorna um ponteiro para o dado, ou NULL não caso seja encontrado
void *e_hashing_search(eHashing *map, uint64_t key, size_t *data_size);

/// @brief Remove um dado do Hash Map e o retorna para o usuário
/// @param map Ponteiro para o Hash Map
/// @param key Chave do dado a ser removido
/// @param out_data_size Ponteiro de SAÍDA que receberá o tamanho do dado removido (em bytes)
/// @return Retorna o dado que foi removido. O usuário é responsável por liberar (free) essa memória
void *e_hashing_remove(eHashing *map, uint64_t key, size_t *out_data_size);

/// @brief Limpa toda a estrutura do Hash Map
/// @param map Ponteiro para o Hash Map
void e_hashing_destroy(eHashing *map);

#endif

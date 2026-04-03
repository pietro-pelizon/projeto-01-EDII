#ifndef E_HASHING
#define E_HASHING

#include <inttypes.h>
#include <stdbool.h>

typedef struct StEHashing eHashing;
typedef struct StBucket bucket;

/// @brief Inicializa o Hash Map
/// @param filename Nome do arquivo que será utilizado pelo hash extensível
/// @param bucket_size Tamanho total do bucket (em bytes)
/// @return Retorna um ponteiro para o hash map já criado
eHashing *e_hashing_init(const char *filename, uint32_t record_size, uint32_t bucket_size);

uint64_t e_hashing_capacity(eHashing *map);

/// @brief Insere um dado no Hash Map, para isso, precisamos ter sua chave da função de hashing
/// e o seu tamanho (em bytes)
/// @param map Ponteiro para o Hash Map
/// @param data O dado que será inserido - usamos "Void *" para podermos inserir qualquer tipo de dado
/// @param key Chave criada pela função de hashing para o dado a ser inserido
/// @return Retorna true caso a inserção ocorra com sucesso e false caso o contrário
bool e_hashing_insert(eHashing *map, void *data, uint64_t key);

/// @brief Busca um dado no Hash Map
/// @param map Ponteiro para o Hash Map
/// @param key Chave do dado a ser buscado
/// @param out_data Dado que será escrito diretamente no buffer do usuário, aí não teremos a necessidade
/// de se preocupar com a memória alocada
/// @return Retorna um ponteiro para o dado, ou NULL não caso seja encontrado
bool e_hashing_search(eHashing *map, uint64_t key, void *out_data);

/// @brief Remove um dado do Hash Map e o retorna para o usuário
/// @param map Ponteiro para o Hash Map
/// @param key Chave do dado a ser removido
/// @return Retorna o dado que foi removido. O usuário é responsável por liberar (free) essa memória
void *e_hashing_remove(eHashing *map, uint64_t key);

/// @brief Limpa toda a estrutura do Hash Map
/// @param map Ponteiro para o Hash Map
void e_hashing_destroy(eHashing *map);

#endif

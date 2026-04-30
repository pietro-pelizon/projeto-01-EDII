#ifndef PROJETO_01_EDII_GEO_HANDLER_H
#define PROJETO_01_EDII_GEO_HANDLER_H

/*
 * @file geo_handler.h
 * @brief Módulo responsável pelo processamento do arquivo geográfico (.geo).
 * Lê e interpreta os comandos de criação de quadras (formas geométricas),
 * alocando a memória necessária e inserindo-as no banco de dados (hashfile).
*/

#include "../include/exhash.h"

// @brief Lê as informações de um arquivo (.geo) e processa as quadras.
// @param caminho_geo Caminho para o arquivo (.geo) a ser lido, referenciado pela flag "-e" na linha de comando.
// @param mapa_quadras Ponteiro para a hashfile onde as quadras serão inseridas.
exhash_t *processa_geo(const char *caminho_geo);

#endif //PROJETO_01_EDII_GEO_HANDLER_H

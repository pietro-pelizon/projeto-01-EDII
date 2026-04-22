#ifndef PROJETO_01_EDII_GEO_HANDLER_H
#define PROJETO_01_EDII_GEO_HANDLER_H

#include "../include/exhash.h"

// @brief Lê as informações de um arquivo .geo e processa as quadras.
// @param caminho_geo Caminho para o arquivo .geo a ser lido.
// @param mapa_quadras Ponteiro para a tabela hash extensível onde as quadras serão inseridas.
void processa_geo(const char *caminho_geo, exhash_t *mapa_quadras);

#endif //PROJETO_01_EDII_GEO_HANDLER_H

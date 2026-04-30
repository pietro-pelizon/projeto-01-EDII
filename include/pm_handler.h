/// @file pm_handler.h
/// @brief Módulo responsável pelo processamento do arquivo de pessoas e moradores (.pm).
/// Interpreta os dados de entrada para instanciar habitantes, identificar suas condições 
/// (moradores ou sem-teto) e validá-los contra a infraestrutura existente da cidade.

#ifndef PROJETO_01_EDII_PM_HANDLER_H
#define PROJETO_01_EDII_PM_HANDLER_H

#include "../include/exhash.h"

/// @brief Processa o arquivo (.pm) e insere todos os habitantes no seu respectivo hashfile.
/// @param caminho_arquivo Caminho para o arquivo (.pm), referenciado pela flag "-pm" na linha de comando.
/// @param mapa_quadras Ponteiro para o hashfile das quadras (necessário para validar endereços).
/// @return Retorna o hashfile criado após processar o arquivo (.pm).
exhash_t *pm_processa_arquivo(const char *caminho_arquivo, exhash_t *mapa_quadras);

#endif //PROJETO_01_EDII_PM_HANDLER_H
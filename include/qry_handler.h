#ifndef PROJETO_01_EDII_QRY_HANDLER_H
#define PROJETO_01_EDII_QRY_HANDLER_H

/**
 * @file qry_handler.h
 * @brief Módulo processador de consultas e eventos interativos.
 *
 * @details Este módulo atua como o orquestrador das dinâmicas da cidade.
 * Ele é responsável por interpretar os comandos do arquivo (.qry) (como nascimentos,
 * falecimentos, mudanças de endereço, remoção de quadras e censos) e aplicar essas
 * alterações no banco de dados (tabelas hash de habitantes e quadras).
 * Além de atualizar o estado interno do sistema, o módulo gera os relatórios finais,
 * emitindo os dados processados para um arquivo de texto (.txt) e adicionando
 * marcações visuais (cruzes, círculos, etc.) ao mapa da cidade (.svg).
 */


#include "../include/qry_handler.h"
#include <stdio.h>
#include "exhash.h"

/// @brief Lê o arquivo (.qry), o traduz para o código do programa e executa as funções necessárias
/// @param caminho_qry Path para o arquivo (.qry) a ser lido
/// @param mapa_pessoas Hashfile das pessoas que vivem na cidade
/// @param mapa_quadras Hashfile das quadras da cidade
/// @param txt Arquivo (.txt) do relatório criado após a leitura do arquivo
/// @param svg Arquivo (.svg) da saída visual criada após a leitura do arquivo
void processa_qry(const char *caminho_qry, exhash_t *mapa_pessoas, exhash_t *mapa_quadras, FILE *txt, FILE *svg);

#endif //PROJETO_01_EDII_QRY_HANDLER_H

<div align="center"> 

# Projeto 01 - Estrutura de Dados II (EDII)

![Autor](https://img.shields.io/badge/Autor-Pietro%20Fernando%20Pelizon-maroon)
![C99](https://img.shields.io/badge/Language-C99-blue)
![Build](https://img.shields.io/badge/Makefile-GCC-green)
![Data Structures](https://img.shields.io/badge/ED-Hashmap-orange)
![Testing](https://img.shields.io/badge/Test-Unity_Framework-red)

</div>

## Estrutura do Repositório

```text
.
├── docs/
├── include/           # Arquivos de cabeçalho (.h)
├── src/               # Implementações (.c) e Makefile
├── testes/            # Arquivos de comando (.geo, .qry e .pm)
├── unit_test/         # Suítes de testes unitários (t_*.c)
├── unity/             # Framework Unity para testes em C
└── README.md          # Documentação do repositório
```


## Sobre o Projeto

Este projeto desenvolve um Sistema de Informações Geográfica (SIG) simulado, focado na implementação e uso de um **Hash Extensível (Extendible Hashing)**. O sistema gerencia entidades urbanas, como **Quadras** e **Habitantes**, processando dados de entrada e gerando saídas visuais vetoriais e relatórios textuais.

O desenvolvimento inclui uma forte base de testes automatizados, utilizando o framework **Unity** para garantir a integridade das estruturas de dados e dos módulos interpretadores (parsers).

### Principais Funcionalidades

* **Hash Extensível:** Implementação da estrutura de dados para indexação e busca eficiente de registros utilizando chaves, suportando expansão dinâmica de diretório.
* **Gestão Urbana:** Manipulação e consulta de dados referentes a `Quadras` e `Habitantes`.
* **Processamento de Arquivos Multiextensão:**
  * `.geo`: Configuração inicial do ambiente geográfico (ex: quadras e formas básicas).
  * `.qry`: Consultas e operações dinâmicas no mapa.
  * `.pm`: Arquivos de povoamento e dados de habitantes.


* **Geração Gráfica:** Exportação visual do estado do mapa e resultados das consultas em formato `.svg`.
* **Testes Unitários:** Ampla cobertura de testes para os módulos principais (TDD) através do diretório `unit_test/`.

## Compilação

O projeto inclui um `Makefile` na pasta `src` para facilitar a compilação do executável principal e das suítes de teste.

* Para compilar o projeto principal, execute:

```bash
cd src && make
```

## Testes Unitários (Unity Framework)
Para garantir a integridade de cada **TAD** (Tipo Abstrato de Dado) e módulo funcional, o projeto utiliza o framework **Unity**.

Os arquivos de teste estão localizados na pasta `unit_test/` com o prefixo `t_*.c`. O `Makefile` está configurado para compilar e executar isoladamente cada suíte de testes.

- Para rodar um teste específico, de dentro da pasta `src/`, utilize o prefixo `t_` seguido do nome do módulo. Por exemplo:
```bash
make t_habitante
```

- Para limpar os binários gerados pela compilação e testes:
```Bash
make clean
```

## Como Executar

A execução do programa segue o padrão de passagem de argumentos via CLI para definição de diretórios e arquivos de entrada/saída.

**Sintaxe Básica:**

```bash
./ted -e [dir_entrada] -f [arq.geo] -pm [arq.pm] -o [dir_saida] -q [arq.qry]
```

**Parâmetros:**

| **Parâmetros** | **Obrigatório?** | **Descrição**                                                              |
|----------------|------------------|----------------------------------------------------------------------------|
| `-e entrada`   | Não              | Diretório-base de entrada. Caso omitido, usa o diretório corrente.         |
| `-f arq.geo`   | Sim              | Nome do arquivo base de geometria presente no diretório de `entrada`.      |
| `-pm arq.pm`   | Não              | Nome do arquivo contendo os dados demográficos/pessoas.                    |
| `-o saida`     | Sim              | Diretório-base de saída onde os arquivos `.svg` e `.txt` serão gravados.   |
| `-q arq.qry`   | Não              | Arquivo de consultas e processamento do sistema.                           |
| `-h \| --help` | Não              | Exibe uma mensagem com as informações necessárias para executar o programa | 

## Estrutura dos Arquivos de Entrada

### Arquivo (`.geo`)

Define a aparência, tamanho e localização das quadras da cidade.

| **Comando** | **Parâmetros**          | **Descrição**                                                                                      |
|-------------|-------------------------|----------------------------------------------------------------------------------------------------|
| `q`         | `cep x y w h`           | Insere uma quadra com os atributos especificados.                                                  |
| `cq`        | `sw cor_fill cor_borda` | Define a cor de preenchimento, a espessura e cor da borda das quadras subsequentes a esse comando. |

### Arquivo (`.pm`)
| **Comando** | **Parâmetros**                 | **Descrição                                                                                                                           |
|-------------|--------------------------------|---------------------------------------------------------------------------------------------------------------------------------------|
| `p`         | `cpf nome sobrenome sexo nasc` | Insere habitante identificado por cpf, nomeado (`nome, sobrenome`), de um certo sexo (`M \|F`), nascido numa determinada data (`dd/mm/aaaa`) |
| `m`         | `cpf cep face num compl`       | Informa que um dado habitante identificado por `cpf` mora num dado endereço, composto por: `cep, face, num, compl`                    |

### Arquivo (`.qry`)
| **Comando** | **Parâmetros**                 | **Descrição**                                                                              |
|-------------|--------------------------------|--------------------------------------------------------------------------------------------|
| `rq`        | `cep`                          | Remove quadra identificada por `cep`.                                                      |
| `pq`        | `cep`                          | Obtém o número de moradores que moram na quadra identificada por `cep` (total e por face). |
| `censo`     | `-`                            | Produz diversas estatísticas sobre os habitantes da cidade.                                |
| `h?`        | `cpf`                          | Obtém os dados do habitante identificado por `cpf`.                                        |
| `nasc`      | `cpf nome sobrenome sexo nasc` | Ocorre o nascimento de uma pessoa na cidade. (Olhar comando `p` do arquivo (`pm`)          |
| `rip`       | `cpf`                          | Morre pessoa identificada por `cpf`.                                                       |
| `mud`       | `cpf cep face num compl`       | Morador identificado por `cpf` se muda para novo endereço (`cep, face, num compl`).        |
| `dspj`      | `cpf`                          | Morador identificado por `cpf` é despejado.                                                | 

## Estrutura do Código (Módulos)

* **`main.c`**: Ponto de entrada, manipulação de argumentos da linha de comando e fluxo geral do sistema.
* **`exhash.c/h`**: Implementação da estrutura de Hash Extensível (gerenciamento de diretórios, buckets/páginas e funções de dispersão).
* **`quadra.c/h`**: TAD responsável pelas informações e operações relativas às Quadras da cidade.
* **`habitante.c/h`**: TAD responsável pelo gerenciamento dos Habitantes e seus atributos.
* **`geo_handler.c/h`**: Interpretador (parser) dedicado aos comandos do arquivo `.geo`.
* **`qry_handler.c/h`**: Interpretador (parser) dedicado aos comandos do arquivo `.qry`.
* **`pm_handler.c/h`**: Interpretador (parser) dedicado aos comandos do arquivo `.pm`.
* **`svg_handler.c/h`**: Responsável pela formatação e escrita das tags vetoriais para geração dos arquivos `.svg`.
* **`test/` & `unity/`**: Módulos de teste (`t_exhash.c`, `t_quadra.c`, etc.) criados com o micro-framework Unity para validação individual de cada biblioteca.

# Autor: Pietro Fernando Pelizon
# Adaptado para rodar DENTRO da pasta src/

PROJ_NAME := ted
CC        := gcc

CFLAGS    := -W -Wall -Wextra -ggdb -std=c99 -fstack-protector-all -I../include -I../unity -I../unity -DUNITY_INCLUDE_DOUBLE
LIBS      := -lm

SRCS      := $(wildcard *.c)
OBJETOS   := $(SRCS:.c=.o)

CORE_OBJS := $(filter-out main.o, $(OBJETOS))

# Regras de compilação:

all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJETOS)
	@echo "Linkando $(PROJ_NAME)..."
	@$(CC) -o $@ $^ $(LIBS)
	@echo "Build completo! Executável gerado na pasta src/"

%.o: %.c
	@echo "Compilando $<..."
	@$(CC) -c $(CFLAGS) $< -o $@

# ==========================================================
# TARGETS DE TESTES
# ==========================================================

# Para os testes e o unity, usamos o "../" para avisar que eles estão fora da src/
test_hab: $(CORE_OBJS) ../test/t_habitante.c ../unity/Unity/src/unity.c
	@echo "Compilando testes do Habitante..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Executando testes..."
	@./$@

test_geo: $(CORE_OBJS) ../test/t_geo_handler.c ../unity/Unity/src/unity.c
	@echo "Compilando testes do geo_handler..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Executando testes..."
	@./$@

test_exhash: $(CORE_OBJS) ../test/t_exhash.c ../unity/Unity/src/unity.c
	@echo "Compilando testes do exhash..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Executando testes..."
	@./$@

test_pm: $(CORE_OBJS) ../test/t_pm_handler.c ../unity/Unity/src/unity.c
	@echo "Compilando testes do pm_handler..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Executando testes..."
	@./$@

test_quadra: $(CORE_OBJS) ../test/t_quadra.c ../unity/Unity/src/unity.c
	@echo "Compilando testes do quadra..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Executando testes..."
	@./$@

test_qry: $(CORE_OBJS) ../test/t_qry_handler.c ../unity/Unity/src/unity.c
	@echo "Compilando testes do qry_handler..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Executando testes..."
	@./$@

clean:
	@echo "Limpando arquivos..."
	@rm -f *.o $(PROJ_NAME) test_hab test_geo test_exhash test_pm test_quadra test_qry
	@echo "✓ Limpeza concluída!"

.PHONY: all clean test_hab test_geo test_hash test_pm test_quadra test_qry
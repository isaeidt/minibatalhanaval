# Compilador e flags de compilação
CC = gcc
CFLAGS = -Wall -Wextra

# Pastas
SRC_DIR = src

# Nomes dos executáveis finais
SERVER_BIN = miniwebserver
ATTACK_BIN = ataque

# Regra principal: compila tudo
all: $(SERVER_BIN) $(ATTACK_BIN)

# Compila o miniwebserver (junto com a lógica dos navios que farão o fork)
$(SERVER_BIN): $(SRC_DIR)/miniwebserver.c $(SRC_DIR)/navios.c
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SRC_DIR)/miniwebserver.c $(SRC_DIR)/navios.c

# Compila o processo de ataque (que roda separado)
$(ATTACK_BIN): $(SRC_DIR)/ataque.c
	$(CC) $(CFLAGS) -o $(ATTACK_BIN) $(SRC_DIR)/ataque.c

# Limpa os arquivos gerados (útil para recomeçar os testes)
clean:
	rm -f $(SERVER_BIN) $(ATTACK_BIN) fifo_navios estado.json

.PHONY: all clean
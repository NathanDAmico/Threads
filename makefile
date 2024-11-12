# Nome do executável
EXEC = mergesort

# Compilador
CC = gcc

# Arquivos fonte
SRC = mergesort.c

# Alvo padrão
all: $(EXEC)

# Regra para compilar o executável
$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

# Limpeza dos arquivos compilados
clean:
	rm -f $(EXEC)

# Executa o programa com argumentos de exemplo
run: $(EXEC)
	./$(EXEC) 4 arq1.dat arq2.dat arq3.dat arq4.dat arq5.dat -o saida.dat
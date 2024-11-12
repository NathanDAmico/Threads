#include <stdio.h> // Biblioteca padrão de entradas e saídas 
#include <stdlib.h> // Biblioteca responsável pela manipulação e alocação de memória
#include <pthread.h> // Biblioteca padrão para criação e manipulação de threads
#include <string.h> // Biblioteca para manipulação de strings (arrays e chars)
#include <time.h> // Biblioteca para manipulação de unidades de tempo 
#include <locale.h> // Biblioteca para inclusão de caracteres especiais no código 


// Estrutura usada para passar argumentos para as threads
typedef struct
{
    int *vetor;            // Ponteiro para o vetor a ser ordenado
    int inicio;            // Índice inicial do vetor a ser ordenado pela thread
    int fim;               // Índice final do vetor a ser ordenado pela thread
    double *tempoExecucao; // Ponteiro para armazenar o tempo de execução da thread
    int numeroThread;      // Identificador da thread

} ThreadArgs;

// Função de comparação usada pelo qsort para ordenar inteiros
int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

// Função executada por cada thread para ordenar uma parte do vetor
void *routine(void *arg)
{
    struct timespec inicio, fim; // Variáveis para medir o tempo de execução
    ThreadArgs *args = (ThreadArgs *)arg;

    // Obtendo o tempo inicial
    clock_gettime(CLOCK_REALTIME, &inicio);

    // Ordena parte do vetor usando qsort()
    qsort(args->vetor + args->inicio, args->fim - args->inicio, sizeof(int), compare);

    // Obtendo o tempo final e calculando o tempo de execução
    clock_gettime(CLOCK_REALTIME, &fim);
    *args->tempoExecucao = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    return NULL;
}

// Função para mesclar duas partes ordenadas do vetor (parte do algoritmo de merge sort)
void merge(int *vetor, int inicio1, int fim1, int inicio2, int fim2)
{
    int n = fim2 - inicio1;                     // Tamanho do vetor mesclado
    int *temp = (int *)malloc(n * sizeof(int)); // Vetor temporário para armazenar os elementos mesclados
    int i = inicio1, j = inicio2, k = 0;

    // Mescla as duas partes ordenadas
    while (i < fim1 && j < fim2)
    {
        if (vetor[i] < vetor[j])
        {
            temp[k++] = vetor[i++];
        }
        else
        {
            temp[k++] = vetor[j++];
        }
    }

    // Copia os elementos restantes da primeira parte, se houver
    while (i < fim1)
    {
        temp[k++] = vetor[i++];
    }

    // Copia os elementos restantes da segunda parte, se houver
    while (j < fim2)
    {
        temp[k++] = vetor[j++];
    }

    // Copia os elementos mesclados de volta para o vetor original
    for (i = 0; i < n; i++)
    {
        vetor[inicio1 + i] = temp[i];
    }

    free(temp); // Libera a memória alocada para o vetor temporário
}

// Função para verificar se o número de threads fornecido é válido (2, 4 ou 8)
int validaEntradaThreads(int threadsNumber)
{
    int validThreads[4] = {1, 2, 4, 8};
    for (int i = 0; i < 4; i++)
    {
        if (threadsNumber == validThreads[i])
        {
            return 1; // Número de threads é válido
        }
    }
    return -1; // Número de threads inválido
}

// Função para processar os argumentos da linha de comando
int filtraArgumentos(int argc, char *argv[], char *argumento)
{
    int i;
    int arquivos = 0;
    int indiceArquivoSaida;

    // Conta a quantidade de arquivos de entrada e localiza o índice do arquivo de saída
    for (i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            indiceArquivoSaida = i + 1;
            break;
        }
        arquivos++;
    }

    // Retorna a quantidade de arquivos de entrada ou o índice do arquivo de saída
    if (strcmp(argumento, "arquivosDeEntrada") == 0)
    {
        return arquivos;
    }
    if (strcmp(argumento, "arquivoDeSaida") == 0)
    {
        return indiceArquivoSaida;
    }
    return -1; // Argumento inválido
}

int main(int argc, char *argv[])
{
    int i;
    int num_threads = atoi(argv[1]); // Convertendo argumento para inteiro (número de threads)
    int arquivos;
    int somaNumeros = 0; // Contador de números nos arquivos
    char *nomeArquivoDeSaida;
    int *vetorNumeros = NULL;

    // Verifica se o número de threads é válido
    if (validaEntradaThreads(num_threads) != 1)
    {
        printf("Numero de threads invalido! Aceitos somente 1, 2, 4 ou 8\n");
        return -1;
    }

    // Obtém o número de arquivos de entrada e o nome do arquivo de saída
    arquivos = filtraArgumentos(argc, argv, "arquivosDeEntrada");
    nomeArquivoDeSaida = argv[filtraArgumentos(argc, argv, "arquivoDeSaida")];

    // Lê os arquivos de entrada e conta a quantidade total de números
    for (i = 0; i <= arquivos - 1; i++)
    {
        FILE *arquivo = fopen(argv[i + 2], "r");
        if (arquivo == NULL)
        {
            printf("Erro ao abrir arquivo %s\n", argv[i + 2]);
            return -1;
        }
        int numero;
        while (fscanf(arquivo, "%d", &numero) == 1)
        {
            somaNumeros++;
        }
        fclose(arquivo);
    }

    // Aloca memória para armazenar todos os números
    vetorNumeros = (int *)malloc(somaNumeros * sizeof(int));

    // Verifica se a alocação foi bem-sucedida
    if (vetorNumeros == NULL)
    {
        printf("Erro ao alocar memória para o vetor de números.\n");
        return -1;
    }

    // Preenche o vetor com os números dos arquivos de entrada
    int indice = 0;
    for (i = 0; i <= arquivos - 1; i++)
    {
        FILE *arquivo = fopen(argv[i + 2], "r");
        if (arquivo == NULL)
        {
            printf("Erro ao abrir arquivo %s\n", argv[i + 2]);
            return -1;
        }
        int numero;
        while (fscanf(arquivo, "%d", &numero) == 1)
        {
            vetorNumeros[indice++] = numero;
        }
        fclose(arquivo);
    }

    int numerosPorThread = somaNumeros / num_threads; // Número de elementos por thread

    // Aloca memória para as threads, argumentos e para armazenar os tempos de execução
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    ThreadArgs *args = (ThreadArgs *)malloc(num_threads * sizeof(ThreadArgs));
    double *temposExecucao = (double *)malloc(num_threads * sizeof(double));

    // Verifica se a alocação foi bem-sucedida
    if (threads == NULL || args == NULL || temposExecucao == NULL)
    {
        printf("Erro ao alocar memória para as threads.\n");
        return -1;
    }

    // Criação das threads para ordenar partes do vetor
    for (i = 0; i < num_threads; i++)
    {
        args[i].vetor = vetorNumeros;
        args[i].inicio = i * numerosPorThread;
        args[i].fim = (i == num_threads - 1) ? somaNumeros : (i + 1) * numerosPorThread;
        args[i].tempoExecucao = &temposExecucao[i];
        args[i].numeroThread = i;

        pthread_create(&threads[i], NULL, routine, &args[i]); // Cria a thread
    }

    // Espera todas as threads terminarem
    for (i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    double tempoTotal = 0;
    for (int i = 0; i < num_threads; i++)
    {
        printf("Tempo de execução do Thread %d: %.6f segundos.\n",
               args[i].numeroThread + 1, temposExecucao[i]);
        tempoTotal += temposExecucao[i];
    }

    // Calcula o tempo total de execução das threads
    printf("Tempo total de execução das threads: %.6f segundos\n", tempoTotal);

    // Realiza a mesclagem dos blocos ordenados pelas threads
    int tamanhoBloco = numerosPorThread;
    while (tamanhoBloco < somaNumeros)
    {
        for (int i = 0; i < somaNumeros; i += 2 * tamanhoBloco)
        {
            int fim1 = i + tamanhoBloco;
            int inicio2 = fim1;
            int fim2 = i + 2 * tamanhoBloco;

            if (fim1 > somaNumeros)
            {
                fim1 = somaNumeros;
            }
            if (fim2 > somaNumeros)
            {
                fim2 = somaNumeros;
            }

            merge(vetorNumeros, i, fim1, inicio2, fim2);
        }
        tamanhoBloco *= 2;
    }

    // Grava o vetor ordenado no arquivo de saída
    FILE *arquivoSaida = fopen(nomeArquivoDeSaida, "w");
    if (arquivoSaida == NULL)
    {
        printf("Erro ao abrir arquivo de saída %s\n", nomeArquivoDeSaida);
        return -1;
    }

    for (i = 0; i < somaNumeros; i++)
    {
        fprintf(arquivoSaida, "%d\n", vetorNumeros[i]);
    }
    fclose(arquivoSaida);

    // Libera memória alocada
    free(vetorNumeros);
    free(threads);
    free(args);
    free(temposExecucao);

    return 0;
}

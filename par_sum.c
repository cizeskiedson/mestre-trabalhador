/*
TRABALHO 1 - PROGRAMACAO CONCORRENTE

autor: edson cizeski 
ra: 107514

Codigo mestre-trabalhador utilizando tecnicas de paralelismo
para ler tarefas de um arquivo e executa-las
*/

/* IMPORTS DE BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>


/* STRUCTS */
struct tarefa{
    int id;
    char comando;
    short int tempo;
    struct tarefa *prox;
};

typedef struct{   
  pthread_t thread_id;        
  int       thread_num;
}thread_info;

/* CONSTANTES GLOBAIS */
#define MESTRE 0
#define END_LINE '\n'
#define PAUSA 'e'

/* VARIAVEIS GLOBAIS */
int threads = 0;
int n = 0;

long soma = 0;
long impar = 0;
long minimo = INT_MAX;
long maximo = INT_MIN; 

struct tarefa *cabeca;
char **buffer;
bool acabou_programa = false;
bool ativo = false;

/* MUTEX LOCKS */
pthread_mutex_t lock_trabalhador;
pthread_mutex_t lock_mestre;
pthread_mutex_t lock_fila;
pthread_cond_t cond_mestre;
pthread_cond_t cond_trabalhador;

/* CABECALHO DE FUNCOES */
void le_arquivo(char* filename);
struct tarefa* le_tarefa(int i);
static void *mestre();
static void *trabalhador();
int calcula_tamanho_arq(FILE* arq);
void inicializa_buffer();
void do_stuff(int tempo);
int adiciona(struct tarefa *no);
void mostra_tarefas();
void remove_comando(char comando);

/* FLUXO PRINCIPAL */
int main(int argc, char *argv[]){
    printf("Bem Vindo\n");
    int opt, tnum, s;
    char *filename;
    thread_info *tinfo;
    
    pthread_mutex_init(&lock_trabalhador, NULL);
    pthread_mutex_init(&lock_mestre, NULL);
    pthread_mutex_init(&lock_fila, NULL);
    cabeca = (struct tarefa*) malloc(sizeof(struct tarefa));
    cabeca->prox = NULL;
    while ((opt = getopt(argc, argv, "f:t:")) != -1) {
        switch (opt) {
        case 'f': /*Nome do arquivo*/
            filename = optarg;
            break;
        case 't': /*Numero de threads*/
            threads = strtoul(optarg, NULL, 0);
        break;
            default:
            fprintf(stderr, "Erro ao usar os argumentos...\n");
            exit(EXIT_FAILURE);
        return -1;
        }
    }
    printf("Comecando a leitura do arquivo %s com %d\n", filename, threads);
    le_arquivo(filename);
    printf("Encerrando a leitura\n");
    tnum = 0; 
    pthread_t thread_mestre;
    pthread_t thread_trabalhador[threads-1];
    printf("Paralelizando...\n");
    s = pthread_create(&thread_mestre, NULL, &mestre, NULL);
    for (tnum = 0; tnum < threads - 1; tnum++) {
        s = pthread_create(&thread_trabalhador[tnum], NULL, &trabalhador, NULL);
        if (s != 0)
            printf("teste");
             //  handle_error_en(s, "pthread_create");
    }

    s = pthread_join(thread_mestre, NULL);
    for (tnum = 0; tnum < threads - 1; tnum++) {
        s = pthread_join(thread_trabalhador[tnum], NULL);
        if (s != 0)
            printf("join");
          //  handle_error_en(s, "pthread_join");
    }
    printf("Encerrando programa...\n");
    printf("Resultado final: %ld %ld %ld %ld\n", soma, impar, minimo, maximo);
    exit(EXIT_SUCCESS);
}


/* LE ARQUIVO */
void le_arquivo(char* filename){
    
    FILE *arq;
    char string[10];

    arq = fopen(filename, "r");

    if(!arq){
        printf("Erro na leitura do arquivo!\n");
        exit(EXIT_FAILURE);
    }

    calcula_tamanho_arq(arq);
    inicializa_buffer();

    int i = 0;
    while (!feof(arq)){
        fgets(string, 10, arq);
        strcpy(buffer[i], string);
        i+=1;
    } 

    fclose(arq);
}

/* LE BUFFER PARA A FILA DE TAREFAS */
static void *mestre(){
    char *cmd, *tmp;
    bool acabou_leitura = false;
    int i =  0;
    printf("MESTRE\n");
    do{
        struct tarefa *no;
        printf("%d EXECUCAO\n", i);
        if(i >= n){
            acabou_leitura = true;
        }else{
            no = le_tarefa(i);
            if(no->comando == PAUSA){
                printf("Pausa...\n");
                sleep(no->tempo);
                i+=1;
                printf("Saindo da pausa...\n");
                continue;
            }
        }
        pthread_mutex_lock(&lock_mestre);
        if(!acabou_leitura){
            adiciona(no);
        }
        mostra_tarefas();
        while(cabeca->prox == NULL && acabou_leitura){ 
            printf("acabou programa?\n");
            //quando a fila ficar vazia espera os trabalhadores ativos terminarem
            if(ativo){
                pthread_cond_wait(&cond_mestre, &lock_mestre);
            }
            printf("ACABOU\n");
            acabou_programa = true;
            break;
        }
        pthread_mutex_unlock(&lock_mestre);
        printf("Executou mestre\n");
        pthread_cond_broadcast(&cond_trabalhador); //acordar trabalhadores ociosos
        i+=1;
    }while(!acabou_programa); //espera a fila acabar ativando as threads conforme precisar
    printf("Terminou mestre\n");
    return 0;
} 
/* EXECUTA TAREFAS */
static void *trabalhador(){
    printf("TRABALHADOR\n");
    while(1){
        struct tarefa *no, *aux;
        pthread_mutex_lock(&lock_trabalhador);
        printf("Comecando a executar...\n");
        ativo = true;
        while(cabeca->prox == NULL){ //senao houver mais tarefas disponiveis bloquear
            printf("Trabalhador Ocioso...\n");
            ativo = false;
            pthread_cond_wait(&cond_trabalhador, &lock_trabalhador);
            ativo = true;
            if(acabou_programa){
               printf("Encerrando fluxo do trabalhador...\n");
               pthread_mutex_unlock(&lock_trabalhador);
               return(0);
            }
        } //se chegou aqui, pode puxar uma tarefa e executar
        no = cabeca->prox;
        aux = no->prox;
        cabeca->prox = aux;
        printf("Executando: %c\t%d\n", no->comando, no->tempo);
        do_stuff(no->tempo);  //processa a tarefa
        pthread_mutex_unlock(&lock_trabalhador);
        printf("Executou tarefa\n");
        pthread_cond_signal(&cond_mestre);
    }
}

struct tarefa* le_tarefa(int i){
    char *cmd, *tmp;
    struct tarefa *no = (struct tarefa*) malloc(sizeof(struct tarefa));
    cmd = strtok(buffer[i], " ");
    tmp = strtok(NULL, " ");
    no->comando = *cmd;
    no->id = i;
    no->tempo = atoi(tmp);
    no->prox = NULL;
    printf("Leu tarefa %c\t%d\n", no->comando, no->tempo);
    return no;
}


/* FAZ AS ATRIBUICOES PARA AS VARIAVEIS GLOBAIS */
void do_stuff(int tempo){
    soma = soma + tempo;
    if(tempo % 2 != 0){
        impar += 1;
    }
    if(tempo < minimo){
        minimo = tempo;
    }
    if(tempo > maximo){
        maximo = tempo;
    }
    printf("%ld %ld %ld %ld\n", soma, impar, minimo, maximo);
}

/* INSERE NOVA TAREFA NA LISTA */
int adiciona(struct tarefa *no){
    struct tarefa *aux, *anterior;
    anterior = cabeca;
    aux = cabeca->prox;
    while(aux != NULL){
        anterior = aux;
        aux = aux->prox;
    }
    aux = no;
    anterior->prox = aux;
    printf("Adicionou nova tarefa %d %c %d\n", aux->id, aux->comando, aux->tempo);
}

void mostra_tarefas(){
    struct tarefa *aux;
    printf("Mostra tarefas\n");
    aux = cabeca->prox;
    while(aux != NULL){
        printf("TAREFA %d %c %d\n", aux->id, aux->comando, aux->tempo);
        aux = aux->prox;
    }
}

/* CALCULA O TAMANHO DO ARQUIVO LIDO */
int calcula_tamanho_arq(FILE* arq){
    char c;
    while((c = fgetc(arq)) != EOF){
        if(c == END_LINE){ 
            n+=1;         
        } 
    }
    n+=1;
    fseek(arq, 0, SEEK_SET);
}

/* INICIALIZA O BUFFER */
void inicializa_buffer(){
    buffer = (char **)malloc(n * sizeof(char*));
    for(int i = 0; i < n; i++){
        buffer[i] = (char*)malloc(10*sizeof(char));
    }
}

void remove_comando(char comando){
    struct tarefa *aux, *anterior;
    aux = cabeca;
    while(aux->comando != comando){
        anterior = aux;
        aux = aux->prox;
    }
    anterior->prox = aux->prox;
    free(aux);
}
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


/* STRUCTS */
typedef struct tipo_tarefa{
    char comando;
    short int tempo;
    struct tipo_tarefa *prox;
}tarefa;

typedef struct{   
  pthread_t thread_id;        
  int       thread_num;
  bool      ocioso;
}thread_info;

/* CONSTANTES GLOBAIS */
#define MESTRE 0
#define END_LINE '\n'
#define PAUSA 'c'

/* VARIAVEIS GLOBAIS */
int threads = 0;
int n = 0;

long soma = 0;
long impar = 0;
long minimo = INT_MAX;
long maximo = INT_MIN; 

tarefa *cabeca;
char **buffer;

/* CABECALHO DE FUNCOES */
void le_arquivo(char* filename);
static void *mestre_trabalhador(void *arg);
int le_buffer();
int calcula_tamanho_arq(FILE* arq);
void inicializa_buffer();

/* FLUXO PRINCIPAL */
int main(int argc, char *argv[]){
    int opt, tnum, s;
    char *filename;
    thread_info *tinfo;
    pthread_attr_t attr;
    void *res;
    
    cabeca = (tarefa*) malloc(sizeof(tarefa*));

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

    le_arquivo(filename);

    s = pthread_attr_init(&attr);
    if (s != 0)
       // handle_error_en(s, "pthread_attr_init");

    tinfo = calloc(threads, sizeof(thread_info));
    if (tinfo == NULL)
        //handle_error("calloc");

    for (tnum = 0; tnum < threads; tnum++) {
        tinfo[tnum].thread_num = tnum;
        if(tnum = 0)
            tinfo[tnum].ocioso = false;
        else
            tinfo[tnum].ocioso = true;
        s = pthread_create(&tinfo[tnum].thread_id, &attr,
                &mestre_trabalhador, &tinfo[tnum]);
        if (s != 0)
            printf("teste");
             //  handle_error_en(s, "pthread_create");
    }


    s = pthread_attr_destroy(&attr);
    if (s != 0)
      //  handle_error_en(s, "pthread_attr_destroy");

    for (tnum = 0; tnum < threads; tnum++) {
        s = pthread_join(tinfo[tnum].thread_id, &res);
        if (s != 0)
          //  handle_error_en(s, "pthread_join");
        free(res);
    }
    free(tinfo);
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

/* CODIGO PARALELO */
static void *mestre_trabalhador(void *arg){
    thread_info *tinfo = arg;
    if(!tinfo->ocioso){
        if(tinfo->thread_num == MESTRE){
            le_buffer();
        }
    }
}

/* LE BUFFER PARA A FILA DE TAREFAS */
int le_buffer(){
    char *cmd, *tmp;
    int i =  0;
    do{
        tarefa no;
        cmd = strtok(buffer[i], " ");
        tmp = strtok(NULL, " ");
        strcpy(no.comando, cmd);
        strcpy(no.tempo, tmp);

        if(no.comando == PAUSA){
            //parar leitura
            break;
        }
        i+=1;
    }while(1);
}
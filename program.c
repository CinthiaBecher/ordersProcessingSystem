#define _GNU_SOURCE
#define SIZE 300
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

//Criacao de pedidos
char* origens[] = {"Aplicativo", "Site", "Loja Física"};
char* fretes[] = {"Expresso", "Normal"};
int codigo = 1000; //cada pedido tem um código diferente

struct Pedido {
    char origem[100];
    char frete[100];
    int valor_total;
    char id_codigo[20]; 
};

//estruturas 
struct Pedido esteira[SIZE];
struct Pedido frete_expresso[SIZE];
struct Pedido frete_normal[SIZE];

//Variaveis para FIFO
int fim = - 1;
int inicio = - 1;

int n_pedidos = 0;
int qtdd_origens[] = {0,0,0}; //qtdd app, site, loja fisica
int qtdd_expresso = 0;
int qtdd_normal = 0;
int qtdd_despachado = 0;

void print_estatisticas();
void print_estrutura();
void espera_tempo_aleatorio();
struct Pedido cria_pedido();
void* produtor();
void* consumidor();

//mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condicao = PTHREAD_COND_INITIALIZER;

void print_estatisticas(){
    //recebe a data
    time_t data = time(NULL);
    struct tm tm = *localtime(&data);

    //printa infos
    printf("\n\n========================================================\n");
    printf("\tESTATÍSTICAS DOS PEDIDOS [%d/%d/%d]\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    printf("---------------------------------------------------------\n");

    printf("\tPEDIDOS RECEBIDOS\t%d\n", fim+1 ); //+1 contabiliza O 0
    printf("\t ↳ APLICATIVO\t\t%d\n", qtdd_origens[0]);
    printf("\t ↳ SITE\t\t\t%d\n", qtdd_origens[1]);
    printf("\t ↳ LOJA FÍSICA\t\t%d\n\n", qtdd_origens[2]);
    printf("\tPEDIDOS DESPACHADOS\t%d\n", qtdd_despachado);
    printf("\t ↳ EXPRESSO\t\t%d\n", qtdd_expresso);
    printf("\t ↳  NORMAL\t\t%d\n\n", qtdd_normal);
    printf("\tPEDIDOS NA ESTEIRA\t%d\n", n_pedidos);
    printf("========================================================\n");
    
    printf("\tFRETE NORMAL\n\t ↳ ");
    print_estrutura(1, frete_normal, qtdd_normal, 0, qtdd_normal-1);

    printf("\n\tFRETE EXPRESSO\n\t ↳ ");
    print_estrutura(1, frete_expresso, qtdd_expresso, 0, qtdd_expresso-1);

    printf("\n\tESTEIRA\n\t ↳ ");
    print_estrutura(1, esteira, n_pedidos, inicio, fim);

    kill(getpid(), SIGKILL);
}

void print_estrutura(int arg, struct Pedido estrutura[], int tam_est, int inicio, int fim){
    if(tam_est == 0) printf(" X VAZIA X\n");
    else{
        if (arg == 0){
            for (int i = inicio; i <= fim; i++)
                printf("⌧  ");
        }else{
            for (int i = inicio; i <= fim; i++)
                printf("[%s]", estrutura[i].id_codigo);
	    }
        printf(" \n");
        
    }
}

struct Pedido cria_pedido(int arg){
    struct Pedido novo_pedido;
    strcpy(novo_pedido.origem, origens[arg]);
    qtdd_origens[arg]++;

    strcpy(novo_pedido.frete, fretes[rand() % 2]);
    novo_pedido.valor_total = rand() % 800 + 100; //valor total é randomico

    //cada pedido tem um codigo identificador
    char str_codigo[20]; //para realizar a concatenação
    sprintf(str_codigo, "%d", codigo++);
    if(arg == 0) strcat(str_codigo, "A");
    else if (arg == 1) strcat(str_codigo, "S");
    else strcat(str_codigo, "F");
    strcpy(novo_pedido.id_codigo, str_codigo);
   
    return novo_pedido;
}

void espera_tempo_aleatorio(void *arg){
    //Recebe o argumento para gerar tempos diferentes para produtor ou consumidor
    int n = *(int*) arg;
    useconds_t tempo_espera;
    tempo_espera = random() % n;
    usleep(tempo_espera);
}

void* produtor(void *arg){
    int n_thread = *(int*) arg; //cada thread cria um produto de origem diferente
    int tempo = 80000; //tempo diferente do produtor e consumidor

    while(1){
        espera_tempo_aleatorio(&tempo);

        pthread_mutex_lock(&mutex);

        if (inicio == - 1) inicio = 0; //FIFO

        //Cria novo pedido
        struct Pedido novo_pedido;
        novo_pedido = cria_pedido(n_thread);
               
        //printa novo pedido
        if(strcmp(novo_pedido.frete, "Expresso") == 0)
            printf("%d  + [ %s | E | R$%d ]\t\t", gettid(), novo_pedido.id_codigo, novo_pedido.valor_total);
        else
            printf("%d  + [ %s | N | R$%d ]\t\t", gettid(), novo_pedido.id_codigo, novo_pedido.valor_total);
        
        fim = fim + 1;
        esteira[fim] = novo_pedido;
        n_pedidos++;

        if(n_pedidos == 1) pthread_cond_signal(&condicao);
                
        print_estrutura(0, esteira, n_pedidos, inicio, fim);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumidor(){
    int tempo = 120000; //tempo diferente do produtor e consumidor
    
    while(1){
        pthread_mutex_lock(&mutex);
        
        while(n_pedidos == 0){
            pthread_cond_wait(&condicao, &mutex);
        }

		espera_tempo_aleatorio(&tempo);

    //Despachando conforme frete
    if( strcmp(esteira[inicio].frete, "Expresso") == 0){
      printf("%d  - [ %s | E | R$%d ]\t\t", gettid(), esteira[inicio].id_codigo, esteira[inicio].valor_total);
      frete_expresso[qtdd_expresso++] = esteira[inicio];
    }else{
      printf("%d  - [ %s | N | R$%d ]\t\t", gettid(), esteira[inicio].id_codigo, esteira[inicio].valor_total);
      frete_normal[qtdd_normal++] = esteira[inicio];
    }

    qtdd_despachado++;
    inicio = inicio + 1;
    n_pedidos--;

	  print_estrutura(0, esteira, n_pedidos, inicio, fim);

		pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(){
    //Signal
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &print_estatisticas;
    sigaction(SIGINT, &sa, NULL);

    printf(" ID | ± |  COD  | F |   R$   |\t\tESTEIRA\n");
    printf("---------------------------------------------------------\n");
    pthread_t threads[5];
    srand(time(NULL));

    int i[] ={0,1,2};
    if(pthread_create(&threads[0], NULL, &produtor, &i[0]) !=0) {
        perror("Falha criação da thread");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&threads[1], NULL, &produtor, &i[1]) !=0) {
        perror("Falha criação da thread");
        exit(EXIT_FAILURE);
    }
	if(pthread_create(&threads[2], NULL, &produtor, &i[2]) !=0) {
        perror("Falha criação da thread");
        exit(EXIT_FAILURE);
    }

    for (int j = 3; j < 5; j++) {
        if(pthread_create(&threads[j], NULL, &consumidor, NULL) !=0){
            perror("Falha criação da thread");
            exit(EXIT_FAILURE);
        }
    }    
    
    pthread_exit(NULL);
    return 0;
}

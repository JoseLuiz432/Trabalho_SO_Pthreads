
/*-------------------------------------------------------------------------------------------------------------------------------------
					Trabalho SO.
					professor: Everthon Valadao
					Aluno1 : Jose Luiz Maciel Pimenta
					Aluno2 : Igor Pedroso
					Aluno3 : Gustavo Dias
-------------------------------------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------------------------
															Bibliotecas Utilizadas
-------------------------------------------------------------------------------------------------------------------------------------*/

#include <pthread.h> //compilar usando gcc -pthread 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> //biblioteca padrao do unix
#include <time.h> //para deixar as coisas mais aleatorias
/*-------------------------------------------------------------------------------------------------------------------------------------
															Constantes Utilizadas 
-------------------------------------------------------------------------------------------------------------------------------------*/

//dos nomes passados no trabalho
#define VANDA 0
#define MARIA 1
#define PAULA 2
#define SUELI 3
#define VALTER 4
#define MARCOS 5
#define PEDRO 6
#define SILAS 7

#define TAM_FILA 8
#define TRUE 1
#define FALSE 0

/*-------------------------------------------------------------------------------------------------------------------------------------
										Estruturas da thread e da fila circular usada para representar a fila do caixa
-------------------------------------------------------------------------------------------------------------------------------------*/
typedef struct {
	int nome;
	int prioridade;
	int aging;
}Dados_Thread;

typedef struct fila{
	Dados_Thread *elemento;
	struct fila *proximo;
}Fila_encadeada;

/*-------------------------------------------------------------------------------------------------------------------------------------
														Variaveis Globais
-------------------------------------------------------------------------------------------------------------------------------------*/
pthread_mutex_t mu_uso_fila  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mu_uso_caixa  = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t flag[TAM_FILA];
int flag_uso_caixa = FALSE;
Fila_encadeada *primeiro;
int vezes;
/*-------------------------------------------------------------------------------------------------------------------------------------
															Funcoes 
-------------------------------------------------------------------------------------------------------------------------------------*/
void* task_threads(void *arguments);
void AdicionarFila(Dados_Thread *elem);
int FilaVazia();
void AvisaoProx();
void AdicionarFilaEncadeada(Fila_encadeada *posicao, short aging_Fila);
void ImprimeFila();
char MeuNomePrint(int nome);
void Verifica_fila();
void Aging_Fila(Fila_encadeada *p);

/*-------------------------------------------------------------------------------------------------------------------------------------
															MAIN 
-------------------------------------------------------------------------------------------------------------------------------------*/

int main( int argc, char *argv[ ] ){
	if(argc < 2){
		printf("numero de argumentos invalido por favor informe o numero de vezes em que a thread ira rodar\n");
		return -1;
	}
	vezes = atoi(argv[1]); 				//numero de vezes em que cada thread ira rodar
	pthread_t threads[TAM_FILA];
	int i, rc;  						//apenas auxiliares
	Dados_Thread *dados = (Dados_Thread*)malloc(TAM_FILA * sizeof(Dados_Thread)); //alocando os dados das threads
	primeiro = NULL; 					//inicializando variavel da fila
	srand(time(NULL));

	for(i=0; i<TAM_FILA; i++){
		dados[i].nome = i; 				//atribuindo nomes de acordo com seu indice
		dados[i].prioridade = i % 4; 	//porque e ciclico a prioridade logo vanda e ... terao prioridade 0
		dados[i].aging = 0;
		if(rc = pthread_create(&threads[i], NULL, task_threads, &(dados[i]) ) ){
			printf("Error creating the consumer thread..\n");
		}
	}


	for(i=0;i<TAM_FILA;i++){		
		pthread_join(threads[i], NULL); //esperando pelas threads criadas
	}

	return 0;
}

/*
		Tarefa atribuida a cada thread
		como argumento foi passado a estrutura da thread (Dados_Thread)
*/
void* task_threads(void *arguments){
	Dados_Thread *dados = ((Dados_Thread*) arguments); 	// cast do argumento 
	Fila_encadeada *posicao = (Fila_encadeada*)malloc(sizeof(Fila_encadeada));  //caixinha da sua posicao na fila
	posicao->elemento = ((Dados_Thread*) arguments);
	
	pthread_cond_init(&(flag[dados->nome]), NULL); 		//iniciando a flag de cada thread
	
	sleep(rand()%3);  								//apenas demonstrativo para que a thread seja iniciada no loop
	
	for(int i=0;i<vezes;i++){
		
		pthread_mutex_lock(&mu_uso_fila);				//trancando o uso da fila, 
		if(flag_uso_caixa){
			printf("%c Entrou na fila do caixa  ",MeuNomePrint(dados->nome));
			AdicionarFilaEncadeada(posicao, TRUE); 		// adiconando o elemento a fila
			ImprimeFila();
			pthread_cond_wait(&(flag[dados->nome]), &mu_uso_fila);  //esperando ser chamado
			pthread_mutex_unlock(&mu_uso_fila);
		}else {	
			flag_uso_caixa = TRUE;
			pthread_mutex_unlock(&mu_uso_fila);
		}
		//pthread_mutex_lock(&mu_uso_caixa);
		printf("%c Esta sendo atendido(a) ",MeuNomePrint(dados->nome));
		sleep(2);  								//Nao entendi aindo o porque mas esse sleep e nescessario para o correto funcionamento
		/* 
				acho que por monpolio da presente thread no uso da fila impedindo que as demais execute 
		*/
		
		pthread_mutex_lock(&mu_uso_fila);
		ImprimeFila(); 
		if(FilaVazia()){ 							//caso a fila esteja vazia se abre o caixa para o proximo utilizar
			flag_uso_caixa = FALSE;	
		}else{
			AvisaoProx();  							// chama o proximo que esta na fila
		}
		printf("%c Foi atendido\n",MeuNomePrint(dados->nome) );
		pthread_mutex_unlock(&mu_uso_fila);
		
		//pthread_mutex_unlock(&mu_uso_caixa);
		
		/*
			retomando as prioridades para seu estado inicial
		*/
		
		sleep(rand()%4); 							//Sleep para a espera para retornar a usar a loterica 
		dados->prioridade = dados->nome % 4;
		dados->aging = 0;
	}	
	printf("Thread de nome %d terminou\n",dados->nome);

}
/*-------------------------------------------------------------------------------------------------------------------------------------
verifica se fila esta vazia:
	se estiver vazia retorna true
-------------------------------------------------------------------------------------------------------------------------------------*/
int FilaVazia(){
	if(primeiro == NULL){
		return TRUE;
	}else return FALSE;
}
/*-------------------------------------------------------------------------------------------------------------------------------------
Avisa o primeiro da fila que esta esperando
e anda o ponteiro do primeiro
-------------------------------------------------------------------------------------------------------------------------------------*/
void AvisaoProx(){
	pthread_cond_signal(&flag[primeiro->elemento->nome]);
	primeiro = primeiro->proximo;
}
/*-------------------------------------------------------------------------------------------------------------------------------------
Adiciona na fila encadeada na posicao correta e faz o aging nos que ele passou na frente 
-------------------------------------------------------------------------------------------------------------------------------------*/
void AdicionarFilaEncadeada(Fila_encadeada *posicao, short aging_Fila){
	Fila_encadeada *auxiliar = primeiro;
	if(primeiro == NULL){
		primeiro = posicao;
		posicao->proximo = NULL;
	}else{
		if(primeiro->elemento->prioridade < posicao->elemento->prioridade){
			posicao->proximo = primeiro;
			primeiro = posicao;
			if(aging_Fila)
				Aging_Fila(posicao);
			
		}else{
			while((auxiliar->proximo != NULL) && (auxiliar->proximo->elemento->prioridade >= posicao->elemento->prioridade)) {
				auxiliar = auxiliar->proximo;
			}	
			if(auxiliar->proximo == NULL){
				auxiliar->proximo = posicao;
				auxiliar = auxiliar->proximo;
				auxiliar->proximo = NULL; 
			}else{
				posicao->proximo = auxiliar->proximo;
				auxiliar->proximo = posicao;	
				if(aging_Fila)
					Aging_Fila(posicao);
			}
		}
		
	}
}
/*-------------------------------------------------------------------------------------------------------------------------------------
Aging na fila a partir de um ponteiro passado
e realocacao em quem tiver a prioridades modificada
-------------------------------------------------------------------------------------------------------------------------------------*/
void Aging_Fila(Fila_encadeada *p){
	Fila_encadeada *aux = p, *aux2;
	while(aux->proximo != NULL){
		aux->proximo->elemento->aging += 1;
		if(aux->proximo->elemento->aging > 1){
			printf("%c sofreu envelhecimento por inanicao\n",MeuNomePrint(aux->proximo->elemento->nome));
			aux->proximo->elemento->prioridade += 1;
			aux->proximo->elemento->aging = 0;
			aux2 = aux->proximo;
			aux->proximo = aux->proximo->proximo;
			AdicionarFilaEncadeada(aux2, FALSE);
		}else
			aux = aux->proximo;
	}

}
/*-------------------------------------------------------------------------------------------------------------------------------------
	Imprimindo a fila  
-------------------------------------------------------------------------------------------------------------------------------------*/
void ImprimeFila(){
	Fila_encadeada *auxiliar = primeiro;
	printf("{Fila:" );
	while(auxiliar != NULL){
		switch(auxiliar->elemento->nome){
			case VANDA:
				printf("V");
			break;
			case MARIA:
				printf("M");
			break;
			case PAULA:
				printf("P");
			break;
			case SUELI:
				printf("S");
			break;
			case VALTER:
				printf("V");
			break;
			case MARCOS:
				printf("M");
			break;
			case PEDRO:
				printf("P");
			break;
			case SILAS:
				printf("S");
			break;
						

		}
		auxiliar  = auxiliar->proximo;
	}
	printf("}\n");
}
char MeuNomePrint(int nome){
	switch(nome){
			case VANDA:
				return 'V';
			break;
			case MARIA:
				return 'M';
			break;
			case PAULA:
				return 'P';
			break;
			case SUELI:
				return 'S';
			break;
			case 4:
				return 'V';
			break;
			case 5:
				return 'M';
			break;
			case 6:
				return 'P';
			break;
			case 7:
				return 'S';
			break;
		}
}

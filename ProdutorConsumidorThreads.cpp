#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h> 
#include <ctime>

#include <iostream>
#include <list>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::string;

//classe que engloba os dados de buffer e numero de iteracoes V a serem passados para produtores e consumidores
class Data                  
{                           
public:
    std::list<int> buffer;  
    int v;
};

void *insertBuffer(void* data);
void *deleteBuffer(void* buffer);
void *insertMinusOne(void* buffer);

//usados como na especificacao do trabalho
int t=0;                    
 
pthread_mutex_t mutex;
pthread_cond_t condP;
pthread_cond_t condC;

int main(int argc, char const *argv[])
{
    //clock_t time = clock();

    //cout << (float)time/CLOCKS_PER_SEC << " seconds" << endl;

    Data*data = new Data();

    int p,c;

    data->v = atoi(argv[1]);
    p = atoi(argv[2]);
    c = atoi(argv[3]);                              
    t = atoi(argv[4]);                              

    //cout << "V:" << data->v << endl;              //numero de iteracoes V
    
    //cout << "P:" << p << endl;                    //numero de produtores p
    //cout << "C:" << c << endl;                    //numero de consumidores c
    //cout << "T:" << t << endl;                    //tamanho do buffer t

    pthread_t producers[p];
    pthread_t consumers[c];
    pthread_t minusOne[c];

    pthread_mutex_init(&mutex, NULL);               //inicializa mutex
    pthread_cond_init(&condP, NULL);                //inicializa variavel de condicao condP
    pthread_cond_init(&condC, NULL);                //inicializa variavel de condicao condC

    int i;

    //Criacao dos produtores
    for(i=0; i<p; i++)
    {
        pthread_create(&(producers[i]), NULL, insertBuffer, (void*)data);
        //cout << "Criado produtor " << i << endl;
    }

    //Criacao dos consumidores
    for(i=0; i<c; i++)
    {
        pthread_create(&(consumers[i]), NULL, deleteBuffer, (void*)&(data->buffer));
        //cout << "Criado consumidor " << i << endl;
    }

    //Sincronizacao dos produtores
    for(i=0; i<p; i++)
    {
        pthread_join(producers[i],NULL);
        //cout << "Sincronizado produtor " << i << endl;
    }


    for(i=0; i<c; i++)
    {        
        pthread_create(&(minusOne[i]), NULL, insertMinusOne, (void*)&(data->buffer));   
        //cout << "Inserido -1 no buffer: " << i << endl;
    }


    //Sincronizacao dos consumidores
    for(i=0; i<c; i++)
    {
        pthread_join(consumers[i],NULL);
        //cout << "Sincronizado consumidor " << i << endl;
    }

    delete data;

    
    //time = clock() - time;
    //cout << (float)time/CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}


//Verifica se o buffer atingiu seu tamanho maximo
bool isMaxSize(std::list<int> l)
{
    if(l.size() == t)
        return true;
    return false;
}

//Verifica se o buffer esta vazio
bool isEmpty(std::list<int> l)
{   
    if(l.size() == 0)
        return true;
    return false;
}

//insere inteiros randomicos no buffer
void *insertBuffer(void* dataFormal)
{
    //cast do ponteiro void passado para ponteiro Data a ser usado
    Data *funcData = (Data *)dataFormal;

    int i, num;

    //seed dos numeros randomicos
    srand(time(NULL));

    for(i=0; i<funcData->v; i++)
    {
        /*inicio da secao critica*/
        pthread_mutex_lock(&mutex);                 

        //bloqueia a thread produtora corrente enquanto o buffer esta cheio
        while(isMaxSize(funcData->buffer))          
        {
            pthread_cond_wait(&condP, &mutex);
        }
        
        //gera numero randomico
        num = rand();//%4096;

        //insere numero no buffer
        funcData->buffer.push_front(num);

        //acorda um thread consumidora na espera da variavel de condicao condC, se houver
        pthread_cond_signal(&condC);

        pthread_mutex_unlock(&mutex);
        /*fim da secao critica*/
    }
}


//deleta inteiros do buffer
void *deleteBuffer(void* buffer)
{
    //cast do ponteiro void passado a funcao para ponteiro std::list<int> a ser usado
    std::list<int> *funcBuffer = (std::list<int> *)buffer;

    int num;

    //loop infinito do consumidor
    for(;;)
    {
        /*inicio da secao critica*/
        pthread_mutex_lock(&mutex);    

        //bloqueia a thread consumidora corrente enquanto o buffer esta vazio
        while(isEmpty(*funcBuffer))
        {
            pthread_cond_wait(&condC, &mutex); 
        }

        //primeiro a entrar eh o primeiro a sair(FIFO)
        num = funcBuffer->back();
        funcBuffer->pop_back();

        //acorda um thread produtor na espera da variavel de condicao condP, se houver
        pthread_cond_signal(&condP);

        pthread_mutex_unlock(&mutex);
        /*fim da secao critica*/

        //se num for igual a -1, o acesso de outras threads eh liberado e a thread atual eh abandonada
        if(num == -1)
        {
            pthread_exit(NULL);
        }

        ///*
        bool isPrime = true;

        //se for menor que 2, nao eh primo
        if(num < 2)
            isPrime = false;
        else
        {
            int i;
            for (i = 2; i <= sqrt(num); i++) 
            {
                //se for maior que 2, e possuir mais divisores alem de 1 e ele mesmo, nao eh primo 
                if (num % i == 0) 
                {
                    isPrime = false;
                    break;
                }
            }
        }

        if(isPrime)
        {
            //mutex utilizado para garantir a exibicao correta dos dados
            pthread_mutex_lock(&mutex);
            cout << "[" << int(pthread_self()) << ":" << num << "]"<< endl;
            pthread_mutex_unlock(&mutex);
        }
        //*/
    }
}

//Insere -1 no buffer
void* insertMinusOne(void* buffer)
{
    //cast do ponteiro void passado a funcao para ponteiro std::list<int> a ser usado
    std::list<int> *funcBuffer = (std::list<int> *)buffer;

    /*inicio da secao critica*/
    pthread_mutex_lock(&mutex);

    //bloqueia a thread produtora corrente enquanto o buffer esta cheio
    while(isMaxSize(*funcBuffer))
        pthread_cond_wait(&condP, &mutex);

    funcBuffer->push_front(-1);

    //acorda um thread cosumidor, se houver, pois o buffer ja nao esta mais vazio
    pthread_cond_signal(&condC);

    pthread_mutex_unlock(&mutex);
    /*fim da secao critica*/

    //finaliza thread
    pthread_exit(NULL);

}

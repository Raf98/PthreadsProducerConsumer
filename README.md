<h1>Compilação e Execução

Para compilar o programa em ambiente Linux, deve ser executado o seguinte
comando no terminal, na pasta em que o arquivo .cpp está:

**g++ -pthread ProdutorConsumidorThreads.cpp -o prodCons**

Após compilado, basta executar o programa no terminal, utilizando o seguinte
formato:

__./prodCons *v p c t* __
  
onde:

  *v* é o número de iterações para inserção de elementos no buffer para os threads
produtores;

  *p* é o número de threads produtores;

  *c* é o número de threads consumidores;

  *t* é o tamanho máximo do buffer.

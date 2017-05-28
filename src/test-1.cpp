#include <iostream>
#include <cstdlib>
#include "ConcurrentHashMap.hpp"
#include "ListaAtomica.hpp"
#include <assert.h>     /* assert */


using namespace std;

static void *thread1(void* hashmap)
{
	ConcurrentHashMap* hashmap1 =(ConcurrentHashMap*) hashmap;

	hashmap1->addAndInc("lucas");
	hashmap1->addAndInc("santiago");
	hashmap1->addAndInc("federico");
	hashmap1->addAndInc("pedro");
	hashmap1->addAndInc("bruno");
	hashmap1->addAndInc("pedro");
}

static void *thread2(void* hashmap)
{
	ConcurrentHashMap* hashmap1 =(ConcurrentHashMap*) hashmap;
	hashmap1->addAndInc("lucas");
	hashmap1->addAndInc("pedro");
	hashmap1->addAndInc("bruno");
	hashmap1->addAndInc("santiago");
	hashmap1->addAndInc("federico");
}





Lista<int> l;
void* incrementar(void* i){
	int indice = *((int*)i);
	l.push_front(indice);
}
int main(int argc, char **argv) {

	//Probando push_front de lista. 
	//Si no funcionaria explotaria al querer agregar dos threads en la misma lista.
	pthread_t thread[1000];
	int arr[1000];
	for (int i = 0; i < 1000; ++i)
	{
		arr[i]=i;
	}
	unsigned int i=0;
	for (i = 0; i < 1000; ++i)
	{
		pthread_create(&thread[i],NULL,&(incrementar),&arr[i]);
	}
	void** returnResult;
	for (int i = 0; i < 1000; ++i)
	{
		pthread_join((thread[i]),returnResult);
	}
	Lista<int>::Iterador iteradorListaActual= l.CrearIt();
	int j=0;
	
	while(iteradorListaActual.HaySiguiente()){
		j++;
		iteradorListaActual.Avanzar();
	}

	assert(j==1000);
	ConcurrentHashMap c;
	pthread_t threada;
	pthread_t threadb;
	pthread_create(&threada,NULL,&thread1,&c);
	pthread_create(&threadb,NULL,&thread2,&c);
	pthread_join(threada,NULL);
	pthread_join((threadb),NULL);
	std::pair <std::string,unsigned int> maximoDeTodos ("pedro",3);
	assert(c.member("lucas"));
	assert(c.member("santiago"));
	assert(c.member("federico"));
	assert(c.member("bruno"));
	assert(c.member("pedro"));

	assert(c.maximum(1) == maximoDeTodos);
	assert(c.maximum(10) == maximoDeTodos);
	assert(c.maximum(20) == maximoDeTodos);
	assert(c.maximum(30) == maximoDeTodos);

	return 0;
}


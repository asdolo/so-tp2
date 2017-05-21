#include <iostream>
#include <cstdlib>
#include "ConcurrentHashMap.hpp"
#include "ListaAtomica.hpp"
using namespace std;

Lista<int> l;
void* incrementar(void* i){
	int indice = *((int*)i);
	l.push_front(indice);
}
int main(int argc, char **argv) {

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
	for (int i = 0; i < 1000; ++i)
	{
		cout << l.iesimo(i) << "\n";
	}

	
	

	return 0;
}


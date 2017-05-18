#ifndef CONCURRENT_HASHMAP_H__
#define CONCURRENT_HASHMAP_H__

#include <atomic>
#include "ListaAtomica.hpp"
#include <utility>      // std::pair, std::make_pair
#include <string>       // std::string
#include <list>
#include <fstream>

using namespace std;
//template <typename T>
class ConcurrentHashMap {
private:
	
	pair<string,unsigned int> _maximos[26];

public:
	struct datos_process_file {
		string filePath;
		ConcurrentHashMap* c;
	};

	Lista<pair<string,unsigned int>>* tabla[26];

	ConcurrentHashMap() {
		for (int i = 0; i < 26; i++)
		{
			tabla[i]= new Lista<pair<string,unsigned int>>();
		}
	}

	~ConcurrentHashMap() {
		
	}


	void addAndInc(string key) {
		bool loEncontre=false;
		unsigned int posicion= mapCharToInt(key.at(0));
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[posicion]->CrearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first.compare(key)==0)
			{
				iteradorListaActual.Siguiente().second++;
				loEncontre=true;
				break;
			}
			iteradorListaActual.Avanzar();
		}
		if (!loEncontre)
		{
			
			
			tabla[posicion]->push_front(pair<string, unsigned int>(key,1));
		}
	}

	bool member(string key) {
		bool loEncontre=false;
		unsigned int posicion= mapCharToInt(key.at(0));
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[posicion]->CrearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first.compare(key)==0)
			{
				loEncontre=true;
				break;
			}
			iteradorListaActual.Avanzar();
		}
		return loEncontre;
	}

	unsigned int mapCharToInt(char a){
		return (unsigned int)a-97;
	}
	static void* maximumFila(void* ind){
		
	
	}

	void *maximumFila(int ind)
    {
       unsigned int indice=   ind;
		unsigned int maximo=0;

		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[indice]->CrearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().second>maximo)
			{
				maximo=iteradorListaActual.Siguiente().second;
				_maximos[indice] = iteradorListaActual.Siguiente();
			}
			iteradorListaActual.Avanzar();
		}
    }

    static void *thread_func(void *ind)
    {
    	ConcurrentHashMap c;

    	c.maximumFila(*((unsigned int*)ind));        
    }


	pair<string, unsigned int> maximum(unsigned int nt){
		pthread_t thread;
		unsigned int i=0;
		pthread_create(&thread,NULL,&(ConcurrentHashMap::thread_func),&i);
		void** returnResult;

		//Si tengo nt<26  tengo que esperar que terminen los threads y crear nuevos con create?
		// Espero a que todos terminen de buscar su maximo

		pthread_join(thread,returnResult);
		//Comparo los resultados _maximos
		pair<string,unsigned int> maximoDeLosMaximos=_maximos[0];
		for (int i = 1; i < 26; i++)
		{
			if (_maximos[i].second>maximoDeLosMaximos.second)
			{
				maximoDeLosMaximos=_maximos[i];
			}
		}
		return maximoDeLosMaximos;
	}

	static pair<string, unsigned int> maximum(unsigned int i,unsigned int m ,list<string> l){
		pair<string, unsigned int> p;
		return p;
	}

	static ConcurrentHashMap count_words(string arch){
		std::ifstream file(arch);
    	std::string str; 
    	ConcurrentHashMap c;
	    while (std::getline(file, str))
	    {
	        c.addAndInc(str);
	    }
	    return c;
	}

	 
	static ConcurrentHashMap count_words(list<string> l){
		
		// Un thread por archivo
		pthread_t threads[l.size()];

		ConcurrentHashMap c;
		datos_process_file estructura[l.size()];
		int i=0;
		for (std::list<string>::const_iterator iterator = l.begin(), end = l.end(); iterator != end; ++iterator) {
		    estructura[i].c=&c;
			estructura[i].filePath = *iterator;
			pthread_create(&threads[i], NULL, &(ConcurrentHashMap::thread_process_file_aux), &estructura[i]);
			i++;
		}

	    return c;
	}

	void* thread_process_file(string arch,ConcurrentHashMap* c){
		std::ifstream file(arch);
    	std::string str; 
	    while (std::getline(file, str))
	    {
	        c->addAndInc(str);
	    }
	    return c;
	}

	static void* thread_process_file_aux(void* estructura){
		ConcurrentHashMap c;

		datos_process_file estr = *((datos_process_file*) (estructura));

    	c.thread_process_file(estr.filePath, estr.c);

	}

	static ConcurrentHashMap count_words(unsigned int n,list<string> l){

	    ConcurrentHashMap c;
	    return c;
	}
};

#endif 

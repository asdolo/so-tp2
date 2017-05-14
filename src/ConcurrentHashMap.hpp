#ifndef CONCURRENT_HASHMAP_H__
#define CONCURRENT_HASHMAP_H__

#include <atomic>
#include "ListaAtomica.hpp"
#include <utility>      // std::pair, std::make_pair
#include <string>       // std::string


//template <typename T>
class ConcurrentHashmap {
private:
	Lista* _letras[26];
	pair<string,unsigned int>* _maximos[26];

public:
	ConcurrentHashmap() {
		for (int i = 0; i < 26; i++)
		{
			_letras[i]= new Lista<pair(string,unsigned int)>();
		}
	}

	~ConcurrentHashmap() {
		
	}


	void addAndInc(string key) {
		bool loEncontre=false;
		unsigned int posicion= mapCharToInt(key.at(0));
		Iterador iteradorListaActual= _letras[posicion].crearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first().compare(key)==0)
			{
				iteradorListaActual.Siguiente().second()++;
				loEncontre=true;
				break;
			}
			iteradorListaActual.Avanzar();
		}
		if (!loEncontre)
		{
			_letras[posicion].push_front(new pair(key,1));
		}
	}

	bool member(string key) {
		bool loEncontre=false;
		unsigned int posicion= mapCharToInt(key.at(0));
		Iterador iteradorListaActual= _letras[posicion].crearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first().compare(key)==0)
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
	void maximum(void* ind){
		unsigned int indice= *ind;
		unsigned int maximo=0;

		Iterador iteradorListaActual= _letras[indice].crearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().second()>maximo)
			{
				maximo=iteradorListaActual.Siguiente().second();
				_maximos[indice]=iteradorListaActual.Siguiente();
			}
			iteradorListaActual.Avanzar();
		}
	
	}
pair<string, unsigned int> maximum(unsigned int nt){
	pthread_t thread;
	unsigned int i=0;
	pthread_create(&thread,NULL,maximum,&i);
	void** returnResult;

	//Si tengo nt<26  tengo que esperar que terminen los threads y crear nuevos con create?
	// Espero a que todos terminen de buscar su maximo

	pthread_join(thread,returnResult);
	//Comparo los resultados _maximos
	pair<string,unsigned int> maximoDeLosMaximos=_maximos[0];
	for (int i = 1; i < 26; i++)
	{
		if (_maximos[i].second()>maximoDeLosMaximos.second())
		{
			maximoDeLosMaximos=*(_maximos[i]);
		}
	}
	return maximoDeLosMaximos;
}

};

#endif 

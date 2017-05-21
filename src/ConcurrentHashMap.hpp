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
	
	pthread_mutex_t mutexes[26];
public:
	struct datos_process_file {
		string filePath;
		ConcurrentHashMap* c;

	};

	struct maximumStruct {
		ConcurrentHashMap* clase;
		unsigned int filasDisponibles;
		pthread_mutex_t mutex;
		bool procesadas[26];
		pair<string,unsigned int> _maximos[26];
	};

	struct count_words_n_threads_struct
	{
		ConcurrentHashMap* clase;
		list<string> filePaths;
		bool procesadas[26];
		unsigned int archivosDisponibles;
		pthread_mutex_t mutex;
	};

	Lista<pair<string,unsigned int>>* tabla[26];

	ConcurrentHashMap() {
		for (int i = 0; i < 26; i++)
		{
			tabla[i]= new Lista<pair<string,unsigned int>>();
			pthread_mutex_init(&mutexes[i], NULL);
		}
	}

	~ConcurrentHashMap() {
		for (int i = 0; i < 26; i++)
		{
			pthread_mutex_destroy(&mutexes[i]);

		}
	}


	void addAndInc(string key) {
		bool loEncontre=false;
		
		// obtenemos la posicion del array correspondiente a la primer letra del string
		unsigned int posicion = mapCharToInt(key.at(0));
		
		pthread_mutex_lock(&mutexes[posicion]);
		// obtenemos un iterador a la lista que tenemos que modificar dentro del hash
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[posicion]->CrearIt();
		
		// buscamos a ver si ya existía el string key
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first.compare(key)==0)
			{
				loEncontre=true;
				break;
			}
			iteradorListaActual.Avanzar();
		}

		if (loEncontre)
		{
			// incremento la cantidad
			iteradorListaActual.Siguiente().second++;
		}
		else
		{
			// no existe el string, lo agrego con cantidad 1
			tabla[posicion]->push_front(pair<string, unsigned int>(key,1));
		}
		pthread_mutex_unlock(&mutexes[posicion]);
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
	

	void *maximumFila(unsigned int ind, pair<string,unsigned int>* arreglo)
    {

		pair<string,unsigned int> maximo;
		maximo.second=0;
		pthread_mutex_lock(&mutexes[ind]);
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[ind]->CrearIt();

		while(iteradorListaActual.HaySiguiente()){
			
			if (iteradorListaActual.Siguiente().second>maximo.second)
			{

			
				maximo=iteradorListaActual.Siguiente();
			}
			iteradorListaActual.Avanzar();
		}
		arreglo[ind]=maximo;
		pthread_mutex_unlock(&mutexes[ind]);
		
    }

    static void *thread_func(void* estruc)
    {
    	maximumStruct* estructura = (maximumStruct*)estruc;

    	unsigned int filaAProcesar;
    	
    	pthread_mutex_lock(&(estructura->mutex));
    	while(estructura->filasDisponibles>0){
    		
    		estructura->filasDisponibles--;
    		filaAProcesar = estructura->clase->proximaFilaDisponible((estructura->procesadas));
    		pthread_mutex_unlock(&(estructura->mutex));
    		
    		estructura->clase->maximumFila(filaAProcesar,(estructura->_maximos));

			pthread_mutex_lock(&(estructura->mutex));
    	}
    	pthread_mutex_unlock(&(estructura->mutex));
		
    	
    }
unsigned int proximaFilaDisponible(bool* array){
	for (unsigned int i = 0; i < 26; i++)
	{
		if (array[i]==false)
		{
			array[i]=true;
			return i;
		}			
	}
	//Nunca se deberia ejecutar.
	return 0;
}

	pair<string, unsigned int> maximum(unsigned int nt){
		pthread_t thread[nt];
		maximumStruct estructura;
		estructura.filasDisponibles=26;
		estructura.clase = this;
		pthread_mutex_init(&estructura.mutex, NULL);
		

		for (int i = 0; i < 26; i++)
		{
			estructura.procesadas[i]=false;	
		}
		
		for (int i = 0; i < nt; ++i)
		{
			pthread_create(&thread[i],NULL,&(ConcurrentHashMap::thread_func),&estructura);
		}
		
		for (int i = 0; i < nt; ++i)
		{
			pthread_join((thread[i]),NULL);
		}
		
		//Si tengo nt<26  tengo que esperar que terminen los threads y crear nuevos con create?
		// Espero a que todos terminen de buscar su maximo

		//Comparo los resultados _maximos
		pair<string,unsigned int> maximoDeLosMaximos=estructura._maximos[0];
		cout << "Par mas grande indice:" << "0"<< ". Palabra:" <<estructura._maximos[0].first << ". Cantidad:" << estructura._maximos[0].second << "\n";
		for (int i = 1; i < 26; i++)
		{
			cout << "Par mas grande indice:" << i << ". Palabra:" <<estructura._maximos[i].first << ". Cantidad:" << estructura._maximos[i].second << "\n";  
			if (estructura._maximos[i].second>maximoDeLosMaximos.second)
			{
				maximoDeLosMaximos=estructura._maximos[i];
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
		    estructura[i].c = &c;
			estructura[i].filePath = *iterator;
			pthread_create(&threads[i], NULL, &(ConcurrentHashMap::thread_process_file_aux), &estructura[i]);
			i++;
		}

		for (i = 0; i < l.size(); i++)
		{
			pthread_join((threads[i]),NULL);
		}

	    return c;
	}

	static void thread_process_file(string arch, ConcurrentHashMap* c){
		std::ifstream file(arch);
    	std::string str; 
	    while (std::getline(file, str))
	    {
	        c->addAndInc(str);
	    }
	    
	}

	static void* thread_process_file_aux(void* estructura){

		datos_process_file estr = *((datos_process_file*) (estructura));
		cout << estr.filePath << "\n";
		cout << estr.c << "\n";

    	estr.c->thread_process_file(estr.filePath, estr.c);

	}

	static ConcurrentHashMap count_words(unsigned int n,list<string> l){

	    ConcurrentHashMap c;
	    // Un thread por archivo
		pthread_t threads[n];
		count_words_n_threads_struct estructura;
		estructura.filePaths=l;
		estructura.archivosDisponibles=l.size();
		estructura.clase = &c;
		pthread_mutex_init(&estructura.mutex, NULL);

		for (int i = 0; i < 26; i++)
		{
			estructura.procesadas[i]=false;	
		}
		for (int i = 0; i < n; ++i)
		{
			pthread_create(&threads[i],NULL,&(ConcurrentHashMap::countWordsAuxiliarNThreads),&estructura);
		}
		
		for (int i = 0; i < n; ++i)
		{
			pthread_join((threads[i]),NULL);
		}
		

	    return c;
	}






	 static void *countWordsAuxiliarNThreads(void* estruc)
    {
    	count_words_n_threads_struct* estructura = (count_words_n_threads_struct*)estruc;

    	string archivoAProcesar;
    	
    	pthread_mutex_lock(&(estructura->mutex));
    	while(estructura->archivosDisponibles>0){
    		
    		estructura->archivosDisponibles--;
    		archivoAProcesar = estructura->filePaths.front();
    		 estructura->filePaths.pop_front();
    		pthread_mutex_unlock(&(estructura->mutex));

    		
    		estructura->clase->thread_process_file(archivoAProcesar, estructura->clase);

			pthread_mutex_lock(&(estructura->mutex));
    	}
    	pthread_mutex_unlock(&(estructura->mutex));
		
    	
    }
};

#endif 

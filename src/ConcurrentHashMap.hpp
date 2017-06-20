#ifndef CONCURRENT_HASHMAP_H__
#define CONCURRENT_HASHMAP_H__

#include <atomic>
#include "ListaAtomica.hpp"
#include <utility>      // std::pair, std::make_pair
#include <string>       // std::string
#include <list>
#include <fstream>
#include <vector>
#include <assert.h>     /* assert */
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

	struct maximum_struct {
		ConcurrentHashMap* clase;
		atomic<int> filasDisponibles;
	};

	struct count_words_n_threads_struct
	{
		ConcurrentHashMap* clase;
		std::vector<string> filePaths;
		atomic<int> archivosDisponibles;
		
	};

	struct datos_multiple_hashmap
	{
		vector<ConcurrentHashMap> hashMapsProcesados;
		list<string> archivosAProcesar;
		pthread_mutex_t mutex;
	};

	Lista<pair<string,unsigned int>>* tabla[26];

	ConcurrentHashMap() {
		//Inicializo el Hash.
		for (int i = 0; i < 26; i++)
		{
			//Creo una lista de pares <Palabra,cantidad de repeticiones> por letra
			tabla[i]= new Lista<pair<string,unsigned int>>();
			//Inicializo los mutex de cada lista.
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

		//Lockeamos la lista de la primer letra de la palabra
		pthread_mutex_lock(&mutexes[posicion]);
		// obtenemos un iterador a la lista que tenemos que modificar dentro del hash
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[posicion]->CrearIt();
		
		// buscamos a ver si ya existía el string key
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first.compare(key)==0)
			{
				//Era la palabra. Entonces salgo del ciclo.
				loEncontre=true;
				break;
			}
			iteradorListaActual.Avanzar();
		}

		if (loEncontre)
		{
			//Ya existia la palabra,entonces incremento la cantidad
			iteradorListaActual.Siguiente().second++;
		}
		else
		{
			// no existe el string, lo agrego con cantidad 1
			tabla[posicion]->push_front(pair<string, unsigned int>(key,1));
		}
		//Desbloqueo el mutex de esa lista.
		pthread_mutex_unlock(&mutexes[posicion]);
	}

	bool member(string key) {
		bool loEncontre=false;
		// obtenemos la posicion del array correspondiente a la primer letra del string
		unsigned int posicion= mapCharToInt(key.at(0));
		//No bloqueamos nada porque deber ser wait-free

		// obtenemos un iterador a la lista que tenemos que buscar dentro del hash
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[posicion]->CrearIt();
		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente, veamos si es la palabra key
			if (iteradorListaActual.Siguiente().first.compare(key)==0)
			{
				//Es la palabra.Salgo del ciclo
				loEncontre=true;
				break;
			}
			iteradorListaActual.Avanzar();
		}
		return loEncontre;
	}

	//Esta funcion convierte un char ascii a entero.
	unsigned int mapCharToInt(char a){
		return (unsigned int)a-97;
	}
	
	// Busca el máximo de la fila indicada en el parámetro ind y lo devuelve en un par
	pair<string, unsigned int> maximumFila(unsigned int ind)
    {
		pair<string,unsigned int> maximo;
		
		// Inicializo el máximo en 0 (las palabras que estén como mínimo tienen 1 repetición)
		maximo.second = 0;
		// Bloqueo el mutex de la lista que voy a revisar. Aquí hago exclusión mutua con addAndInc
		pthread_mutex_lock(&mutexes[ind]);
		// obtenemos un iterador a la lista que tenemos que revisar
		Lista<pair<string, unsigned int>>::Iterador iteradorListaActual = tabla[ind]->CrearIt();

		while(iteradorListaActual.HaySiguiente()){
			// Hay siguiente
			if (iteradorListaActual.Siguiente().second > maximo.second)
			{
				// Encontré uno mayor
				maximo = iteradorListaActual.Siguiente();
			}
			iteradorListaActual.Avanzar();
		}
		
		pthread_mutex_unlock(&mutexes[ind]);		
    	return maximo;
    }
    
    static void *maximumThread(void* data)
    {
    	// Casteo lo que me pasaron al tipo (maximum_struct*)
    	pair<string, unsigned int>* result = new pair<string, unsigned int>();
    	result->second=0;
    	maximum_struct* estructura = (maximum_struct*) data;
    	int filaAProcesar = estructura->filasDisponibles--;
    	while(filaAProcesar>0)
    	{
    		pair<string, unsigned int> resultInterno = estructura->clase->maximumFila(filaAProcesar-1);
    		if (result->second<resultInterno.second)
    		{ 
    			*result=resultInterno;
     		}
    		filaAProcesar = estructura->filasDisponibles--;
    	}
    	
    	return (void*)result;
    }
	
    
    // Recibe como parámetro un arreglo de tamaño 26 de bools, los cuales indican si
    // la lista correspondiente a la posición i-ésima del arreglo ya fué procesada por algún thread
    // en la función maximumThread
    // Devuelve el índide de la primer fila que esté sin procesar (el primer bool en false) y la marca como procesada (true)
    unsigned int proximaFilaDisponible(bool* array){
	for (unsigned int i = 0; i < 26; i++)
	{
		if (array[i] == false)
		{
			array[i] = true;
			return i;
		}			
	}
	    
	// Nunca se deberia ejecutar.
	return 0;
    }

	pair<string, unsigned int> maximum(unsigned int nt){
		// Creo nt threads
		pthread_t thread[nt];
		// Creo una estructura que será pasada a cada thread
		maximum_struct estructura;
		estructura.filasDisponibles = 26;
		estructura.clase = this;
		
		pair<string, unsigned int> maximoDeLosMaximos;
		maximoDeLosMaximos.second=0;

		//Ejecuto los nt threads
		for (int i = 0; i < nt; ++i)
		{
			pthread_create(&thread[i], NULL, &(ConcurrentHashMap::maximumThread), &estructura);
		}
		
		// Espero que los nt threads terminen
		for (int i = 0; i < nt; ++i)
		{
			void* result;
			pthread_join(thread[i], &result);
			
			if (maximoDeLosMaximos.second < ((pair<string,unsigned int>*)result)->second)
			{
				
				maximoDeLosMaximos= *((pair<string,unsigned int>*)result);
			}
			//lo borro
			delete (pair<string,unsigned int>*) result;
		}
		
		return maximoDeLosMaximos;
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
		
    	estr.c->thread_process_file(estr.filePath, estr.c);

	}

	static ConcurrentHashMap count_words(unsigned int n, list<string> l){

	    ConcurrentHashMap c;
	    // Un thread por archivo
		pthread_t threads[n];
		count_words_n_threads_struct estructura;
		std::vector<string> v{ std::begin(l), std::end(l) };
		estructura.filePaths=v;
		
		estructura.archivosDisponibles = v.size();
		estructura.clase = &c;
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

    	// obtenemos el indice de forma atomica
    	int ind = estructura->archivosDisponibles--;
    	while(ind>0)
    	{
    		string archivoAProcesar = (estructura->filePaths)[ind-1];
    		
    		estructura->clase->thread_process_file(archivoAProcesar, estructura->clase);

			ind = estructura->archivosDisponibles--;
    	}
    	
		
    	
    }


    static void* thread_process_file_new_hashmap(void* estruc)
    {
    	datos_multiple_hashmap* estructura = (datos_multiple_hashmap*)estruc;

    	string archivoAProcesar;
    	
    	pthread_mutex_lock(&(estructura->mutex));

    	while(estructura->archivosAProcesar.size()>0){
    		
    	 	archivoAProcesar = estructura->archivosAProcesar.front();
    		estructura->archivosAProcesar.pop_front();
    		pthread_mutex_unlock(&(estructura->mutex));
    		ConcurrentHashMap c= ConcurrentHashMap::count_words(archivoAProcesar);
    		pthread_mutex_lock(&(estructura->mutex));
    		estructura->hashMapsProcesados.push_back(c);

    	}
    	pthread_mutex_unlock(&(estructura->mutex));
    }



    static pair<string, unsigned int> maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs)
    {
    	// Creamos p_archivos threads
		pthread_t threads_archivos[p_archivos];

		struct datos_multiple_hashmap estructura_datos;

		// Inicializamos la lista de info de archivos
		estructura_datos.archivosAProcesar = archs;

		//Inicializo el mutex que sera utilizado para asignar filas a los threads
		pthread_mutex_init(&estructura_datos.mutex, NULL);

		for (int i = 0; i < p_archivos; i++)
		{
			pthread_create(&threads_archivos[i], NULL, &(ConcurrentHashMap::thread_process_file_new_hashmap), &estructura_datos);
		}

		// Espero a que se creen todos los ConcurrentHashMap
		for (int i = 0; i < p_archivos; ++i)
		{
			pthread_join((threads_archivos[i]),NULL);
		}

		for (int i = 1; i < estructura_datos.hashMapsProcesados.size(); i++)
		{
			estructura_datos.hashMapsProcesados[0].merge(&(estructura_datos.hashMapsProcesados[i]));
		}
		//Calculo el maximo del hashmap final.
		return estructura_datos.hashMapsProcesados[0].maximum(p_maximos);

    }

static pair<string, unsigned int> maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string> archs)
    {
    	ConcurrentHashMap c=count_words(p_archivos,archs);
    	return c.maximum(p_maximos);
    	
    }
	void merge (ConcurrentHashMap* source)
	{
		for (int i = 0; i < 26; i++)
		{
			// obtenemos un iterador a la lista correspondiente a la letra actual de source
			Lista<pair<string, unsigned int>>::Iterador iteradorListaActual = source->tabla[i]->CrearIt();
			
			// Para cada palabra de la lista actual, la agrego a this
			while(iteradorListaActual.HaySiguiente())
			{
				// Agrego la palabra actual a this tantas veces como aparezca en source 
				for (int j = 0; j < iteradorListaActual.Siguiente().second; j++)
				{
					this->addAndInc(iteradorListaActual.Siguiente().first);
				}

				iteradorListaActual.Avanzar();
			}


		}
	

	}



};



#endif 

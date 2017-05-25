#ifndef CONCURRENT_HASHMAP_H__
#define CONCURRENT_HASHMAP_H__

#include <atomic>
#include "ListaAtomica.hpp"
#include <utility>      // std::pair, std::make_pair
#include <string>       // std::string
#include <list>
#include <fstream>
#include <vector>

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
	
	//Busca el maximo de la fila ind y lo guarda en el puntero a arreglo de par pasado por parametro
	void *maximumFila(unsigned int ind, pair<string,unsigned int>* arreglo)
    {
		pair<string,unsigned int> maximo;
		//Inicializo el maximo en 0 (Las palabras minimo tienen 1 repetición)
		maximo.second=0;
		//Lockeo el mutex de cada lista. Aqui hago exclusion mutua con addAndInc
		pthread_mutex_lock(&mutexes[ind]);
		// obtenemos un iterador a la lista que tenemos que consultar dentro del hash
		Lista<pair<string,unsigned int>>::Iterador iteradorListaActual= tabla[ind]->CrearIt();

		while(iteradorListaActual.HaySiguiente()){
			//Hay siguiente
			if (iteradorListaActual.Siguiente().second>maximo.second)
			{
				//Encontre uno mayor	
				maximo=iteradorListaActual.Siguiente();
			}
			iteradorListaActual.Avanzar();
		}
		arreglo[ind]=maximo;
		pthread_mutex_unlock(&mutexes[ind]);
		
    }

    static void *maximumThread(void* estruc)
    {
    	//Casteo a la estructura que me pasaron
    	maximumStruct* estructura = (maximumStruct*)estruc;

    	unsigned int filaAProcesar;
    	//Bloqueo el mutex que tienen todos los threads
    	//Asi me fijo cual fila esta disponible para procesar
    	//Y ademas modifico la estructura poniendo como procesada la fila que me ponga a procesar
    	pthread_mutex_lock(&(estructura->mutex));
    	while(estructura->filasDisponibles>0){
    		//Alguna fila queda disponible
    		estructura->filasDisponibles--;
    		//Busco cual es la fila disponible
    		filaAProcesar = estructura->clase->proximaFilaDisponible((estructura->procesadas));
    		//Desbloqueo el mutex porque ya tengo la fila que tengo que procesar.
    		pthread_mutex_unlock(&(estructura->mutex));
    		
    		//Busco el maximo de la fila. La funcion maximumFila verifica 
    		//no estar ejecutandose a la par de addAndInc
    		estructura->clase->maximumFila(filaAProcesar,(estructura->_maximos));
    		//Bloqueo el mutex para consultar si hay alguna fila disponible (El while)
			pthread_mutex_lock(&(estructura->mutex));
    	}
    	//Desbloqueo. No hay mas filas disponibles
    	pthread_mutex_unlock(&(estructura->mutex));		
    	
    }
    //Devuelve la primer fila que este sin procesar y la marca como procesada
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
		//Creo nt threads
		pthread_t thread[nt];
		//Creo una estructura que sera pasada a cada thread
		maximumStruct estructura;
		estructura.filasDisponibles=26;
		estructura.clase = this;
		//Inicializo el mutex que sera utilizado para asignar filas a los threads
		pthread_mutex_init(&estructura.mutex, NULL);
		
		//Inicializo un array de bools de filas procesadas
		for (int i = 0; i < 26; i++)
		{

			estructura.procesadas[i]=false;	
		}
		
		//Ejecuto los nt threads
		for (int i = 0; i < nt; ++i)
		{
			pthread_create(&thread[i],NULL,&(ConcurrentHashMap::maximumThread),&estructura);
		}
		
		//Espero que los nt threads terminen
		for (int i = 0; i < nt; ++i)
		{
			pthread_join((thread[i]),NULL);
		}
		
		//Comparo los resultados _maximos
		pair<string,unsigned int> maximoDeLosMaximos=estructura._maximos[0];
		for (int i = 1; i < 26; i++)
		{
			if (estructura._maximos[i].second>maximoDeLosMaximos.second)
			{
				maximoDeLosMaximos=estructura._maximos[i];
			}
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

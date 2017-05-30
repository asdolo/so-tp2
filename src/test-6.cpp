#include <iostream>
#include <cstdlib>
#include "ConcurrentHashMap.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
using namespace std;


#define BILLION  1000000000L;

int main(int argc, char **argv) {

	pair<string, unsigned int> p;
	list<string> l = { "corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4","superArchivo","superArchivo","superArchivo" };

	if (argc != 3) {
		cerr << "uso: " << argv[0] << " #tarchivos #tmaximum" << endl;
		return 1;
	}
  	 struct timespec start, stop;
    unsigned long accum;

    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

	p = ConcurrentHashMap::maximum(atoi(argv[1]), atoi(argv[2]), l);
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

     accum =  ((stop.tv_sec - start.tv_sec)*1000000000L) +( stop.tv_nsec - start.tv_nsec ) ;
    
    struct timespec start2, stop2;
    unsigned long accum2;

    if( clock_gettime( CLOCK_REALTIME, &start2) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

  p = ConcurrentHashMap::maximum2(atoi(argv[1]), atoi(argv[2]), l);
    if( clock_gettime( CLOCK_REALTIME, &stop2) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    accum2 =  ((stop2.tv_sec - start2.tv_sec)*1000000000L) +( stop2.tv_nsec - start2.tv_nsec ) ;
            ;
    printf( "Cantidad de archivos:%d\nCantidad de threads:%d\nTiempo con maximum :%ld ns\nTiempo con maximum2 :%ld ns\nDiferencia de tiempos :%ld ns\n\n",atoi(argv[1]), atoi(argv[2]),( long)accum, ( long) accum2, ( long)(accum-accum2 ));

	
	
	return 0;
}


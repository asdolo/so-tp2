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
	list<string> l = { "corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4" };

	if (argc != 3) {
		cerr << "uso: " << argv[0] << " #tarchivos #tmaximum" << endl;
		return 1;
	}
	 struct timespec start, stop;
    double accum;

    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

	p = ConcurrentHashMap::maximum2(atoi(argv[1]), atoi(argv[2]), l);
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    accum = ( stop.tv_sec - start.tv_sec )
          + ( stop.tv_nsec - start.tv_nsec )
            ;
    printf( "Cantidad de archivos:%d\nCantidad de threads:%d\nTiempo:%lf\n\n",atoi(argv[1]), atoi(argv[2]),accum );
	
	
	return 0;
}


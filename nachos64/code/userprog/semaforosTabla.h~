#ifndef SEMT_H
#define SEMT_H

#define MAX_SEMAFOROS 128
#include "bitmap.h"
#include "../threads/synch.h"

class SemTable {

  public:
    SemTable();
    ~SemTable();
    int getUnixHandle( int NachosSem );
    void Print();               // Print contents
    int CreateSem(Semaphore* indicador );
    int Close( int indicador );
    void SemSignal(int id);
    void SemWait(int id);
  private:
    Semaphore** semCreados;		// A vector with user opened files
    BitMap * semMap;		// A bitmap to control our vector
    int usage;			// How many threads are using this table
    
};

#endif



#include "semaforosTabla.h"
#include <unistd.h>
#include "system.h"
#include "../threads/synch.h"



SemTable::SemTable(){
	semCreados = new Semaphore*[MAX_SEMAFOROS];
	usage = 0;
	semMap = new BitMap(MAX_SEMAFOROS);
}

SemTable::~SemTable(){
	delete[] semCreados;
	delete semMap;
}
 void SemTable::SemSignal(int id){		 
	semCreados[id]->V();
	
 }
void SemTable::SemWait(int id){
	semCreados[id]->P();
}
int SemTable::CreateSem(const char* buffer, int indicador ){
	
	int clearPosition = semMap->Find();
	Semaphore* s = new Semaphore(buffer, indicador); 
	semCreados[clearPosition] = s;
	return clearPosition;
}

int SemTable::Close( int indicador ){

	Semaphore* s;
	s = semCreados[indicador];
	delete s;
	semCreados = NULL;
	semMap->Clear(indicador);
	return 0;
}



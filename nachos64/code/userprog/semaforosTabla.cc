

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

int SemTable::CreateSem(Semaphore* indicador ){
	
	int clearPosition = semMap->Find();
	semCreados[clearPosition] = indicador;
	return clearPosition;
}

int SemTable::Close( int indicador ){

	Semaphore* s;
	s = semCreados[indicador];
	delete s;
	semMap->Clear(indicador);
	return 0;
}



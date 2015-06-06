#include "threadsTabla.h"
typedef int SpaceId;
threadsTabla::threadsTabla(){
	threadsOn = new Thread*[MAX_THREADS];
	threadsOnMap = new BitMap(MAX_THREADS);
	hilosJoin = new hilosEnEspera[MAX_THREADS] ;
	BitMap * hilosMapa = new BitMap(MAX_THREADS);
}

threadsTabla::~threadsTabla(){
	delete threadsOn;
	delete threadsOnMap;
	delete hilosJoin;
	delete hilosMapa;
}
/*
int threadsTabla::addJoin(Thread* thread, Semaphore* s,int identificador, SpaceId id){
	//int identificador = hilosMapa->Find();
	hilosJoin[identificador].hilo = thread;
	hilosJoin[identificador].sem = s;
	hilosJoin[identificador].hiloEsperado = id;
	//hilosMapa->Mark(identificador);
	return identificador;
}
*/
/*
int threadsTabla::delJoin(Thread* thread, Semaphore* s,int identificador, SpaceId id){
	hilosJoin[identificador].hilo = NULL;
	hilosJoin[identificador].sem = NULL;
	hilosJoin[identificador].hiloEsperado = -1;
	//hilosMapa->Clear(identificador);
	return identificador;

}*/
int threadsTabla::addJoin(Thread* thread, Semaphore* s, SpaceId id){
	int identificador = hilosMapa->Find();
	hilosJoin[identificador].hilo = thread;
	hilosJoin[identificador].sem = s;
	hilosJoin[identificador].hiloEsperado = id;
	hilosMapa->Mark(identificador);
	return identificador;
}

int threadsTabla::delJoin(Thread* thread, Semaphore* s,int identificador, SpaceId id){
	hilosJoin[identificador].hilo = NULL;
	hilosJoin[identificador].sem = NULL;
	hilosJoin[identificador].hiloEsperado = -1;
	hilosMapa->Clear(identificador);
	return identificador;

}

bool threadsTabla::UniqueSpaceUsing(AddrSpace *space,SpaceId ID){
	int i = 0;
	bool Unico = true;
	
	//Si otro thread tiene la misma pagina fisica quiere decir que no es el 
  	//unico con la misma tabla de archivos y demas
	TranslationEntry* pageTable =  space->getPageTable();
	int physicalPage = pageTable[0].physicalPage;
	int physicalPageCurrent;
	while(i < MAX_THREADS && Unico){
		if(threadsOnMap->Test(i)){
		   	TranslationEntry* pageTable =  threadsOn[i]->space->getPageTable();
			 physicalPageCurrent = pageTable[0].physicalPage;
			if(physicalPageCurrent == physicalPage && i != ID){
				Unico = false;
			}		
			
		}
		i++;
	}
	return Unico;
}


SpaceId threadsTabla::AddThread(Thread* thread){
	int clearPosition = threadsOnMap->Find();
	threadsOn[clearPosition] = thread;
	return clearPosition;
}

int threadsTabla::DelThread(int NachosHandle){
	 threadsOnMap->Clear(NachosHandle);
	return 0;
}



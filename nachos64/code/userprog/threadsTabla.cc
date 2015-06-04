#include "threadsTabla.h"

threadsTabla::threadsTabla(){
	threadsOn = new Thread*[MAX_THREADS];
	threadsOnMap = new Bitmap(MAX_THREADS);
}

threadsTabla::~threadsTabla(){
	delete threadsOn;
	delete threadsOnMap;
}

SpaceId threadsTabla::AddThread(Thread* thread){
	int clearPosition = threadsOnMap->Find();
	threadsOn[clearPosition] = thread;
	return clearPosition;
}

int threadsTabla::DelThread(int NachosHandle){
	threadsOn[NachosHandle];
	 threadsOnMap->Clear(NachosHandle);
	return 0;
}


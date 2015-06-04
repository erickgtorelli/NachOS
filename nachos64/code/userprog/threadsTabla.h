#ifndef THREADSTABLA_H
#define THREADSTABLA_H

#define MAX_THREADS 128
#include "bitmap.h"
#include "../threads/thread.h"
class threadsTabla{
	typedef SpaceId int;
	
	public: 
	threadsTabla();
	~threadsTabla();
	
	SpaceId AddThread(int* Thread);
	int DelThread(int NachosHandle);
 	inline Thread* getThread(int NachosHandle){return threadsOn[NachosHandle]};
 	
 	private:
 	Thread** threadsOn;		// A vector with user operating threads
    	BitMap * threadsOnMap;	// A bitmap to control our vector

};



#endif //THREADSTABLA

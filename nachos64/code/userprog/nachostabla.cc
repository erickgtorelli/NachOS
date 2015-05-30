#include "nachostabla.h"
#include <unistd.h>

NachosOpenFilesTable::NachosOpenFilesTable(){
	openFiles = new int[MAX_FILES];
	usage = 0;
	openFilesMap = new BitMap(MAX_FILES);
	
}

NachosOpenFilesTable::~NachosOpenFilesTable(){
	delete openFiles;
	delete openFilesMap;
}

int NachosOpenFilesTable::Open( int UnixHandle ){
	int clearPosition = openFilesMap->Find();
	openFiles[clearPosition] = UnixHandle;
	return clearPosition;
}

 int NachosOpenFilesTable::Close( int NachosHandle ){
	openFiles[NachosHandle];
	 openFilesMap->Clear(NachosHandle);
	return openFiles[NachosHandle];;
}

bool NachosOpenFilesTable::isOpened( int NachosHandle ){
	if(openFilesMap->Test(NachosHandle)){
		return true;
	}
	return false;
}

int NachosOpenFilesTable::getUnixHandle( int NachosHandle ){
	return openFiles[NachosHandle];
}

void NachosOpenFilesTable::addThread(){
	usage++;
}

void NachosOpenFilesTable::delThread(){
	usage--;
	if(usage == 0){
		for(int i=0;i<MAX_FILES;i++){
			if(isOpened(i)){
				close(getUnixHandle(i));
				this->Close(i);					
								
			}			
		}	
	}
}

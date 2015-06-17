#include "system.h"
#include "syscall.h"
#include "swap.h"
#include <sstream>
#include "machine.h"
Swap::Swap(){
  int pageNum = 64;
  std::stringstream nameStream;
  nameStream << "Swap." << sid;
  swapFileName = nameStream.str();
  fileSystem->Create(swapFileName.c_str(), pageNum * PageSize);
  swapFile = fileSystem->Open(swapFileName.c_str());
        Addr = space;
}

bool Swap::swapIn(int page, int frame){


  int charsRead;
  char* dest = machine->mainMemory + frame * PageSize;

  charsRead = swapFile->ReadAt(dest, PageSize, page * PageSize);

  bool res = (charsRead == PageSize);
  
  if (res){
    //coremap->usedFrames[frame] = true;
    //coremap->owner[frame] = id;
    machine->tlb[page].valid = true;
    machine->tlb[page].dirty = false;
  }

  return res;
}

bool Swap::swapOut(int frame,int NumPages){
        
                int page = -1;
        for(int i = 0;i<NumPages;i++){
                if(machine->tlb[i].physicalPage == frame)
                 page = i;
                }
  if (page == -1){
    ASSERT(false);
    return false;
  }
  
    int charsWritten;
    char* source = machine->mainMemory + frame * 128;

    charsWritten = swapFile->WriteAt(source, 128, page * 128);
    ASSERT(charsWritten == 128);
  

  machine->tlb[page].valid = false;
  machine->tlb[page].dirty = false;
  machine->tlb[page].physicalPage = -1;

  //coremap->usedFrames[frame] = true;


  return true;
}



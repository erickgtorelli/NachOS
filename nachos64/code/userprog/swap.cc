#include "system.h"
#include "syscall.h"
#include "swap.h"
#include <sstream>
#include "machine.h"
Swap::Swap(){
  int pageNum = 64;
  std::stringstream nameStream;
  nameStream << "Swap";
  swapFileName = nameStream.str();
  fileSystem->Create(swapFileName.c_str(), pageNum * PageSize);
  swapFile = fileSystem->Open(swapFileName.c_str());
}
Swap::~Swap(){
}
bool Swap::swapIn(int page, int frame){


  int charsRead;
  char* dest = machine->mainMemory + frame * PageSize;

  charsRead = swapFile->ReadAt(dest, PageSize, page * PageSize);

  bool res = (charsRead == PageSize);
  
  if (res){
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


  return true;
}



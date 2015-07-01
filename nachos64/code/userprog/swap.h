#ifndef SWAP_H
#define SWAP_H

#include "openfile.h"
#include <vector>
#include <string>


typedef int SpaceId;

class Swap{

 public:

  Swap();
  ~Swap();
  ///
  //bool isSwaped(int page){
    //return MapSwap->Test(page);
  //}

  bool swapIn(int page, int frame);
  bool swapOut(int frame,int NumPages);

 private:
  
  OpenFile* swapFile;
  std::string swapFileName;
  
};

#endif

// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <unistd.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

#include "translate.h"
#include <fcntl.h>
#include "threadsTabla.h"
#include "semaforosTabla.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'lltranslation 

// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
threadsTabla* threadsActivos = new threadsTabla();
SemTable* Semaforos = new SemTable();
int FIFOSwap = 0;
int FIFOTLB = 0;
int FIFOMainMem=0;
void returnFromSystemCall() {

        int pc, npc;

        pc = machine->ReadRegister( PCReg );
        npc = machine->ReadRegister( NextPCReg );
        machine->WriteRegister( PrevPCReg, pc );        // PrevPC <- PC
        machine->WriteRegister( PCReg, npc );           // PC <- NextPC
        machine->WriteRegister( NextPCReg, npc + 4 );   // NextPC <- NextPC + 4

}       // returnFromSystemCall


//Función que convierte de la memoria virtual de NachOS
//a la memoria fisica del sistema

char * ReadFromNachosMemory(int virtualmemory){
	char* string = new char[100];
	
	int fin = '\0';
	int value = 0;
	int posicion = 0;
	//Read Name File
	do{
		machine->ReadMem(virtualmemory+posicion,1,&value);
		*(string + posicion) = value;
		posicion++;
	}while(value != fin);

	*(string + posicion) ='\0';
	
	return string;
}
//Detiene el sistema de NachOS
void Nachos_Halt() {                    // System call 0

        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();

}       // Nachos_Halt
//Crea un archivo con el nombre del archivo proveniente
//del registro 4
void Nachos_Create() {                    // System call 4
/* System call definition described to user
	int Create(
		char *name	// Register 4
	);
*/
	// Read the name from the user memory, see 4 below
	// Use NachosOpenFilesTable class to create a relationship
	// between user file and unix file
	// Verify for errors
	int nameNachos = machine->ReadRegister(4);
	char* buffer  = ReadFromNachosMemory(nameNachos);
	int UnixHandel = creat((const char*)buffer, 0777);
	if(UnixHandel != -1){
		int NachosHandle = currentThread->openedFiles->Open(UnixHandel);
		machine->WriteRegister( 2, NachosHandle + 3); 
		printf("Se creó el archivo\n");
	}
	else{
		perror("Error al crear el archivo\n");	
		exit(1);
	}

	 returnFromSystemCall();


}       // Nachos_Create
//Abre el archivo especificado con el nombre 
//proveniente del registro 4   

void Nachos_Open() {                    // System call 5
/* System call definition described to user
	int Open(
		char *name	// Register 4
	);
*/
	// Read the name from the user memory, see 4 below
	// Use NachosOpenFilesTable class to create a relationship
	// between user file and unix file
	// Verify for errors

	int nameNachos = machine->ReadRegister(4);
	
	
	char* buffer  = ReadFromNachosMemory(nameNachos);
	int UnixHandel = open((const char *)buffer,O_RDWR);
	if(UnixHandel != -1){
		int NachosHandle = currentThread->openedFiles->Open(UnixHandel);
		machine->WriteRegister( 2, NachosHandle + 3); 
		DEBUG('j',"Se abrio el archivo\n");
	}
	else{
		perror("Error al abrir el archivo\n");	
		exit(1);
	}

	 returnFromSystemCall();


}       // Nachos_Open
//Escribe en el archivo con el id especificado en el registro 4
void Nachos_Write() {                   // System call 7
/*
 //System call definition described to user
        void Write(
		char *buffer,	// Register 4
		int size,	// Register 5
		 OpenFileId id	// Register 6
	);

*/

        char* buffer=NULL;
        int size = machine->ReadRegister( 5 );	// Read size to write

	int error=-1;

	int Unix;
	int nameNachos;
	int fin = '\0';
	int value = 0;
	int posicion=0;
	ssize_t bytes;
	buffer=new char[size];
	
	nameNachos = machine->ReadRegister(4);
        // buffer = Read data from address given by user;
        OpenFileId id = machine->ReadRegister( 6 );	// Read file descriptor

	// Need a semaphore to synchronize access to console
	// Console->P();
	switch (id) {
		
		case  ConsoleInput:	// User could not write to standard input
			machine->WriteRegister( 2, -1 );
			break;
		case  ConsoleOutput:
				do{
					machine->ReadMem(nameNachos+posicion,1,&value);
					buffer[posicion] = value;
					++posicion;		
				}while(value != fin);
				buffer[size]=fin;
			        bytes = write(1,(const void*)buffer,size);
				machine->WriteRegister(2,bytes);
			break;
		case ConsoleError:	// This trick permits to write integers to console
			printf( "%d\n", machine->ReadRegister( 4 ) );
		 	break;
		default:
			if(currentThread->openedFiles->isOpened(id-3)){
				Unix = currentThread->openedFiles->getUnixHandle(id-3);
			        fin = '\0';
			        value = 0;
			        posicion=0;
				do{
					machine->ReadMem(nameNachos+posicion,1,&value);
					buffer[posicion] = value;
					++posicion;		
				}while(value != fin);
				buffer[size] = fin;
				bytes = write(Unix,(const void*)buffer,size);
				machine->WriteRegister(2,bytes);
			}else{
				
				machine->WriteRegister(2,error);
			}

			
			// All other opened files
			// Verify if the file is opened, if not return -1 in r2
			// Get the unix handle from our table for open files
			// Do the write to the already opened Unix file
			// Return the number of chars written to user, via r2

			break;	

	}
	// Update simulation stats, see details in Statistics class in machine/stats.cc
	// Console->V();
        returnFromSystemCall();		// Update the PC registers

}       // Nachos_Write
//Lee del archivo o consola la cantidad de bytes especificados en el registro 5
void Nachos_Read(){
	char* buffer=NULL;
        int size = machine->ReadRegister( 5 );	// Read size to read

	int error=-1;

	int Unix;
	int nameNachos;
	int fin = '\0';
	int value = 0;
	int posicion=0;
	ssize_t bytes;
	buffer = new char[size];
	nameNachos = machine->ReadRegister(4);
        // buffer = Read data from address given by user;
        OpenFileId id = machine->ReadRegister( 6 );	// Read file descriptor
	int puntero;
	char* ptr;
	// Need a semaphore to synchronize access to console
	// Console->P();
	switch (id) {
			
		case  ConsoleInput:	// User could not write to standard input
			//puntero = machine->ReadRegister(2);
			printf("Introduce texto a almacenar\n");
			bytes = read(0,(void*)buffer,size);
		        fin = '\0';
		        value = 0;
		        posicion=0;
			ptr = buffer;
			do{
				machine->WriteMem(nameNachos+posicion,1,*ptr);
				++posicion;
				++ptr;
			} while(*ptr !=fin);
			machine->WriteMem(nameNachos+posicion,1,'\0');				
			machine->WriteRegister(2,bytes);
			break;
		case  ConsoleOutput:
			machine->WriteRegister( 2, -1 );	
			break;
		case ConsoleError:	// This trick permits to write integers to console
			printf( "%d\n", machine->ReadRegister( 4 ) );
		 	break;
		default:
			
			if(currentThread->openedFiles->isOpened(id-3)){
				Unix = currentThread->openedFiles->getUnixHandle(id-3);
				bytes = read(Unix,(void*)buffer,size);
			        fin = '\0';
			        value = 0;
			        posicion=0;
				ptr = buffer;
				do{
					machine->WriteMem(nameNachos+posicion,1,*ptr);
					++posicion;
					++ptr;
				} while(*ptr !=fin);
				
				machine->WriteMem(nameNachos+posicion,1,'\0');				
				machine->WriteRegister(2,bytes);
			}else{
				
				machine->WriteRegister(2,error);
			}

			
			// All other opened files
			// Verify if the file is opened, if not return -1 in r2
			// Get the unix handle from our table for open files
			// Do the write to the already opened Unix file
			// Return the number of chars written to user, via r2

			break;	

	}
	// Update simulation stats, see details in Statistics class in machine/stats.cc
	// Console->V();

        returnFromSystemCall();		// Update the PC registers
	
}
//Termina el proceso, y libera la memoria ulizada por este en caso de que otro
//proceso no la este utilizando
void Nachos_Exit(){
	int SpaceId = machine->ReadRegister(4);
	AddrSpace *space = currentThread->space;
	
	if(threadsActivos->UniqueSpaceUsing(space,SpaceId)){
		currentThread->Finish();	
	}
	//Solo se elimina el stack si otros threads estan usando la demas memoria
	else{
		TranslationEntry* pageTable =  space->getPageTable();
		for(unsigned int i = (space->getNumPages()) - 8; i<(space->getNumPages());i++){
			bzero((void*)&machine->mainMemory[128 * pageTable[i].physicalPage], 128);
			MiMapa->Clear(i);
		}
		currentThread->setStatus(BLOCKED);
	}
	threadsActivos->avisarHilo(SpaceId);
	
        returnFromSystemCall();		
}

void Nachos_Close(){
	
	int NachosHandle = machine->ReadRegister(4);
	DEBUG('j',"Cerrando el archivo %d \n",NachosHandle);
	if(currentThread->openedFiles->isOpened(NachosHandle)){
		int UnixHandle = currentThread->openedFiles->Close(NachosHandle-3);
		machine->WriteRegister(2,close(UnixHandle));
	}

	 returnFromSystemCall();
}

void Nachos_SemCreate(){
	
	int nameNachos = machine->ReadRegister(4);
	int numRecursos = machine->ReadRegister(5);
	char* buffer = ReadFromNachosMemory(nameNachos);
   	int ret = Semaforos->CreateSem((const char*)buffer, numRecursos);
   	machine->WriteRegister(2,ret);
	returnFromSystemCall();
}

void Nachos_SemDestroy(){
	
	int id = machine->ReadRegister(4);
	Semaforos->Close(id);
	returnFromSystemCall();
}

void NachosForkThread( void * p ) { // for 64 bits version
 	   DEBUG('j', "NachosForkThread");
    AddrSpace *space;

    space = currentThread->space;
    space->InitRegisters();             // set the initial register values
    space->RestoreState();              // load page table register

// Set the return address for this thread to the same as the main thread
// This will lead this thread to call the exit system call and finish
    machine->WriteRegister(RetAddrReg, 4 );
   int x = *((int*)(&p));
    machine->WriteRegister( PCReg, (long) p );
    machine->WriteRegister( NextPCReg, x + 4 );

    machine->Run();                     // jump to the user progam
    ASSERT(false);

}


void Nachos_Fork() {			// System call 9

	DEBUG( 'u', "Entering Fork System call\n" );
	// We need to create a new kernel thread to execute the user thread
	Thread * newT = new Thread( "child to execute Fork code" );

	// We need to share the Open File Table structure with this new child
	newT->openedFiles = currentThread->openedFiles;
	//currentThread->SpaceId = threadsActivos->AddThread(currentThread);
	newT->SpaceId = threadsActivos->AddThread(newT);
	// Child and father will also share the same address space, except for the stack
	// Text, init data and uninit data are shared, a new stack area must be created
	// for the new child
	// We suggest the use of a new constructor in AddrSpace class,
	// This new constructor will copy the shared segments (space variable) from currentThread, passed
	// as a parameter, and create a new stack for the new child
	newT->space = new AddrSpace( currentThread->space );
            DEBUG('j', "Constructor Terminado\n");
					
	// We (kernel)-Fork to a new method to execute the child code
	// Pass the user routine address, now in register 4, as a parameter
	// Note: in 64 bits register 4 need to be casted to (void *)
	DEBUG('j', "Registro 4  %d \n ",machine->ReadRegister( 4 ));
	newT->Fork( NachosForkThread,(void*) machine->ReadRegister( 4 ) );
	currentThread->Yield();
	returnFromSystemCall();	// This adjust the PrevPC, PC, and NextPC registers

	DEBUG( 'u', "Exiting Fork System call\n" );
}	

void Nachos_Yield(){
	currentThread->Yield();
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
}

void startProcess(const char *filename)
{
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
	}
	space = new AddrSpace(executable);
	currentThread->space = space;

	delete executable;			// close file

	space->InitRegisters();		// set the initial register values
	space->RestoreState();		// load page table register

	machine->Run();			// jump to the user progam
	ASSERT(false);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

void Nachos_Join(){

	
	//	otra implementacion usando los id de los hilos para guardar
	/*
	int identificador = currentThread->SpaceId;
	Semaphore* s = new Semaphore("Join",0);
	int id = machine->ReadRegister(4);
	identificador = threadsActivos->addJoin(currentThread,s,identificador,id);
	s->P();
	threadsActivos->delJoin(currentThread,s,identificador,id);
	machine->WriteRegister(2,identificador);
	*/
	
	//implementacion usando bitMap
	Semaphore* s = new Semaphore("Join",0);
	int id = machine->ReadRegister(4);
	int identificador = threadsActivos->addJoin(currentThread,s,id);
	s->P();
	threadsActivos->delJoin(currentThread,s,identificador,id);
	machine->WriteRegister(2,identificador);
	returnFromSystemCall();	

}

void Nachos_Exec(){
	Thread * newT = new Thread( "Exec Thread" );
	newT->SpaceId = threadsActivos->AddThread(newT);
	char* name = ReadFromNachosMemory(machine->ReadRegister(4));
	startProcess((const char*)name);
}

int verificarTLB(){
	
	return MapaTLB->Find();
}

	//Revisa si la Main memory esta llena, sino hace algoritmo de remplazo
int freeIndexFrame(){
	TranslationEntry* pageTable = currentThread->space->getPageTable();
	int frame;
	bool FIFO = false;
	//Revisa si la Main memory esta llena, sino hace algoritmo de remplazo
	if((frame = MapaMainMem->Find()) ==-1){
				if(FIFOMainMem>31){
					
					FIFOMainMem=0;
				}
				frame = FIFOMainMem;
				FIFO = true;
				FIFOMainMem++;
			
			}
			
	DEBUG('k',"Free Frame : %d \n",	(frame));
			if(!FIFO){
				return frame;
			}
			
	//Si se usa el algoritmo de remplazo se debe de verificar si estan en la TLB, si fuera el caso
	//invalidar la pagina		
	for (int i = 0; i < TLBSize; i++)
  	  if (machine->tlb[i].physicalPage == frame &&
		machine->tlb[i].valid){
		pageTable[machine->tlb[i].virtualPage]=machine->tlb[i];
     			 machine->tlb[i].valid = false;
      		break;
    			}
    		
    	
    //Invalidar en el page table
    if(pageTable[inversedPages[frame]].dirty == true){
		pageTable[inversedPages[frame]].valid = false;
		swap->swapOut(frame,inversedPages[frame]);
	}else{
    	bzero((void*)&machine->mainMemory[128 * frame], 128);
	}
    	return frame;
} 
//Revisa si hay campo en el tlb sino hace algoritmo de remplazo
int freeTLB(){
	int pagina;
	if( (pagina =  MapaTLB->Find()) == -1){
			if(FIFOTLB>3){
				FIFOTLB = 0;	
			}
			pagina = FIFOTLB;
				FIFOTLB++;
			}
			return pagina;
}
//return the frame on loaded memory
int loadFromFileToPageTable(const char* filename,int direccion){
	OpenFile *executable = fileSystem->Open(filename);
	//DEBUG('M',"filename: %s",filename);

	int numeroPagina = direccion /128;
		int direccionVirtual = numeroPagina*128  +   currentThread->space->encabezadoProceso.code.inFileAddr;
	TranslationEntry* pageTable = currentThread->space->getPageTable();
	
	int freeFrame = freeIndexFrame();
	
	//actualizar pageTable
	pageTable[numeroPagina].physicalPage = freeFrame;
    pageTable[numeroPagina].valid = true;
	if(direccion < currentThread->space->encabezadoProceso.initData.virtualAddr){ 
	pageTable[numeroPagina].readOnly = true;} 
	else{
	pageTable[numeroPagina].readOnly = false; 
	}
	
	
		
	//se carga directamente a la memoria
	executable->ReadAt(&(machine->mainMemory[128 * freeFrame]),
				   128, direccionVirtual);
	DEBUG('k',"En memoria esta %d \n",	(machine->mainMemory[128 * freeFrame]));		   
	return freeFrame;
	
	
}
void Nachos_PageFaultException(){
	
	int direccion = machine->ReadRegister(39);
	DEBUG('k',"Se genero una excepcion para la direccion: %d \n", machine->ReadRegister(39));
	int numeroPagina = direccion/128;
	//printf("numeroPagina %d \n",numeroPagina);
	TranslationEntry* paTable = currentThread->space->getPageTable();		
	int pagina, frame;
	int pila = ((currentThread->space->getNumPages()-8) * 128) - currentThread->space->encabezadoProceso.uninitData.size;	
	DEBUG('k',"Pila inicia %d\n", pila);
	int codigo = 0;
	DEBUG('k',"direccion %d\n", direccion);
	
	if(paTable[numeroPagina].valid == false){
		if(paTable[numeroPagina].dirty == true){
			DEBUG('k',"swap %d\n", numeroPagina);
			//LISTO
			 //Swap 
			 //printf("cargando del swap\n");
			frame = freeIndexFrame();
			inversedPages[frame] = numeroPagina;
			pagina = freeTLB();
			swap->swapIn(numeroPagina,frame);
			machine->tlb[pagina].virtualPage=numeroPagina;
			machine->tlb[pagina].valid = true;
			paTable[numeroPagina].physicalPage=frame;
			paTable[numeroPagina].valid=true;
			
		}else if(direccion >= pila){
			//LISTO
			 DEBUG('k',"Cargando Pila %d\n", numeroPagina);
			//No esta en el page table y es pila o unitData
			//printf("Pila que no esta en el page Table \n");
			frame = freeIndexFrame();
			inversedPages[frame] = numeroPagina;
			pagina = freeTLB();
			bzero((void*)&machine->mainMemory[128 * frame], 128);
			DEBUG('M',"En memoria esta %d \n",	(machine->mainMemory[128 * frame]));	
				paTable[numeroPagina].physicalPage=frame;
				paTable[numeroPagina].valid=true;
				paTable[numeroPagina].readOnly = false;
				machine->tlb[pagina].virtualPage = numeroPagina;
				machine->tlb[pagina].physicalPage=frame;
				machine->tlb[pagina].valid=true;
		}
		else if(direccion >= codigo && direccion  < pila){
			 DEBUG('k',"Cargando Pagina de codigo o datos inicializados %d\n", numeroPagina);
			 frame = loadFromFileToPageTable(currentThread->fileName,direccion);
			 //LISTO
			 //Se actualiza el tlb
			 pagina = freeTLB();
			 //printf);
			 //printf("frame: %d ,paginaTLB: %d,virualPage: %d\n",frame,pagina,numeroPagina);
			 machine->tlb[pagina].physicalPage = frame;
			 machine->tlb[pagina].valid = true;
			 machine->tlb[pagina].virtualPage = numeroPagina;
			 
		
		}
		
		}
		else{
			//LISTO
			//Esta en el PageTable
			//Se carga al TLB
			
				pagina = freeTLB();
				DEBUG('k', "Cargando al tlb del page table paginaTLB: %d,paginaVirtual: %d\n", 
					pagina,numeroPagina);
				//DEBUG('k',"virtualPage en pagetable: %d\n",paTable[numeroPagina].virtualPage);
				
				machine->tlb[pagina].virtualPage=numeroPagina;
				machine->tlb[pagina].physicalPage=paTable[numeroPagina].physicalPage;
				machine->tlb[pagina].valid=true;
				machine->tlb[pagina].readOnly=paTable[numeroPagina].readOnly;
				machine->tlb[pagina].use=paTable[numeroPagina].use;
				machine->tlb[pagina].dirty=paTable[numeroPagina].dirty;
				
				DEBUG('k', "Pagina fisica en: %d,paginaVirtual: %d\n", 
					paTable[numeroPagina].physicalPage,numeroPagina);
	}
		DEBUG('k', "///////////////////////////////////////////////////\n");
	}

	



void Nachos_SemSignal(){
	int id = machine->ReadRegister(4);
	Semaforos->SemSignal(id);
	returnFromSystemCall();
}

void Nachos_SemWait(){
	int id = machine->ReadRegister(4);
	Semaforos->SemWait(id);
	returnFromSystemCall();
}
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);	
    switch ( which ) {
       case SyscallException:
          switch ( type ) {
             case SC_Halt:
                Nachos_Halt();             // System call # 0
                break;
	     case SC_Create:
		Nachos_Create();
		break;
             case SC_Open:
                Nachos_Open();             // System call # 5
                break;
	     case SC_Join:
		Nachos_Join();
		break;
	     case SC_Read:     		   // System call # 6
		Nachos_Read();
		break;
             case SC_Write:
                Nachos_Write();             // System call # 7
                break;
	     case SC_SemCreate:		    // System call # 11
		Nachos_SemCreate();
		break;
	     case SC_Exit:
		Nachos_Exit();
		break;
	     case SC_Close:		//System call # 8
	     	Nachos_Close();
	     	break;
	     case SC_SemDestroy:
		Nachos_SemDestroy();
		break;
	     case SC_Fork:
		Nachos_Fork();
		break;
	     case SC_Yield:
		Nachos_Yield();
		break;
	     case SC_Exec:
		Nachos_Exec();
		break;
		case SC_SemWait:
			Nachos_SemWait();
			break;
		case SC_SemSignal:
			Nachos_SemSignal();
			break;
             default:
                printf("Unexpected syscall exception %d\n", type );
                ASSERT(false);
                break;
          }
       		break;
	case PageFaultException:
		//printf("PageFaultException...\n");
		Nachos_PageFaultException();
		break;
       default:
          printf( "Unexpected exception %d\n", which );
          ASSERT(false);
          break;
    }

}






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
#include "nachostabla.h"
#include "translate.h"
#include <fcntl.h>


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
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
NachosOpenFilesTable* openedFiles = new NachosOpenFilesTable();

void returnFromSystemCall() {

        int pc, npc;

        pc = machine->ReadRegister( PCReg );
        npc = machine->ReadRegister( NextPCReg );
        machine->WriteRegister( PrevPCReg, pc );        // PrevPC <- PC
        machine->WriteRegister( PCReg, npc );           // PC <- NextPC
        machine->WriteRegister( NextPCReg, npc + 4 );   // NextPC <- NextPC + 4

}       // returnFromSystemCall


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

void Nachos_Halt() {                    // System call 0

        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();

}       // Nachos_Halt

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
	char buffer[100];
	int nameNachos = machine->ReadRegister(4);
	int fin = '\0';
	int value = 0;
	int posicion = 0;
	//Read Name File
	do{
		machine->ReadMem(nameNachos+posicion,1,&value);
		buffer[posicion] = value;
		posicion++;
	}while(value != fin);
	buffer[posicion]=fin;
	int UnixHandel = creat("PruebaWrite.txt", 0777);
	if(UnixHandel != -1){
		int NachosHandle = openedFiles->Open(UnixHandel);
		machine->WriteRegister( 2, NachosHandle + 3); 
		printf("Se creó el archivo\n");
	}
	else{
		perror("Error al crear el archivo\n");	
		exit(1);
	}

	 returnFromSystemCall();


}       // Nachos_Create


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
	char buffer[100];
	int nameNachos = machine->ReadRegister(4);
	char* buffer  = ReadFromNachosMemory(nameNachos);
	int fin = '\0';
	int value = 0;
	int posicion = 0;
	//Read Name File
	do{
		machine->ReadMem(nameNachos+posicion,1,&value);
		buffer[posicion] = value;
		posicion++;
	}while(value != fin);

	buffer[posicion]='\0';
	int UnixHandel = open((const char *)buffer,O_RDWR);
	if(UnixHandel != -1){
		int NachosHandle = openedFiles->Open(UnixHandel);
		machine->WriteRegister( 2, NachosHandle + 3); 
		printf("Se abrio el archivo\n");
	}
	else{
		perror("Error al abrir el archivo\n");	
		exit(1);
	}

	 returnFromSystemCall();


}       // Nachos_Open

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
			if(openedFiles->isOpened(id-3)){
				Unix = openedFiles->getUnixHandle(id-3);
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
			
			if(openedFiles->isOpened(id-3)){
				Unix = openedFiles->getUnixHandle(id-3);
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

void Nachos_Exit(){
		
}

void Nachos_Close(){
	
	int NachosHandle = machine->ReadRegister(4);
	printf("Cerrando el archivo %d \n",NachosHandle);
	if(openedFiles->isOpened(NachosHandle)){
		int UnixHandle = openedFiles->Close(NachosHandle-3);
		machine->WriteRegister(2,close(UnixHandle));
	}

	 returnFromSystemCall();
}

void Nachos_SemCreate(){
	
	int valorInicial= machine->ReadRegister(5);
	int nombre = machine->ReadRegister(4);
        char buffer[100];
	int nameNachos = machine->ReadRegister(4);
	int fin = '\0';
	int value = 0;
	int posicion = 0;
	do{
		machine->ReadMem(nombre+posicion,1,&value);
		buffer[posicion] = value;
		posicion++;
	}while(value != fin);
	buffer[posicion]=fin;
	Semaphore * semActual = new Semaphore(buffer, valorInicial);
        int espacio = controlSem->Find();
	vectorSem[espacio] = semActual;
	controlSem->Mark(espacio);
	machine->WriteRegister(2,espacio);
	printf("Semaforo creado\n");
}

void Nachos_SemDestroy(){
	
	int id = machine->ReadRegister(4);
	delete vectorSem[id];
	controlSem->Clear(id);
	printf("Destruccion de semaforos");
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
             default:
                printf("Unexpected syscall exception %d\n", type );
                ASSERT(false);
                break;
          }
       		break;
       default:
          printf( "Unexpected exception %d\n", which );
          ASSERT(false);
          break;
    }

}

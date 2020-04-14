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
void func() {
  printf("Thread Switch\n");
  interrupt->Halt();
}
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

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } 
    else if (which == PageFaultException) {
      int vAddress = machine->ReadRegister(BadVAddrReg);
      DEBUG('a', "Virtual Address Missed.\n");
      //add x
      #ifdef IPT_USE
        machine->replaceIpt(vAddress);
      #else
      if (machine->tlb != NULL) {
        //...
        DEBUG('a', "TLB MISS HANDLER");
        machine->replaceTlbLRU(vAddress);
        //machine->replaceTlbFIFO(vAddress);
      }
      #endif
    } else if((which == SyscallException) && (type == SC_Exit)) {
      int status = machine->ReadRegister(4);
      Thread *t = new Thread("test 1");
      t->Fork((VoidFunctionPtr)(func), (void*)("./test/Halt"));
      currentThread->Finish();
      printf("Prog Exit with status %d\n", status);
      
      
      //interrupt->Halt();
    } else {
      printf("Unexpected user mode exception %d %d\n", which, type);
      machine->DumpState();
      printf("Main Memory\n");
      for (int i = 0; i < machine->pageTableSize; i++) {
        printf("%d:\t%d", i, machine->pageTable[i].physicalPage);
        if (i % 4 == 3)
          printf("\n");
        else
          printf("\t");
      }
      ASSERT(FALSE);
    }
}

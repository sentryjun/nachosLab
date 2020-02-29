// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
        //Add by jun test tid uid
	printf("*** thread %d name %s tid %d uid %d looped %d times\n", 
            which, currentThread->getName(), currentThread->getThreadID(), 
            currentThread->getUserID(),num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}
//add by jun
//-----------------------------------------------------------------------
// ThreadTest2
//  Test the Threads Limits
//p----------------------------------------------------------------------

void
ThreadTest2() 
{ 
    DEBUG('t', "Entering ThreadTest2");
    for (int i = 0; i <= 128; i++) {
      Thread *t = new Thread("Fork Thread");
      printf("*** thread with pid %d forked\n", t->getThreadID());
    }
}

//----------------------------------------------------------------------
//  TSprint
//    print all threads status
//----------------------------------------------------------------------

void TSprint(int what) 
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  printf("%10s %10s %10s %15s\n", "uid", "tid", "name", "status");
  ThreadPrint((int)currentThread);
  List *lst = scheduler->getReadyList();
  lst->Mapcar(ThreadPrint);
  currentThread->Yield();
  interrupt->SetLevel(oldLevel);
}

//-----------------------------------------------------------------------
//  ThreadTest3
//    Test TS
//-----------------------------------------------------------------------

void ThreadTest3() {
    DEBUG('t', "Entering ThreadTest3");

    Thread *t[5];
    for (int i = 0; i < 5; i++) {
      t[i] = new Thread("forked Thread");
    }

    for (int i = 0; i < 5; i++) {
      t[i]->Fork((VoidFunctionPtr)TSprint, (void*) 0);
    }
    
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	    ThreadTest1();
        break;
    case 2:
      ThreadTest2();
      break;
    case 3:
      ThreadTest3();
    default:
      printf("No test specified.\n");
      break;
    }
}


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
        currentThread->useSlice();
        printf("*** thread %d name %s tid %d slice %d looped %d times\n", which,
               currentThread->getName(), currentThread->getThreadID(),
               currentThread->getUsedSlice(), num);
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
  printf("%10s %10s %10s %10s %15s\n", "uid", "tid", "name", "priority", "status");
  ThreadPrint((int)currentThread);
  List *lst = scheduler->getReadyList();
  lst->Mapcar(ThreadPrint);
  currentThread->Yield();
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

void PriorityTest() { 
  for (int i = 0; i < 5; i++) {
    printf("thread name=%s, id=%d, userid=%d, priority=%d looped %d times\n",
           currentThread->getName(), currentThread->getThreadID(),
           currentThread->getUserID(), currentThread->getPriority(), i);
    //currentThread->Yield();
  }
}
void PriorityTest1() { 
  for (int i = 0; i < 70; i++) {
    printf("thread name=%s, id=%d, usedSlice=%d, level=%d looped %d times\n",
           currentThread->getName(), currentThread->getThreadID(),
           currentThread->getUsedSlice(), currentThread->getLevel(), i);
    //currentThread->Yield();
    interrupt->OneTick();
    interrupt->OneTick();
    interrupt->OneTick();
    interrupt->OneTick();
  }
}
void PriorityTest2() { 
  for (int i = 0; i < 40; i++) {
    printf("thread name=%s, id=%d, usedSlice=%d, level=%d looped %d times\n",
           currentThread->getName(), currentThread->getThreadID(),
           currentThread->getUsedSlice(), currentThread->getLevel(), i);
    //currentThread->Yield();
    interrupt->OneTick();
    interrupt->OneTick();
    interrupt->OneTick();
    interrupt->OneTick();
    if (i == 30) {
      Thread *newT = new Thread("fork N", 4);
      newT->Fork((VoidFunctionPtr)PriorityTest1, (void*)4);
    }
  }
}
void PriorityTest3() { 
  for (int i = 0; i < 50; i++) {
    printf("thread name=%s, id=%d, usedSlice=%d, level=%d looped %d times\n",
           currentThread->getName(), currentThread->getThreadID(),
           currentThread->getUsedSlice(), currentThread->getLevel(), i);
    //currentThread->Yield();
    interrupt->OneTick();
    interrupt->OneTick();
    interrupt->OneTick();
    interrupt->OneTick();
  }
}
//------------------------------------------------------------------------
//  Prority Test
//-----------------------------------------------------------------------
void ThreadTest4() { 
  DEBUG('t', "Entering ThreadTest4");
  Thread *t1 = new Thread("fork 1", 3);
  t1->Fork((VoidFunctionPtr)PriorityTest, (void*)1);
  Thread *t2 = new Thread("fork 2", 1);
  t2->Fork((VoidFunctionPtr)PriorityTest, (void*)2);
  Thread *t3 = new Thread("fork 3", 2);
  t3->Fork((VoidFunctionPtr)PriorityTest, (void*)3);

  //currentThread->Yield();
  PriorityTest();
}

void ThreadTest5() { 
  DEBUG('t', "Entering ThreadTest5\n");
  

    Thread *t1 = new Thread("Fork1", 3, 4);
    t1->Fork((VoidFunctionPtr)PriorityTest1, (void*)1);
    Thread *t2 = new Thread("Fork2", 3, 3);
    t2->Fork((VoidFunctionPtr)PriorityTest1, (void*)2);
  //interrupt->OneTick();

  currentThread->Yield();
  
}
void ThreadTest6() { 
  DEBUG('t', "Entering ThreadTest6\n");
  

    Thread *t1 = new Thread("Fork1", 3);
   
    Thread *t2 = new Thread("Fork2", 3);
   
    Thread *t3 = new Thread("Fork3", 3);
    
    
    t1->Fork((VoidFunctionPtr)PriorityTest1, (void*)1);
    t2->Fork((VoidFunctionPtr)PriorityTest2, (void*)2);
    t3->Fork((VoidFunctionPtr)PriorityTest3, (void*)3);
  //interrupt->OneTick();
    //PriorityTest2();
    currentThread->Yield();
}

#define N 256
int bufferNumber = 0;
Semaphore *mutex = new Semaphore("MUTEX", 1);
Semaphore *empty = new Semaphore("EMPTY", N);
Semaphore *full = new Semaphore("FULL", 0);
void SempProducer(int num) {
  while (1)
  {
    empty->P();
    mutex->P();
    if (bufferNumber >= N) { 
      printf("Producer can not produce\n");
    } else {
      printf("Producer %d make products %d \n",currentThread->getThreadID(), bufferNumber);
      bufferNumber++;
    }
    mutex->V();
    full->V();
  }
}
void SempConsumer(int num) {
  while (1){
    /* code */
    full->P();
    mutex->P();
    if (bufferNumber <= 0) {
      printf("customer cannot use\n");
    } else {
      printf("Customer %d get products %d \n", currentThread->getThreadID(),
             bufferNumber);
      bufferNumber--;
    }
    mutex->V();
    empty->V();
  }
}
void SempTest() { 
  Lock *lock = new Lock("LockTest"); 
  
  }

Condition *conPro = new Condition("Producer");
Condition *conCon = new Condition("Consumer");
Lock *bufferLock = new Lock("bufferLock");
void CondProducer(int num) {
  while (1)
  {
    bufferLock->Acquire();
    while (bufferNumber >= N) {
      printf("Producer canot produce\n");
      conPro->Wait(bufferLock);
    }
    printf("Producer %d make products %d \n",currentThread->getThreadID(), bufferNumber);
    bufferNumber++;

    conCon->Signal(bufferLock);
    bufferLock->Release();
  }

  
}

void CondConsumer(int num) {
  while (1)
  {
    bufferLock->Acquire();
    while (bufferNumber <= 0)
    {
      printf("Consumer canot get\n");
      conCon->Wait(bufferLock);
    }
    printf("Customer %d get products %d \n", currentThread->getThreadID(),
             bufferNumber);
    bufferNumber--;
    conPro->Signal(bufferLock);
    bufferLock->Release();
  }
}

void CondTest() {
  DEBUG('t', "Entering Condition Test\n");
  Thread *p1 = new Thread("producer 1");
  Thread *p2 = new Thread("producer 2");

  Thread *c1 = new Thread("Consumer 1");
  Thread *c2 = new Thread("Consumer 2");
  p1->Fork(CondProducer, (void *)1);
  p2->Fork(CondProducer, (void *)2);
  c1->Fork(CondConsumer, (void *)3);
  c2->Fork(CondConsumer, (void *)4);
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
      break;
    case 4:
      scheduler->setAlgorithm(FPPS);
      ThreadTest4();
      break;
    case 5:
      scheduler->setAlgorithm(RRS);
      ThreadTest5();
      break;
    case 6:
      scheduler->setAlgorithm(MLQS);
      ThreadTest6();
      break;
    case 7:
      scheduler->setAlgorithm(RRS);
      CondTest();
      break;
    default:
      printf("No test specified.\n");
      break;
    }
}


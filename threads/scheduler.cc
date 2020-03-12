// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "scheduler.h"
#include "system.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads to empty.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{ 
    readyList = new List;
    readyList1 = new List;
    readyList2 = new List;
    algorithm = FCFS;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList;
    delete readyList1;
    delete readyList2;
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    DEBUG('t', "Putting thread %s on ready list.\n", thread->getName());

    thread->setStatus(READY);
    //readyList->Append((void *)thread);
    //add by jun
    //sorted insert
    switch (algorithm)
    {
    case FCFS:
      readyList->Append((void *)thread);
      break;
    case FPPS:
      readyList->SortedInsert((void *)thread, thread->getPriority());
      break;
    case RRS:
      readyList->Append((void *)thread);
      break;
    case MLQS:
      ReadyToRunMLQS(thread);
      break;
    default:
      readyList->Append((void *)thread);
      break;
    }
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    //add by jun
    switch (algorithm)
    {
    case FCFS:
      return (Thread *)readyList->Remove();
    case FPPS:
      return FindNextToRunFPPS();
    case RRS:
      return FindNextToRunRRS();
    case MLQS:
      return FindNextToRunMLQS();
    default:
      return (Thread *)readyList->Remove();
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread)
{
    Thread *oldThread = currentThread;
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (currentThread->space != NULL) {	// if this thread is a user program,
        currentThread->SaveUserState(); // save the user's CPU registers
	currentThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    currentThread = nextThread;		    // switch to the next thread
    currentThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG('t', "Switching from thread \"%s\" to thread \"%s\"\n",
	  oldThread->getName(), nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);
    
    DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!
    if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
	threadToBeDestroyed = NULL;
    }
    
#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {		// if there is an address space
        currentThread->RestoreUserState();     // to restore, do it.
	currentThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    printf("Ready list contents:\n");
    readyList->Mapcar((VoidFunctionPtr) ThreadPrint);
    printf("Ready list 1 contents:\n");
    readyList1->Mapcar((VoidFunctionPtr) ThreadPrint);
    printf("Ready list 2 contents:\n");
    readyList2->Mapcar((VoidFunctionPtr) ThreadPrint);
}

bool Scheduler::FPPSCheck(Thread *t) {
    if(algorithm != FPPS){
      return false;
    }
    Thread *next = (Thread*)readyList->getFirst(); 
    if (next != NULL && next->getPriority() < t->getPriority()) {
      return true;
    }
    return false;
}

Thread* Scheduler::FindNextToRunFPPS(){
    Thread *nextThread;
    nextThread = (Thread *)readyList->Remove();
    DEBUG('t', "next thread is%d\n", (int)nextThread);
    if (nextThread == NULL) {
      return NULL;
    }
    if (currentThread != threadToBeDestroyed &&
        nextThread->getPriority() >= currentThread->getPriority()) {
      readyList->SortedInsert((void *)nextThread, nextThread->getPriority());
      return NULL;
    } else {
      return nextThread;
    }
    return nextThread;
}

bool Scheduler::RRSCheck(Thread*t)  { 
  if (algorithm != RRS) {
    return false;
  }
  
  return t->getUsedSlice() >= t->getSlice(); 
  };

Thread* Scheduler::FindNextToRunRRS() {
  //scheduler->getReadyList()->Mapcar(ThreadPrint);
  Thread *nextThread;
  nextThread = (Thread *)readyList->Remove();
  currentThread->clearSlice();
  if (nextThread == NULL) {
    return NULL;
  }
    return nextThread;
}
void Scheduler::ReadyToRunMLQS(Thread *thread) { 
  int l = thread->getLevel();
  switch (l)
  {
  case 0:
    readyList->Append((void *)thread);
    break;
  case 1:
    readyList1->Append((void *)thread);
    break;
  case 2:
    readyList2->Append((void *)thread);
    break;
  default:
    readyList->Append((void *)thread);
    break;
  }
}
bool Scheduler::MLQSCheck(Thread *t) {
  //printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
  if (algorithm != MLQS) {
    return false;
  }
  bool test = (t->getUsedSlice()) >= (t->getSlice());
  return t->getUsedSlice() >= t->getSlice();
}

Thread *Scheduler::FindNextToRunMLQS() {
        //scheduler->Print();
  int l = currentThread->getLevel();
  //如果发生了抢断那么未用完的时间片会当作用完

  bool flag = MLQSCheck(currentThread);
  DEBUG('t', "\ncurrent Thread level = %d \n Used Time Slice is %d\n", l,
        currentThread->getUsedSlice());
  
  if (flag) {
    if (l < 2) {
      currentThread->setLevel(l+1);
    } else {
      currentThread->setLevel(2);
    }
  }
  DEBUG('t', "\nnew Thread level = %d \n\n", currentThread->getLevel());
  if(!readyList->IsEmpty()) {
    //任何情况下只要优先级最高的队列有元素就会先返回
    return (Thread *)readyList->Remove();
  } 
  if(readyList->IsEmpty() && (!readyList1->IsEmpty())){
    //如果第一队列没有元素
    //优先级第二稿的有元素也会返回
    //因为就算原来的优先级为0现在也要插入T优先级1的队列里
    return (Thread *)readyList1->Remove();
  }
  if (readyList->IsEmpty() && readyList1->IsEmpty() && l == 0 && currentThread != threadToBeDestroyed) {
    //在第一队列和第二队列都没有线程时，
    //如果当前线程不是要结束的线程
    //并且原先是最高优先级继续运行自己
    return NULL;
  }
   //最后再去调第三个List
  return (Thread *)readyList2->Remove();
  
}

bool Scheduler::MLQSCheck1(Thread *t) { 
  int l = t->getLevel();
  if(algorithm != MLQS) {
    return false;
  }
  if (l > 0 && (!readyList->IsEmpty())) {
    return true;
  }
  if (l > 1 && (!readyList1->IsEmpty())) {
    return true;
  }
  return false;
}
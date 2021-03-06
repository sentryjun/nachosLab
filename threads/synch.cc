// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
  name = debugName;
  lockSema = new Semaphore(debugName, 1);
  owner = NULL;
}
Lock::~Lock() { delete lockSema; }
void Lock::Acquire() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  lockSema->P();
  owner = currentThread;
  (void)interrupt->SetLevel(oldLevel);
}
void Lock::Release() { 
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  ASSERT(isHeldByCurrentThread());
  lockSema->V();
  owner = NULL;
  (void)interrupt->SetLevel(oldLevel);
}
bool Lock::isHeldByCurrentThread() { return currentThread == owner; }

Condition::Condition(char* debugName) {
  name = debugName;
  queue = new List;
  
}
Condition::~Condition() { delete queue; }
void Condition::Wait(Lock* conditionLock) { 
    //ASSERT(FALSE);
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread());
    queue->Append((void *)currentThread);
    conditionLock->Release();
    currentThread->Sleep();
    conditionLock->Acquire();
    (void)interrupt->SetLevel(oldLevel);
}
void Condition::Signal(Lock* conditionLock) {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  ASSERT(conditionLock->isHeldByCurrentThread());
  Thread *thread =(Thread *) queue->Remove();
  if(thread != NULL) {
      
    scheduler->ReadyToRun(thread);
  }
  (void)interrupt->SetLevel(oldLevel);
}
void Condition::Broadcast(Lock* conditionLock) {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  ASSERT(conditionLock->isHeldByCurrentThread());
  while (!queue->IsEmpty()) {
    Signal(conditionLock);
  }
  (void)interrupt->SetLevel(oldLevel);
}

Barrier::Barrier(char *debugName, int number) { 
  name = debugName;
  this->number = number;
  waiting = 0;
  barrierCondition = new Condition(debugName);
  barrierLock = new Lock(debugName);
}

Barrier::~Barrier() {
  delete barrierLock;
  delete barrierCondition;
}

void Barrier::Wait() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  barrierLock->Acquire(); 
  if (waiting < number) {
    waiting++;
    barrierCondition->Wait(barrierLock);
  } else {
    barrierCondition->Broadcast(barrierLock);
  }
  (void)interrupt->SetLevel(oldLevel);
}

RWLock::RWLock(char *debugName){
  name = debugName;
  //rLock = new Lock("reader");
  wLock = new Lock("writer");
  rcount = 0;
  wflag = false;
  // reader = new List();
}

RWLock::~RWLock() {
  //delete rLock;
  delete wLock;
}

void RWLock::AquireR() { 
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  //如果没有读者 第一个读者自动获得写锁
  if (rcount <= 0 || wflag == false) {
    wflag = true;
    wLock->Acquire();
  }
  rcount++;
  //rLock->Acquire();
  (void)interrupt->SetLevel(oldLevel);
}

void RWLock::ReleaseR() {
   IntStatus oldLevel = interrupt->SetLevel(IntOff);
  //如果是最后一个读者
  if (wLock->isHeldByCurrentThread()) {
    wflag = false;
    wLock->Release();
  }
  rcount--;
  (void)interrupt->SetLevel(oldLevel);
}
void RWLock::ReleaseW() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  wflag = false;
  wLock->Release();
  (void)interrupt->SetLevel(oldLevel);
}

bool RWLock::isWriter() { return wLock->isHeldByCurrentThread(); }
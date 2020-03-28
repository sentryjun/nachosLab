#include "syscall.h"
#define MAXN 800

int A[MAXN];
#include "syscall.h"

int
main()
{
  int i = 0;
  for (i = 0; i < MAXN; i++) {
    A[i] = i;
  }
  Halt();
  /* not reached */
}

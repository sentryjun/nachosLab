#include "syscall.h"
#define MAXN 2048

int A[MAXN];
#include "syscall.h"

int
main()
{
  int i = 0;
  for (i = 0; i < MAXN; i++) {
    A[i] = i;
  }
  Exit(A[0]);
  /* not reached */
}

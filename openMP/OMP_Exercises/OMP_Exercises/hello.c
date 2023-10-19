#include <omp.h>

int main()
{
  int i = 0;
#pragma omp parallel for ordered
  for (i = 0; i < 10; i++)
  {
#pragma omp ordered
    // {
      int ID = omp_get_thread_num();
      // #pragma omp ordered
      printf("Hello World from %d\n", ID);
    // }
  }
}
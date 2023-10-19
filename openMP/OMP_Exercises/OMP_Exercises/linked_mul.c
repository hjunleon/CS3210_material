#include <stdlib.h>
#include <stdio.h>
#include "omp.h"

#ifndef N
#define N 50
#endif
#ifndef FS
#define FS 6//38
#endif

#define LEN FS + N + 1
#define NULL_VAL -1

static long long fib_cache[LEN];

struct node
{
   int data;
   int fibdata;
   struct node *next;
};

int fib(int n)
{
   int x, y, result, tmp;
   if (n < 2)
   {
      return (n);
   }
   else
   {
#pragma omp critical
      {
         tmp = fib_cache[n];
      }
      if (fib_cache[n] != NULL_VAL)
      {
         // printf("fib_cache[%d]: %d\n ", n, fib_cache[n]);
         return fib_cache[n];
      }

      x = fib(n - 1);
      y = fib(n - 2);
      result = (x + y);
      fib_cache[n] = result;
      return result;
   }
}

void processwork(struct node *local_p)
{
   int n = local_p->data;
   // printf("Initial n: %d\n", n);
   local_p->fibdata = fib(n);
   printf("%d got %d\n",n,local_p->fibdata);
}

struct node *init_list(struct node *p)
{
   int i;
   struct node *head = NULL;
   struct node *temp = NULL;

   head = malloc(sizeof(struct node));
   p = head;
   p->data = FS;
   p->fibdata = 0;
   for (i = 0; i < N; i++)
   {
      temp = malloc(sizeof(struct node));
      p->next = temp;
      p = temp;
      p->data = FS + i + 1;
      p->fibdata = i + 1;
   }
   p->next = NULL;
   return head;
}

void init_fib_cache()
{
   printf("Cache has length: %d\n", LEN);
   for (int i = 0; i < LEN; i++)
   {
      fib_cache[i] = NULL_VAL;
   }
   fib_cache[0] = 0;
   fib_cache[1] = 1;
}

int main(int argc, char *argv[])
{
   double start, end;
   struct node *p = NULL;
   struct node *temp = NULL;
   struct node *head = NULL;

   printf("Process linked list\n");
   printf("  Each linked list node will be processed by function 'processwork()'\n");
   printf("  Each ll node will compute %d fibonacci numbers beginning with %d\n", N, FS);

   p = init_list(p);
   init_fib_cache();
   head = p;

   start = omp_get_wtime();
   {
      int idx;
      //private(idx)
#pragma omp parallel for private(idx) shared(p)
      for (int idx = 0; idx <= N; idx++)
      {
         printf("THread %d with idx: %d\n", omp_get_thread_num(),idx);
         struct node *local_cp = malloc(sizeof(struct node));
#pragma omp critical
         {
            local_cp->data = p->data;
            local_cp->fibdata = p->fibdata;
            local_cp->next = p->next;
            p = p->next;
         }
         processwork(local_cp);
         struct node *to_edit = head;
         while(local_cp->data != to_edit->data)
         // for (int i = 0; i < idx; i++)     // the thread id and idx are NOT synced because nvr specified ordered + recall scheduling, but not the local_cp 
         {
            to_edit = to_edit->next;
         }
         printf("THread %d with idx: %d completed operation\n", omp_get_thread_num(),idx);
         printf("Thread %d editting node %d/%d from %d to %d \n", omp_get_thread_num(),to_edit->data, local_cp->data,to_edit->fibdata, local_cp->fibdata);
         // to_edit->data = local_cp->data;
         // fib_cache[to_edit->data];
         to_edit->fibdata = local_cp->fibdata;
         // to_edit->next = local_cp->next;
         free(local_cp);
      }
   }

   end = omp_get_wtime();
   p = head;
   while (p != NULL)
   {
      printf("%d : %d\n", p->data, p->fibdata);
      temp = p->next;
      free(p);
      p = temp;
   }
   free(p);

   printf("Compute Time: %f seconds\n", end - start);

   return 0;
}

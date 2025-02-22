/*

This program will numerically compute the integral of

				  4/(1+x*x)

from 0 to 1.  The value of this integral is pi -- which
is great since it gives us an easy way to check the answer.

The is the original sequential program.  It uses the timer
from the OpenMP runtime library

History: Written by Tim Mattson, 11/99.

*/
#include <stdio.h>
#include <omp.h>
static long num_steps = 1000000;
double step;
int main()
{
	int i;
	double pi, sum = 0.0;
	double start_time, run_time;

	step = 1.0 / (double)num_steps;

	start_time = omp_get_wtime();
#pragma omp parallel for shared(sum) private(i)
// #pragma omp parallel for private(i) reduction(+:sum)
	for (i = 1; i <= num_steps; i++)
	{
		double x = (i - 0.5) * step;
		#pragma omp atomic
		sum += 4.0 / (1.0 + x * x);
	}

	pi = step * sum;
	run_time = omp_get_wtime() - start_time;
	printf("\n pi with %d steps is %f in %f seconds ", num_steps, pi, run_time);
}

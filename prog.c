#include <unistd.h>
#include <inttypes.h>
#include <omp.h>
#include <sys/time.h> 
#include <stdlib.h>
#include <stdio.h>

double wtime()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

char func(int v)
{
  if(v == 2)
    return '2';
  else if(v == 4)
    return '4';
  else if(v == 6)
    return '6';
  return '8';
}

void matrix_vector_product(double *a, double *b, double *c, int m, int n)
{
  for (int i = 0; i < m; i++)
  {
    c[i] = 0.0;
    for (int j = 0; j < n; j++)
      c[i] += a[i * n + j] * b[j];
  }
}

void matrix_vector_product_omp(double *a, double *b, double *c, int m, int n, int threads)
{
#pragma omp parallel num_threads(threads)
{
  int nthreads = omp_get_num_threads();
  int threadid = omp_get_thread_num();
  int items_per_thread = m / nthreads;
  int lb = threadid * items_per_thread;
  int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
  for (int i = lb; i <= ub; i++)
  {
    c[i] = 0.0;
    for (int j = 0; j < n; j++)
      c[i] += a[i * n + j] * b[j];
  }
}
}

double run_serial(int size)
{
  double *a, *b, *c;
  int m, n;
  m = n = size;
  a = malloc(sizeof(*a) * m * n);
  b = malloc(sizeof(*b) * n);
  c = malloc(sizeof(*c) * m);
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
      a[i * n + j] = i + j;
  }
  for (int j = 0; j < n; j++)
    b[j] = j;
  double t = wtime();
  matrix_vector_product(a, b, c, m, n);
  t = wtime() - t;
  printf("Elapsed time (serial): %.6f sec.\n", t);
  free(a);
  free(b);
  free(c);
  return t;
}


double run_parallel(int size, int threads)
{
  double *a, *b, *c;
  int m, n;
  m = n = size;
  // Allocate memory for 2-d array a[m, n]
  a = malloc(sizeof(*a) * m * n);
  b = malloc(sizeof(*b) * n);
  c = malloc(sizeof(*c) * m);
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
      a[i * n + j] = i + j;
  }
  for (int j = 0; j < n; j++)
    b[j] = j;
  double t = wtime();
  matrix_vector_product_omp(a, b, c, m, n, threads);
  t = wtime() - t;
  printf("Elapsed time (parallel): %.6f sec.\n", t);
  free(a);
  free(b);
  free(c);
  return t;
}

int main(int argc, char **argv)
{
  FILE *fp1, *fp2, *fp3;
  int m, n;
  double serial, parallel;
  char buf[9], bufn[5] = {' ', ' ', ' ', ' ', '\0'};
  m = n = 15000;
  fp1 = fopen("data1.dat", "wb");
  fp2 = fopen("data2.dat", "wb");
  fp3 = fopen("data3.dat", "wb");
  for(int i = 0; i < 3; i++)
  {
    int k = m + (i * 5000);
    printf("Matrix-vector product (c[m] = a[m, n] * b[n]; m = %d, n = %d)\n", k, k);
    printf("Memory used: %" PRIu64 " MiB\n", ((k * k + 2 * k) * sizeof(double)) >> 20);
    for(int j = 2; j <= 8; j += 2)
    {
      serial = run_serial(k);
      parallel = run_parallel(k, j);
      snprintf(buf, 10, "%.6f", serial / parallel);
      buf[8] = '\n';
      bufn[0] = func(j);
      if(i == 0)
      {
        fputs(bufn, fp1);
        fputs(buf, fp1);
      }
      else if(i == 1)
      {
        fputs(bufn, fp2);
        fputs(buf, fp2);
      }
      else
      {
        fputs(bufn, fp3);
        fputs(buf, fp3);
      }
    }
  }
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  
  return 0;
}

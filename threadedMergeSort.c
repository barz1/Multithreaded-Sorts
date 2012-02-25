/*Michael Barszcz 
*Copyright February 2012
* This is an implementation of a multithreaded, non-recursive mergesort
* right now it is set up to work only for array sizes that are a power of
* 2 (womp womp) and works for ints, floats, doubles, chars
* I used OpenMP to multithread it. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define TYPE    double

extern clock_t clock();
void nrmgsrt (void *, int, int, int (*)(const void *, const void *));
int compare(const void *, const void *);
void init_list(TYPE *, int);
int test_sort(TYPE *, int);


int main(int argc, char **argv) {
  //Check for proper number of arguments, argument 1 corresponds to array size
  if (argc != 2) {
      printf("Usage: %s [problem size]\n", argv[0]);
      return 1;
  }
  clock_t t1, t0;
  char *sort = (char *)malloc(sizeof(char)*7);
  double elapsed = 0.0;
  int i, N = atoi(argv[1]);
  TYPE *lst = malloc(sizeof(TYPE)*N);

  init_list(lst, N);

  //Print initial unsorted array if compiled with -DPRNT
  #ifdef PRNT
  for (i=0; i<N; i++) {
    printf("Value %2d: %d\n", i, lst[i]);
  }
  printf("\n\n");
  #endif

  t0 = clock();
  nrmgsrt((TYPE *)lst, N, sizeof(TYPE), compare);
  sort = "NMGSRT";
  t1 = clock();
  elapsed = (t1-t0)/((double)CLOCKS_PER_SEC);

  //Print sorted array if compiled with -DPRNT
  #ifdef PRNT
  for (i=0; i<N; i++) {
    printf("Value %2d: %d\n", i, lst[i]);
  }
  #endif

  //Test if array is sorted
  if (test_sort(lst, N)) 
      printf("%s Passed, Elapsed Time: %10.8f\n", sort, elapsed);
  else
      printf("%s Failed\n", sort);

  free(lst);
  return 0;
}

//Function to generate array of n elements of type TYPE
void init_list(TYPE *lst, int n) {
  int i, seed = time(0)%100;
  srand(seed);
  for (i=0; i<n; i++) {
    lst[i] = (TYPE)rand();
  }
}

//Function to test if array is sorted
int test_sort(TYPE *lst, int size) {
    int i;
    for (i=0; i<size-1; i++) {
        if (lst[i] > lst[i+1]) {
	    printf("Failed at index: %d\n", i);
            return 0;
        }
    }
    return 1;
}

//Compare function
int compare(const void *a, const void *b) {
  return (*(TYPE *)a - *(TYPE *)b);
}

//Non-recursive merge sort
void nrmgsrt(void *keys, int n, int size, int (*compare)(const void *, const void *)) {
    char *key = (char *)keys;
    int i, j, k, index, start, end, middle, blks = 2, lsize = 1, rsize = 1;
    int groups = n/blks;
    char *larr, *rarr;
    while (groups > 0) {
	#pragma omp parallel 
	{
	    #pragma omp for private(end, middle, index, i, j, k, larr, rarr)
 	    for (start=0; start<=n-blks; start+=blks) {
                char *larr = (char *)calloc(blks, sizeof(char)*size);
                char *rarr = (char *)calloc(blks, sizeof(char)*size);
	        end = start+blks;
	        middle = (start+end)/2;

                //Copy values from array to left subarray
                index=0;
                for (i=start; i<middle; i++) 
                    memcpy(&larr[index++*size], &key[i*size], size);
                //Copy values from array to right subarray
                index=0;
                for (i=middle; i<end; i++) 
                    memcpy(&rarr[index++*size], &key[i*size], size);
            
                i=j=0;
                k=start;
                //Begin merging values back into final array
                while (i<lsize && j<rsize) {
                    if (compare(&larr[i*size], &rarr[j*size]) < 0) 
                        memcpy(&key[k++*size], &larr[i++*size], size);
                    else 
                        memcpy(&key[k++*size], &rarr[j++*size], size);
                }                
                //Copy any remaining values from left subarray
                while (i<lsize) 
                    memcpy(&key[k++*size], &larr[i++*size], size);
                //Copy any remaining values from right subarray
                while (j<rsize) 
                    memcpy(&key[k++*size], &rarr[j++*size], size);

                free(larr);
                free(rarr);
            }   
	}
        rsize = lsize = blks;
        blks = blks*2;
        groups = n/blks;
    }
}

/* Here is a C example to read "abs02" and print the rectangles: */
/* (blemished by preprocessor directives required for Windows (sorry)) */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if defined (_WIN64)
#  define stat stat64
#  define fstat fstat64
#endif

#define dim 2

typedef struct {
               double  l, h;
               } typinterval;
typedef typinterval  typrect[dim];


int main() {
  int datafile;
  typrect rectangle;
  int ferr, length, numbRects, nbytes, i, j;
  struct stat status;
# if defined (_WIN32) || defined (_WIN64)
    int O_MODE= O_RDONLY | O_BINARY;
# else
    int O_MODE= O_RDONLY;
# endif

  datafile= open("abs02",O_MODE);
  
  ferr= fstat(datafile,&status);
  length= status.st_size;
  numbRects= length / sizeof(typrect);
  
  printf("Number of rectangles: %10d\n",numbRects);
  printf(" Size of a rectangle: %10d\n",(int)sizeof(typrect));
  printf("\n");
  
  i= 0;
  do {
    nbytes= read(datafile,rectangle,sizeof(typrect));
    if (nbytes != sizeof(typrect)) {
      printf("Error during read!\n");
      return 1;
    }
    
    for (j= 0; j < dim; j++) {
      printf("% .15e % .15e\n",rectangle[j].l,rectangle[j].h);
    }
    printf("\n");
    
    i++;
  } while (i < numbRects);
  
  close(datafile);
  return 0;
}

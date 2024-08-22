// For Sun Solaris on i386 architecture, here is a
// C example to read "abs02" and print the rectangles:
/******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
  
  datafile= open("abs02",O_RDONLY);
  
  ferr= fstat(datafile,&status);
  length= status.st_size;
  numbRects= length / sizeof(typrect);
  
  printf("Number of rectangles: %10d\n",numbRects);
  printf(" Size of a rectangle: %10d\n",sizeof(typrect));
  printf("\n");
  
  i= 0;
  do {
    nbytes= read(datafile,rectangle,sizeof(typrect));
    if (nbytes != sizeof(typrect)) {
      printf("Error during read!\n");
      return 1;
    }
    
    for (j= 0; j < 2; j++) {
      printf("% .15e % .15e\n",rectangle[j].l,rectangle[j].h);
    }
    printf("\n");
    
    i++;
  } while (i < numbRects);
  
  close(datafile);
  return 0;
}
/******************************************************************/
/*
CAUTION
regarding conversion to ASCII for programs reading in ASCII numbers.
NOTE that, especially in the artificial files (see "description.pdf"),
the stored doubles fill in the complete mantissa. Thus a conversion with
less then 15 fractional digits would lead to falsified data.


First lines of output:
----------------------
Here are the first lines of output from the program above:

Number of rectangles:    1000000
 Size of a rectangle:         32

 7.912761233509085e-05  8.278824376602774e-04
 1.435067319239531e-04  9.470766944464349e-04

 1.113108637611602e-03  1.967621643689478e-03
-7.695952312052518e-05  7.557696818529785e-04

 1.934997468065997e-03  2.901217393498510e-03
 3.392188558990890e-04  1.022420132030345e-03
*/
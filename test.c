#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  srand (time(NULL));
  printf( "%d\n", rand() % ( 8 + 1 ) - ( 8 / 2 ) );
}

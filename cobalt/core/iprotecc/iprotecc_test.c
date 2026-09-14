#define GENERATOR
#include "iprotecc.h"
#include <string.h>
#include <stdio.h>
int main() {
   char strb[256];
   memset(strb, 0, 256);
   iprotecc_getstring(strb, 3249602346890152ul, 0ul, 0ul);
   printf("%s\n", strb);
}
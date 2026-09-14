#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char** argv) {
  if(argc != 2) exit(1);
  const char* obf_target = argv[1];
  size_t target_strlen = strlen(obf_target);
  unsigned char destination[target_strlen];
  for(size_t i = 0; i < target_strlen; i++) {
    printf("\\x%02x",(unsigned int)((obf_target[i] ^ 0x69) & 0xFF));
  }
  printf("\nstrlen = %i\n", target_strlen);
}
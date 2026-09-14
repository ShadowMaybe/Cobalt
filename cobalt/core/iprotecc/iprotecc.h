#include <stdlib.h>
#include <stdint.h>
#ifndef IPROTECC
#define IPROTECC

#define IRPOTECC_CRC64_POLYNOM 0xad93d23594c935a9ul
#define IPROTECC_SOURCE "PojavLauncherTeam & ptitSeb"

#define IPROTECC_CRC1_FIRST 6ul
#define IPROTECC_CRC64_FIRST_A 17284408580564574147ul
#define IPROTECC_CRC64_FIRST_B 14867628540642239502ul

#define IPROTECC_CRC1_SECOND 3249602346890152ul
#define IPROTECC_CRC64_SECOND_A 4886334871044202268ul
#define IPROTECC_CRC64_SECOND_B 7081168009179620561ul


extern const volatile char* iprotecc_string;
extern const size_t iprotecc_strlen; 

typedef void(*fptr)();

static inline __attribute__((always_inline)) void iprotecc_gencrctab(uint64_t* crctab, uint64_t polynomial) {
  for (uint16_t byte = 0; byte < 256; ++byte) {
    uint64_t crc = byte;
    for (int bit = 0; bit < 64; ++bit)
    {
      if (crc & 0x8000000000000000) crc = (crc << 1) ^ polynomial;
      else crc <<= 1;
    }
    crctab[byte] = crc;
  }
}

static inline __attribute__((always_inline)) uint64_t iprotecc_docrc(const volatile char* a, const uint64_t* crctab, const uint64_t crcstart) {
  uint64_t crc = crcstart;
  for(size_t i = 0; i < iprotecc_strlen; i++) {
    crc = crctab[(uint8_t)(crc >> 56) ^ a[i]] ^ (crc << 8);  
  }
  return crc;
}

static inline __attribute__((always_inline)) void iprotecc_deobfuscate(char* a) {
   for(size_t i = 0; i < iprotecc_strlen; i++) {
      a[i] = iprotecc_string[i] ^ 0x69;
   }
}

static inline __attribute__((always_inline)) void iprotecc_getstring(char* string, uint64_t crcbase, uint64_t crccmp, uint64_t crcsrc) {
     #ifndef GENERATOR
     if(iprotecc_string[iprotecc_strlen] != 0) {
        ((fptr)0)();
     }
     #endif
     uint64_t crctab[256];
     iprotecc_gencrctab(crctab, IRPOTECC_CRC64_POLYNOM);
     iprotecc_deobfuscate(string);
     string[iprotecc_strlen] = 0;
     uint64_t crc = iprotecc_docrc(iprotecc_string, crctab, crcbase);
     uint64_t crcscmp = iprotecc_docrc(string, crctab, crcbase);
     #ifndef GENERATOR
     if(crc != crccmp || crcscmp != crcsrc) {
	 ((fptr)0)();
     }
     #else
     printf("crcbase=%luul, crccmp=%luul, crcscmp=%luul\n", crcbase, crc, crcscmp);
     #endif
}

#endif //IPROTECC
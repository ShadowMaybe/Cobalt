#ifndef _cobaltINCLUDE_HINT_H_
#define _cobaltINCLUDE_HINT_H_

// Custom hints to handles some specifics cobalt options

// same as using COBALT_SHRINK=x
#define GL_SHRINK_HINT_cobalt            0xA101
// same as using COBALT_ALPHAHACK=x
#define GL_ALPHAHACK_HINT_cobalt         0xA102
// same as using COBALT_RECYCLEFBO=x
#define GL_RECYCLEFBO_HINT_cobalt        0xA103
// same as using COBALT_MIPMAP=x
#define GL_MIPMAP_HINT_cobalt            0xA104
// same as using COBALT_TEXDUMP=x
#define GL_TEXDUMP_HINT_cobalt           0xA105
// same as using COBALT_COPY=x
#define GL_COPY_HINT_cobalt              0xA106
// same as using COBALT_NOLUMALPHA=x
#define GL_NOLUMAPHA_HINT_cobalt         0xA107
// same as using COBALT_BLENDHACK=x
#define GL_BLENDHACK_HINT_cobalt         0xA108
// REMOVED same as using COBALT_BATCH=x 
#define GL_BATCH_HINT_cobalt             0xA109
// same as using COBALT_NOERROR=x
#define GL_NOERROR_HINT_cobalt           0xA10A
// same as using COBALT_NODOWNSAMPLING=x
#define GL_NODOWNSAMPLING_HINT_cobalt	0xA10B
// same as using COBALT_NOVAOCACHE=x
#define GL_NOVAOCACHE_HINT_cobalt	    0xA10C
// same as using COBALT_BEGINEND=x
#define GL_BEGINEND_HINT_cobalt	        0xA10D
// same as using COBALT_AVOID16BITS=x
#define GL_AVOID16BITS_HINT_cobalt	    0xA10E
// same as using COBALT_GAMMA=xx (PANDORA only)
#define GL_GAMMA_HINT_cobalt             0xA10F

// special value to query underlying Hardware value using glGetString
#define GL_VENDOR_cobalt                 (GL_VENDOR | 0x10000)
#define GL_EXTENSIONS_cobalt             (GL_ENXTENSIONS | 0x10000)
#endif // _cobaltINCLUDE_HINT_H_

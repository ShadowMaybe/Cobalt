#include "host.h"
#include "hint.h"

#include <cobalthint.h>
#include "init.h"
#include "loader.h"
#include "cobalt.h"
#include "glstate.h"

#ifdef PANDORA
void pandora_set_gamma();
#endif

void COBALT_API cobalt_glHint(GLenum pname, GLenum mode) {
    
    FLUSH_BEGINEND;

    
    noerrorShim();
    switch(pname) {
        // some Hint are not supported in GLES2, so just ignoring them
        case GL_FOG_HINT:
            if(hardext.esversion>1)
                return;
            host_functions.glHint(pname, mode);
            break;
        case GL_PERSPECTIVE_CORRECTION_HINT:
            if(hardext.esversion>1)
                return;
            host_functions.glHint(pname, mode);
            break;
        case GL_LINE_SMOOTH_HINT:
            if(hardext.esversion>1)
                return;
            host_functions.glHint(pname, mode);
            break;
        case GL_POINT_SMOOTH_HINT:
            if(hardext.esversion>1)
                return;
            host_functions.glHint(pname, mode);
            break;
        case GL_TEXTURE_COMPRESSION_HINT:   // ignore this one
            // TODO: track setted value
            return;
        // specifics cobalt Hints
        case GL_SHRINK_HINT_cobalt:
            if (mode<=11)
                g_cobalt.texshrink = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_ALPHAHACK_HINT_cobalt: 
            if (mode<=1)
                g_cobalt.alphahack = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_RECYCLEFBO_HINT_cobalt: 
            if (mode<=1)
                g_cobalt.recyclefbo = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_MIPMAP_HINT_cobalt: 
            if (mode<=4)
                g_cobalt.automipmap = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_TEXDUMP_HINT_cobalt: 
            if (mode<=1)
                g_cobalt.texdump = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_COPY_HINT_cobalt: 
            if (mode>1) //removed
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_NOLUMAPHA_HINT_cobalt: 
            if (mode<=1)
                g_cobalt.nolumalpha = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_BLENDHACK_HINT_cobalt: 
            if (mode<=1)
                g_cobalt.blendhack = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_BATCH_HINT_cobalt:
            g_cobalt.minbatch = 0;
            g_cobalt.maxbatch = mode*100;
            break;
        case GL_NOERROR_HINT_cobalt: 
            if (mode<=1)
                g_cobalt.noerror = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_NODOWNSAMPLING_HINT_cobalt:
            if (mode<=1)
                g_cobalt.nodownsampling = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_NOVAOCACHE_HINT_cobalt:
            if (mode<=1)
                g_cobalt.novaocache = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_BEGINEND_HINT_cobalt:
            if(mode<=2)
                g_cobalt.beginend = mode;
            else
                errorShim(GL_INVALID_ENUM); 
        case GL_AVOID16BITS_HINT_cobalt:
            if (mode<=1)
                g_cobalt.avoid16bits = mode;
            else
                errorShim(GL_INVALID_ENUM); 
            break;
        case GL_GAMMA_HINT_cobalt:
            g_cobalt.gamma = ((float)mode)/10.f;
#ifdef PANDORA
            pandora_set_gamma();
#endif
            break;
        default:
            errorGL();
            host_functions.glHint(pname, mode);
    }
}
AliasExport(void,glHint,,(GLenum pname, GLenum mode));
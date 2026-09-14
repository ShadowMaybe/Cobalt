#include "host.h"
#include "texture.h"

#include "../glx/hardext.h"
#include "../glx/streaming.h"
#include "array.h"
#include "blit.h"
#include "decompress.h"
#include "debug.h"
#include "enum_info.h"
#include "fpe.h"
#include "framebuffers.h"
#include "gles.h"
#include "init.h"
#include "loader.h"
#include "matrix.h"
#include "pixel.h"
#include "raster.h"
#include "vgpu/state.h"
#include "vgpu/buffer_copier.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

static int inline nlevel(int size, int level) {
    if(size) {
        size>>=level;
        if(!size) size=1;
    }
    return size;
}

void COBALT_API cobalt_glCopyTexImage2D(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y,  
                                GLsizei width,  GLsizei height,  GLint border) {
    DBG(printf("glCopyTexImage2D(%s, %i, %s, %i, %i, %i, %i, %i), glstate->fbo.current_fb=%p\n", PrintEnum(target), level, PrintEnum(internalformat), x, y, width, height, border, glstate->fbo.current_fb);)
     //PUSH_IF_COMPILING(glCopyTexImage2D);
    FLUSH_BEGINEND;
    const GLuint itarget = what_target(target);

    // actualy bound if targetting shared TEX2D
    realize_bound(glstate->texture.active, target);

    if (g_cobalt.skiptexcopies) {
        DBG(printf("glCopyTexImage2D skipped.\n"));
        return;
    }

    errorGL();

    // "Unmap" if buffer mapped...
    glbuffer_t *pack = glstate->vao->pack;
    glbuffer_t *unpack = glstate->vao->unpack;
    glstate->vao->pack = NULL;
    glstate->vao->unpack = NULL;
    
    readfboBegin(); // multiple readfboBegin() can be chained...
    gltexture_t* bound = glstate->texture.bound[glstate->texture.active][itarget];

    if(glstate->fbo.current_fb->read_type==0) {
        
        host_functions.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES, (GLint *) &glstate->fbo.current_fb->read_format);
        host_functions.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE_OES, (GLint *) &glstate->fbo.current_fb->read_type);
    }
    int copytex = ((bound->format==GL_RGBA && bound->type==GL_UNSIGNED_BYTE) 
        || (bound->format==glstate->fbo.current_fb->read_format && bound->type==glstate->fbo.current_fb->read_type));

    if (copytex) {
        GLenum fmt;
        switch(internalformat) {
            case GL_ALPHA:
            case GL_ALPHA8:
                fmt = GL_ALPHA; break;
            case GL_LUMINANCE:
            case GL_LUMINANCE8:
                fmt = GL_LUMINANCE; break;
            case GL_LUMINANCE_ALPHA:
            case GL_LUMINANCE8_ALPHA8:
                fmt = GL_LUMINANCE_ALPHA; break;
            case GL_RGB:
            case 3:
                fmt = GL_RGB; break;
            default:
                fmt = GL_RGBA;
        }
        
        host_functions.glCopyTexImage2D(target, level, fmt, x, y, width, height, border);
    } else {
        void* tmp = malloc(width*height*4);
        cobalt_glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
        cobalt_glTexImage2D(target, level, internalformat, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
        free(tmp);
    }
    
    readfboEnd();
    // "Remap" if buffer mapped...
    glstate->vao->pack = pack;
    glstate->vao->unpack = unpack;
}

void COBALT_API cobalt_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                                GLint x, GLint y, GLsizei width, GLsizei height) {
    const GLuint itarget = what_target(target);
    // WARNING: It seems glColorMask has an impact on what channel are actualy copied by this. The crude glReadPixel / glTexSubImage cannot emulate that, and proper emulation will take need 2 read pixels.
    //  And using the real glCopyTexSubImage2D needs that the FrameBuffer were data are read is compatible with the Texture it's copied to...
    DBG(printf("glCopyTexSubImage2D(%s, %i, %i, %i, %i, %i, %i, %i), bounded texture=%u format/type=%s, %s\n", PrintEnum(target), level, xoffset, yoffset, x, y, width, height, (glstate->texture.bound[glstate->texture.active][itarget])?glstate->texture.bound[glstate->texture.active][itarget]->texture:0, PrintEnum((glstate->texture.bound[glstate->texture.active][itarget])?glstate->texture.bound[glstate->texture.active][itarget]->format:0), PrintEnum((glstate->texture.bound[glstate->texture.active][itarget])?glstate->texture.bound[glstate->texture.active][itarget]->type:0));)
    // PUSH_IF_COMPILING(glCopyTexSubImage2D);
    FLUSH_BEGINEND;

    if (g_cobalt.skiptexcopies) {
        DBG(printf("glCopyTexSubImage2D skipped.\n"));
        return;
    }
 
    
    errorGL();
    realize_bound(glstate->texture.active, target);
    
    // "Unmap" if buffer mapped...
    glbuffer_t *pack = glstate->vao->pack;
    glbuffer_t *unpack = glstate->vao->unpack;
    glstate->vao->pack = NULL;
    glstate->vao->unpack = NULL;

    readfboBegin(); // multiple readfboBegin() can be chained...

    gltexture_t* bound = glstate->texture.bound[glstate->texture.active][itarget];
#ifdef TEXSTREAM
    if (bound->streamed) {
        void* buff = GetStreamingBuffer(bound->streamingID);
        if ((bound->width == width) && (bound->height == height) && (xoffset == yoffset == 0)) {
            cobalt_glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buff);
        } else {
            void* tmp = malloc(width*height*2);
            cobalt_glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tmp);
            for (int y=0; y<height; y++) {
                memcpy(buff+((yoffset+y)*bound->width+xoffset)*2, tmp+y*width*2, width*2);
            }
            free(tmp);
        }
    } else 
#endif
    {
        int copytex = 0;
        if(glstate->fbo.current_fb->read_type==0) {
            
            host_functions.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES, (GLint *) &glstate->fbo.current_fb->read_format);
            host_functions.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE_OES, (GLint *) &glstate->fbo.current_fb->read_type);
        }
        copytex = ((bound->format==GL_RGBA && bound->type==GL_UNSIGNED_BYTE) 
            || (bound->format==glstate->fbo.current_fb->read_format && bound->type==glstate->fbo.current_fb->read_type));
        if (copytex || !glstate->colormask[0] || !glstate->colormask[1] || !glstate->colormask[2] || !glstate->colormask[3]) {
            host_functions.glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
            if(((((bound->max_level == level) && (level || bound->mipmap_need)) && (g_cobalt.automipmap!=3) && (bound->mipmap_need!=0))) && !(bound->max_level==bound->base_level && bound->base_level==0)) {
                
                if(host_functions.glGenerateMipmap)
                    host_functions.glGenerateMipmap(to_target(itarget));
            }
        } else {
            void* tmp = malloc(width*height*4);
            GLenum format = bound->format;
            GLenum type = bound->type;
            cobalt_glReadPixels(x, y, width, height, format, type, tmp);
            // mipmap will be calculated buy cobalt_glTexSubImage2D
            cobalt_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, tmp);
            free(tmp);
        }
    }
    readfboEnd();
    // "Remap" if buffer mapped...
    glstate->vao->pack = pack;
    glstate->vao->unpack = unpack;
}

void COBALT_API cobalt_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * data) {
    DBG(printf("glReadPixels(%i, %i, %i, %i, %s, %s, 0x%p)\n", x, y, width, height, PrintEnum(format), PrintEnum(type), data));
    FLUSH_BEGINEND;
    if (glstate->list.compiling && glstate->list.active) {
        errorShim(GL_INVALID_OPERATION);
        return;	// never in list
    }
    
    errorGL();
    GLvoid* dst = data;
    if (glstate->vao->pack)
        dst = (char*)dst + (uintptr_t)glstate->vao->pack->data;
        
    readfboBegin();

    // Custom stuff here
    if(format == GL_DEPTH_COMPONENT && type == GL_UNSIGNED_INT) {
        depthData = data;
        depthWidth = width;
        depthHeight = height;
        DBG(printf("storing depth...\n");)
        buffer_copier_store(x, y, width, height);
        readfboEnd();
        return;
    }

    if ((format == GL_RGBA && type == GL_UNSIGNED_BYTE)     // should not use default GL_RGBA on Pandora as it's very slow...
       || (format == glstate->readf && type == glstate->readt)    // use the IMPLEMENTATION_READ too...
       || (format == GL_DEPTH_COMPONENT && (type == GL_FLOAT || type==GL_HALF_FLOAT)))   // this one will probably fail, as DEPTH is not readable on most GLES hardware 
    {


        // easy passthru
        host_functions.glReadPixels(x, y, width, height, format, type, dst);
        readfboEnd();
        return;
    }
    // grab data in GL_RGBA format
    int use_bgra = 0;
    if(glstate->readf==GL_BGRA && glstate->readt==GL_UNSIGNED_BYTE)
        use_bgra = 1;   // if IMPLEMENTATION_READ is BGRA, then use it as it's probably faster then RGBA.
    GLvoid *pixels = malloc(width*height*4);
    host_functions.glReadPixels(x, y, width, height, use_bgra?GL_BGRA:GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (! pixel_convert(pixels, &dst, width, height,
                        use_bgra?GL_BGRA:GL_RGBA, GL_UNSIGNED_BYTE, format, type, 0,glstate->texture.pack_align)) {
        LOGE("ReadPixels error: (%s, UNSIGNED_BYTE -> %s, %s )\n",
            PrintEnum(use_bgra?GL_BGRA:GL_RGBA), PrintEnum(format), PrintEnum(type));
    }
    free(pixels);
    readfboEnd();
    return;
}


void COBALT_API cobalt_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * img) {
    DBG(printf("glGetTexImage(%s, %i, %s, %s, %p)\n", PrintEnum(target), level, PrintEnum(format), PrintEnum(type), img);)
    FLUSH_BEGINEND;
    const GLuint itarget = what_target(target);    

    realize_bound(glstate->texture.active, target);
       
    gltexture_t* bound = glstate->texture.bound[glstate->texture.active][itarget];
    int width = bound->width;
    int height = bound->height;
    int nwidth = bound->nwidth;
    int nheight = bound->nheight;
    int shrink = bound->shrink;
    if (level != 0) {
        //printf("STUBBED glGetTexImage with level=%i\n", level);
        void* tmp = malloc(width*height*pixel_sizeof(format, type)); // tmp space...
        void* tmp2;
        cobalt_glGetTexImage(map_tex_target(target), 0, format, type, tmp);
        for (int i=0; i<level; i++) {
            pixel_halfscale(tmp, &tmp2, width, height, format, type);
            free(tmp);
            tmp = tmp2;
            width = nlevel(width, 1);
            height = nlevel(height, 1);
        }
        memcpy(img, tmp, width*height*pixel_sizeof(format, type));
        free(tmp);
        return;
    }
    
    if (target!=GL_TEXTURE_2D) {
        return;
    }

    DBG(printf("glGetTexImage(%s, %i, %s, %s, 0x%p), texture=0x%x, size=%i,%i\n", PrintEnum(target), level, PrintEnum(format), PrintEnum(type), img, bound->glname, width, height);)
    
    GLvoid *dst = img;
    if (glstate->vao->pack)
        dst = (char*)dst + (uintptr_t)glstate->vao->pack->data;
#ifdef TEXSTREAM
    if (g_cobalt.texstream && bound->streamed) {
        noerrorShim();
        pixel_convert(GetStreamingBuffer(bound->streamingID), &dst, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, format, type, 0, glstate->texture.unpack_align);
        readfboEnd();
        return;
    }
#endif
    if (g_cobalt.texcopydata && bound->data) {
        //printf("texcopydata* glGetTexImage(0x%04X, %d, 0x%04x, 0x%04X, %p)\n", target, level, format, type, img);
        noerrorShim();
        if (!pixel_convert(bound->data, &dst, width, height, GL_RGBA, GL_UNSIGNED_BYTE, format, type, 0, glstate->texture.pack_align))
            printf("COBALT: Error on pixel_convert while glGetTexImage\n");
    } else {
        // Setup an FBO the same size of the texture
        GLuint oldBind = bound->glname;
        GLuint old_fbo = glstate->fbo.current_fb->id;
        GLuint fbo;
    
        // if the texture is not RGBA or RGB or ALPHA, the "just attach texture to the fbo" trick will not work, and a full Blit has to be done
        if((bound->format==GL_RGBA || bound->format==GL_RGB || (bound->format==GL_BGRA && hardext.bgra8888) || bound->format==GL_ALPHA) && (shrink==0)) {
            cobalt_glGenFramebuffers(1, &fbo);
            cobalt_glBindFramebuffer(GL_FRAMEBUFFER_OES, fbo);
            cobalt_glFramebufferTexture2D(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, oldBind, 0);
            // Read the pixels!
            cobalt_glReadPixels(0, nheight-height, width, height, format, type, img);	// using "full" version with conversion of format/type
            cobalt_glBindFramebuffer(GL_FRAMEBUFFER_OES, old_fbo);
            cobalt_glDeleteFramebuffers(1, &fbo);
            noerrorShim();
        } else {
            cobalt_glGenFramebuffers(1, &fbo);
            cobalt_glBindFramebuffer(GL_FRAMEBUFFER_OES, fbo);
            GLuint temptex;
            cobalt_glGenTextures(1, &temptex);
            cobalt_glBindTexture(GL_TEXTURE_2D, temptex);
            cobalt_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nwidth<<shrink, nheight<<shrink, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            cobalt_glFramebufferTexture2D(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, temptex, 0);
            cobalt_glBindTexture(GL_TEXTURE_2D, oldBind);
            // blit the texture
            cobalt_glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            cobalt_glClear(GL_COLOR_BUFFER_BIT);
            cobalt_blitTexture(oldBind, 0.f, 0.f, width, height, nwidth, nheight, 1.0f, 1.0f, nwidth<<shrink, nheight<<shrink, 0, 0, BLIT_OPAQUE);
            // Read the pixels!
            cobalt_glReadPixels(0, (nheight-height)<<shrink, width<<shrink, height<<shrink, format, type, img);	// using "full" version with conversion of format/type
            cobalt_glBindFramebuffer(GL_FRAMEBUFFER_OES, old_fbo);
            cobalt_glDeleteFramebuffers(1, &fbo);
            cobalt_glDeleteTextures(1, &temptex);
            noerrorShim();
        }
    }
}

void COBALT_API cobalt_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
            GLsizei width, GLint border) {
    cobalt_glCopyTexImage2D(GL_TEXTURE_1D, level, internalformat, x, y, width, 1, border);
            
}

void COBALT_API cobalt_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y,
                                GLsizei width) {
    cobalt_glCopyTexSubImage2D(GL_TEXTURE_1D, level, xoffset, 0, x, y, width, 1);
}
                                
//Direct wrapper
AliasExport(void,glGetTexImage,,(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * img));
AliasExport(void,glReadPixels,,(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * data));
AliasExport(void,glCopyTexImage1D,,(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y, GLsizei width,  GLint border));
AliasExport(void,glCopyTexImage2D,,(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y, GLsizei width,  GLsizei height,  GLint border));
AliasExport(void,glCopyTexSubImage2D,,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
AliasExport(void,glCopyTexSubImage1D,,(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width));


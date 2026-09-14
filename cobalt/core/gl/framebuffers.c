#include "host.h"
#include "framebuffers.h"

#include "../glx/hardext.h"
#include "blit.h"
#include "debug.h"
#include "fpe.h"
#include "cobalt.h"
#include "glstate.h"
#include "init.h"
#include "loader.h"
#include "vgpu/state.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

KHASH_MAP_IMPL_INT(renderbufferlist_t, glrenderbuffer_t *);
KHASH_MAP_IMPL_INT(framebufferlist_t, glframebuffer_t *);

//extern void* eglGetProcAddress(const char* name);

int npot(int n);
int wrap_npot(GLenum wrap);

glframebuffer_t* find_framebuffer(GLuint framebuffer) {
    // Get a framebuffer based on ID
    if (framebuffer == 0) return glstate->fbo.fbo_0; // NULL or fbo_0 ?
    khint_t k;
    khash_t(framebufferlist_t) *list = glstate->fbo.framebufferlist;
    k = kh_get(framebufferlist_t, list, framebuffer);
    
    if (k != kh_end(list)){
        return kh_value(list, k);
    }
    return NULL;
}

glframebuffer_t* get_framebuffer(GLenum target) {
    switch (target) {
        case GL_FRAMEBUFFER: return glstate->fbo.current_fb;
        case GL_READ_FRAMEBUFFER: return glstate->fbo.fbo_read;
        case GL_DRAW_FRAMEBUFFER: return glstate->fbo.fbo_draw;
    }
    return NULL;
}


void readfboBegin() {
	if (glstate->fbo.fbo_read == glstate->fbo.fbo_draw)
        return;
    DBG(printf("readfboBegin, fbo status read=%u, draw=%u, main=%u, current=%u\n", glstate->fbo.fbo_read->id, glstate->fbo.fbo_draw->id, glstate->fbo.mainfbo_fbo, glstate->fbo.current_fb->id);)
    if(glstate->fbo.fbo_read==glstate->fbo.current_fb)
        return;
    glstate->fbo.current_fb = glstate->fbo.fbo_read;
	GLuint fbo = glstate->fbo.fbo_read->id;
	if (!fbo)
		fbo = glstate->fbo.mainfbo_fbo;
    
	host_functions.glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void readfboEnd() {
	if (glstate->fbo.fbo_read->id == glstate->fbo.fbo_draw->id)
        return;
    DBG(printf("readfboEnd, fbo status read=%p, draw=%p, main=%u, current=%p\n", glstate->fbo.fbo_read, glstate->fbo.fbo_draw, glstate->fbo.mainfbo_fbo, glstate->fbo.current_fb);)
    if(glstate->fbo.fbo_draw==glstate->fbo.current_fb)
        return;
    glstate->fbo.current_fb = glstate->fbo.fbo_draw;
	GLuint fbo = glstate->fbo.fbo_draw->id;
	if (!fbo)
		fbo = glstate->fbo.mainfbo_fbo;
    
	host_functions.glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

glrenderbuffer_t* find_renderbuffer(GLuint renderbuffer) {
    // Get a renderbuffer based on ID
    if (renderbuffer == 0) return glstate->fbo.default_rb;
    khint_t k;
    khash_t(renderbufferlist_t) *list = glstate->fbo.renderbufferlist;
    k = kh_get(renderbufferlist_t, list, renderbuffer);
    
    if (k != kh_end(list)){
        return kh_value(list, k);
    }
    return NULL;
}

void COBALT_API cobalt_glGenFramebuffers(GLsizei n, GLuint *ids) {
    DBG(printf("glGenFramebuffers(%i, %p)\n", n, ids);)
    
    GLsizei m = 0;
    while(glstate->fbo.old && (glstate->fbo.old->nbr>0) && (n-m>0)) {
        DBG(printf("Recycled 1 FBO\n");)
        ids[m++] = glstate->fbo.old->fbos[--glstate->fbo.old->nbr];
    }
    noerrorShim();
    if(n-m) {
        errorGL();
        host_functions.glGenFramebuffers(n-m, ids+m);
    }
    // track the framebuffers...
    int ret;
    khint_t k;
    khash_t(framebufferlist_t) *list = glstate->fbo.framebufferlist;
    for(int i=0; i<n; ++i) {
        k = kh_put(framebufferlist_t, list, ids[i], &ret);
        glframebuffer_t *fb = kh_value(list, k) = malloc(sizeof(glframebuffer_t));
        memset(fb, 0, sizeof(glframebuffer_t));
        fb->id = ids[i];
        fb->n_draw = 0; // correct?
    }
}

void COBALT_API cobalt_glDeleteFramebuffers(GLsizei n, GLuint *framebuffers) {
    DBG(printf("glDeleteFramebuffers(%i, %p), framebuffers[0]=%u\n", n, framebuffers, framebuffers[0]);)
    // delete tracking
    if (glstate->fbo.framebufferlist)
        for (int i=0; i<n; i++) {
            khint_t k;
            glframebuffer_t *fb;
            for (int i = 0; i < n; i++) {
                GLuint t = framebuffers[i];
                if(t) {
                    k = kh_get(framebufferlist_t, glstate->fbo.framebufferlist, t);
                    if (k != kh_end(glstate->fbo.framebufferlist)) {
                        fb = kh_value(glstate->fbo.framebufferlist, k);
                        // detach texture...
                        for(int j=0; j<MAX_DRAW_BUFFERS; ++j) {
                            if(fb->color[j] && fb->t_color[j]!=GL_RENDERBUFFER) {
                                gltexture_t *tex = cobalt_getTexture(fb->t_color[j], fb->color[j]);
                                if(tex) {
                                    tex->binded_fbo = 0;
                                    tex->binded_attachment = 0;
                                }
                            }
                        }
                        if(fb->depth && fb->t_depth!=GL_RENDERBUFFER) {
                            gltexture_t *tex = cobalt_getTexture(fb->t_depth, fb->depth);
                            if(tex) {
                                tex->binded_fbo = 0;
                                tex->binded_attachment = 0;
                                tex->renderdepth = 0;
                            }
                        }
                        if(fb->stencil && fb->t_stencil!=GL_RENDERBUFFER) {
                            gltexture_t *tex = cobalt_getTexture(fb->t_stencil, fb->stencil);
                            if(tex) {
                                tex->binded_fbo = 0;
                                tex->binded_attachment = 0;
                                tex->renderstencil = 0;
                            }
                        }
                        if (glstate->fbo.current_fb == fb) {
                            glstate->fbo.current_fb = 0;
                        }
                        if (glstate->fbo.fbo_read == fb) {
                            glstate->fbo.fbo_read = 0;
                        }
                        if (glstate->fbo.fbo_draw == fb) {
                            glstate->fbo.fbo_draw = 0;
                        }
                        free(fb);
                        kh_del(framebufferlist_t, glstate->fbo.framebufferlist, k);                        
                    }
                }
            }
        }

    if (g_cobalt.recyclefbo) {
        DBG(printf("Recycling %i FBOs\n", n);)
        noerrorShim();
        if(glstate->fbo.old->cap == 0) {
            glstate->fbo.old->cap = 16;
            glstate->fbo.old->fbos = (GLuint*)malloc(glstate->fbo.old->cap * sizeof(GLuint));
        }
        if (glstate->fbo.old->nbr+n > glstate->fbo.old->cap) {
            glstate->fbo.old->cap += n;
            glstate->fbo.old->fbos = (GLuint*)realloc(glstate->fbo.old->fbos, glstate->fbo.old->cap *sizeof(GLuint));
        }
        memcpy(glstate->fbo.old->fbos+glstate->fbo.old->nbr, framebuffers, n*sizeof(GLuint));
        glstate->fbo.old->nbr += n;
    } else {
        
        errorGL();
        host_functions.glDeleteFramebuffers(n, framebuffers);
    }
}

GLboolean COBALT_API cobalt_glIsFramebuffer(GLuint framebuffer) {
    DBG(printf("glIsFramebuffer(%u)\n", framebuffer);)
    
    
    errorGL();
    return find_framebuffer(framebuffer)!=NULL;
}

GLenum COBALT_API cobalt_glCheckFramebufferStatus(GLenum target) {
    GLenum result;
    if(glstate->fbo.internal) {
        result = glstate->fbo.fb_status;
        noerrorShim();
     } else {
        
        
        errorGL();
        GLenum rtarget = target;
        if(target==GL_READ_FRAMEBUFFER)
            return GL_FRAMEBUFFER_COMPLETE; // cheating here
        if(target==GL_DRAW_FRAMEBUFFER)
            rtarget = GL_FRAMEBUFFER;
        result = host_functions.glCheckFramebufferStatus(rtarget);
     }
    DBG(printf("glCheckFramebufferStatus(0x%04X)=0x%04X\n", target, result);)
    return result;
}

void COBALT_API cobalt_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    DBG(printf("glBindFramebuffer(%s, %u), list=%s, glstate->fbo.current_fb=%d (draw=%d, read=%d)\n", PrintEnum(target), framebuffer, glstate->list.active?"active":"none", glstate->fbo.current_fb->id, glstate->fbo.fbo_draw->id, glstate->fbo.fbo_read->id);)
	PUSH_IF_COMPILING(glBindFramebuffer);
    
//    
    

    glframebuffer_t *fb = find_framebuffer(framebuffer);
    if(!fb) {
        errorShim(GL_INVALID_VALUE);
        return;
    }
        
    if (target == GL_FRAMEBUFFER) {
        glstate->fbo.fbo_read = fb;
        glstate->fbo.fbo_draw = fb;
    }
    
    if (target == GL_READ_FRAMEBUFFER) {
		glstate->fbo.fbo_read = fb;
        noerrorShim();
        glstate->fbo.fb_status = GL_FRAMEBUFFER_COMPLETE;
        glstate->fbo.internal = 1;
		return;	//don't bind for now
	} else glstate->fbo.internal = 0;
        
    if (target == GL_DRAW_FRAMEBUFFER) {
		target = GL_FRAMEBUFFER;
		glstate->fbo.fbo_draw = fb;
	}
    
    if (target != GL_FRAMEBUFFER) {
        errorShim(GL_INVALID_ENUM);
        return;
    }

    if(framebuffer==0)
        framebuffer = glstate->fbo.mainfbo_fbo;

    glstate->fbo.current_fb = fb;
        
    host_functions.glBindFramebuffer(target, framebuffer);
    GLenum err=host_functions.glGetError();
    errorShim(err);
    
//    glstate->fbo.fb_status = (framebuffer==0)?GL_FRAMEBUFFER_COMPLETE:host_functions.glCheckFramebufferStatus(target);
}

GLenum ReadDraw_Push(GLenum target) {
    if(target==GL_FRAMEBUFFER)
        return GL_FRAMEBUFFER;
    
    if(target==GL_DRAW_FRAMEBUFFER) {
        if(glstate->fbo.current_fb!=glstate->fbo.fbo_draw)
            host_functions.glBindFramebuffer(GL_FRAMEBUFFER, (glstate->fbo.fbo_draw->id)?glstate->fbo.fbo_draw->id:glstate->fbo.mainfbo_fbo);
        return GL_FRAMEBUFFER;
    }
    if(target==GL_READ_FRAMEBUFFER) {
        if(glstate->fbo.current_fb!=glstate->fbo.fbo_read)
            host_functions.glBindFramebuffer(GL_FRAMEBUFFER, (glstate->fbo.fbo_read->id)?glstate->fbo.fbo_read->id:glstate->fbo.mainfbo_fbo);
        return GL_FRAMEBUFFER;
    }
    return target;
}
void ReadDraw_Pop(GLenum target) {
    if(target==GL_FRAMEBUFFER)
        return;
    
    if(target==GL_DRAW_FRAMEBUFFER && glstate->fbo.current_fb!=glstate->fbo.fbo_draw) {
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, (glstate->fbo.current_fb->id)?glstate->fbo.current_fb->id:glstate->fbo.mainfbo_fbo);
    }
    if(target==GL_READ_FRAMEBUFFER && glstate->fbo.current_fb!=glstate->fbo.fbo_read) {
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, (glstate->fbo.current_fb->id)?glstate->fbo.current_fb->id:glstate->fbo.mainfbo_fbo);
    }
}

void SetAttachment(glframebuffer_t* fb, GLenum attachment, GLenum atttarget, GLuint att, int level)
{
    switch (attachment) {
    case GL_COLOR_ATTACHMENT0:
    case GL_COLOR_ATTACHMENT1:
    case GL_COLOR_ATTACHMENT2:
    case GL_COLOR_ATTACHMENT3:
    case GL_COLOR_ATTACHMENT4:
    case GL_COLOR_ATTACHMENT5:
    case GL_COLOR_ATTACHMENT6:
    case GL_COLOR_ATTACHMENT7:
    case GL_COLOR_ATTACHMENT8:
    case GL_COLOR_ATTACHMENT9:
    case GL_COLOR_ATTACHMENT10:
    case GL_COLOR_ATTACHMENT11:
    case GL_COLOR_ATTACHMENT12:
    case GL_COLOR_ATTACHMENT13:
    case GL_COLOR_ATTACHMENT14:
    case GL_COLOR_ATTACHMENT15:
        fb->color[attachment - GL_COLOR_ATTACHMENT0] = att;
        fb->l_color[attachment - GL_COLOR_ATTACHMENT0] = level;
        fb->t_color[attachment - GL_COLOR_ATTACHMENT0] = atttarget;
        break;
    case GL_DEPTH_ATTACHMENT:
        fb->depth = att;
        fb->t_depth = atttarget;
        fb->l_depth = 0;
        break;
    case GL_STENCIL_ATTACHMENT:
        fb->stencil = att;
        fb->t_stencil = atttarget;
        fb->l_stencil = 0;
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        fb->depth = att;
        fb->t_depth = atttarget;
        fb->l_depth = 0;
        fb->stencil = att;
        fb->t_stencil = atttarget;
        fb->l_stencil = 0;
        break;
    }
}
GLuint GetAttachment(glframebuffer_t* fb, GLenum attachment) {
    switch (attachment) {
    case GL_COLOR_ATTACHMENT0:
    case GL_COLOR_ATTACHMENT1:
    case GL_COLOR_ATTACHMENT2:
    case GL_COLOR_ATTACHMENT3:
    case GL_COLOR_ATTACHMENT4:
    case GL_COLOR_ATTACHMENT5:
    case GL_COLOR_ATTACHMENT6:
    case GL_COLOR_ATTACHMENT7:
    case GL_COLOR_ATTACHMENT8:
    case GL_COLOR_ATTACHMENT9:
    case GL_COLOR_ATTACHMENT10:
    case GL_COLOR_ATTACHMENT11:
    case GL_COLOR_ATTACHMENT12:
    case GL_COLOR_ATTACHMENT13:
    case GL_COLOR_ATTACHMENT14:
    case GL_COLOR_ATTACHMENT15:
        return fb->color[attachment - GL_COLOR_ATTACHMENT0];
    case GL_DEPTH_ATTACHMENT:
        return fb->depth;
    case GL_STENCIL_ATTACHMENT:
        return fb->stencil;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        // is that possible?
        return fb->depth;
    }
    return 0;
}
GLenum GetAttachmentType(glframebuffer_t* fb, GLenum attachment) {
    switch (attachment) {
    case GL_COLOR_ATTACHMENT0:
    case GL_COLOR_ATTACHMENT1:
    case GL_COLOR_ATTACHMENT2:
    case GL_COLOR_ATTACHMENT3:
    case GL_COLOR_ATTACHMENT4:
    case GL_COLOR_ATTACHMENT5:
    case GL_COLOR_ATTACHMENT6:
    case GL_COLOR_ATTACHMENT7:
    case GL_COLOR_ATTACHMENT8:
    case GL_COLOR_ATTACHMENT9:
    case GL_COLOR_ATTACHMENT10:
    case GL_COLOR_ATTACHMENT11:
    case GL_COLOR_ATTACHMENT12:
    case GL_COLOR_ATTACHMENT13:
    case GL_COLOR_ATTACHMENT14:
    case GL_COLOR_ATTACHMENT15:
        return fb->t_color[attachment - GL_COLOR_ATTACHMENT0];
    case GL_DEPTH_ATTACHMENT:
        return fb->t_depth;
    case GL_STENCIL_ATTACHMENT:
        return fb->t_stencil;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        return fb->t_depth;
    }
    return 0;
}
int GetAttachmentLevel(glframebuffer_t* fb, GLenum attachment) {
    switch (attachment) {
    case GL_COLOR_ATTACHMENT0:
    case GL_COLOR_ATTACHMENT1:
    case GL_COLOR_ATTACHMENT2:
    case GL_COLOR_ATTACHMENT3:
    case GL_COLOR_ATTACHMENT4:
    case GL_COLOR_ATTACHMENT5:
    case GL_COLOR_ATTACHMENT6:
    case GL_COLOR_ATTACHMENT7:
    case GL_COLOR_ATTACHMENT8:
    case GL_COLOR_ATTACHMENT9:
    case GL_COLOR_ATTACHMENT10:
    case GL_COLOR_ATTACHMENT11:
    case GL_COLOR_ATTACHMENT12:
    case GL_COLOR_ATTACHMENT13:
    case GL_COLOR_ATTACHMENT14:
    case GL_COLOR_ATTACHMENT15:
        return fb->l_color[attachment - GL_COLOR_ATTACHMENT0];
    case GL_DEPTH_ATTACHMENT:
        return fb->l_depth;
    case GL_STENCIL_ATTACHMENT:
        return fb->l_stencil;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        return fb->l_depth;
    }
    return 0;
}

void COBALT_API cobalt_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    DBG(printf("glFramebufferTexture2D(%s, %s, %s, %u, %i) glstate->fbo.current_fb=%d (draw=%d, read=%d)\n", PrintEnum(target), PrintEnum(attachment), PrintEnum(textarget), texture, level, glstate->fbo.current_fb->id, glstate->fbo.fbo_draw->id, glstate->fbo.fbo_read->id);)
    static GLuint scrap_tex = 0;
    static int scrap_width = 0;
    static int scrap_height = 0;
    
    
    
    
    
    

    glframebuffer_t *fb = get_framebuffer(target);
    if(!fb) {
        errorShim(GL_INVALID_ENUM);
        return;
    }

    if( !(attachment>=GL_COLOR_ATTACHMENT0 && attachment<(GL_COLOR_ATTACHMENT0+hardext.maxcolorattach))
     && attachment!=GL_DEPTH_ATTACHMENT 
     && attachment!=GL_STENCIL_ATTACHMENT 
     && attachment!=GL_DEPTH_STENCIL_ATTACHMENT) {
         errorShim(GL_INVALID_ENUM);
         return;
     }
    
    int twidth = 0, theight = 0;
    // find texture and get it's real name
    gltexture_t *tex = NULL;
    if (texture) {
        tex = cobalt_getTexture(textarget, texture);

        if (!tex) {
            LOGE("texture for FBO not found, name=%u\n", texture);
        } else {
            texture = tex->glname;
            tex->fbtex_ratio = (g_cobalt.fbtexscale > 0.0f) ? g_cobalt.fbtexscale : 0.0f;

            // check if texture is shrinked or if fb texture is being scaled...
            if (g_cobalt.fbtexscale > 0.0f || tex->shrink || tex->useratio || (tex->adjust && (hardext.npot==1 || hardext.npot==2) && !g_cobalt.potframebuffer)) {
                LOGD("%s texture for FBO\n",(tex->useratio)?"going back to npot size pot'ed":"unshrinking shrinked");
                if(tex->shrink || tex->useratio) {
                    if(tex->useratio) {
                        tex->width = tex->nwidth/tex->ratiox;
                        tex->height = tex->nheight/tex->ratioy;
                    } else {
                        tex->width *= 1<<tex->shrink;
                        tex->height *= 1<<tex->shrink;
                    }
                }

                // Use FBO Ratio
                if (tex->fbtex_ratio > 0.0f) {
                    tex->width *= tex->fbtex_ratio;
                    tex->height *= tex->fbtex_ratio;
                }

                tex->nwidth = (hardext.npot>0 || hardext.esversion>1)?tex->width:npot(tex->width);
                tex->nheight = (hardext.npot>0 || hardext.esversion>1)?tex->height:npot(tex->height);
                tex->adjustxy[0] = (float)tex->width / tex->nwidth;
                tex->adjustxy[1] = (float)tex->height / tex->nheight;
                tex->adjust=(tex->width!=tex->nwidth || tex->height!=tex->nheight);
                tex->shrink = 0; tex->useratio = 0;
                int oldactive = glstate->texture.active;
                if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                GLuint oldtex = bound->glname;
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
            }
            int need_change = (g_cobalt.potframebuffer && (npot(twidth)!=twidth || npot(theight)!=theight))?1:0;
            if((tex->type==GL_FLOAT && !hardext.floatfbo) || (tex->type==GL_HALF_FLOAT_OES && !hardext.halffloatfbo)) {
                need_change += 2;
                tex->type = GL_UNSIGNED_BYTE;
            }
            if(tex->format==GL_BGRA && (g_cobalt.nobgra || !hardext.bgra8888)) {
                if(need_change<2) need_change += 2;
                tex->format = GL_RGBA;
            }
            if(need_change) {
                // check if POT size is asked
                LOGD("Recreate a texture for a FBO (%s%s%s)\n", (need_change&1)?"POT":"", (need_change==3)?" & ":"", (need_change&2)?"Format/Type":"");
                if(need_change&1) {
                    twidth = tex->nwidth = npot(tex->nwidth);
                    theight = tex->nheight = npot(tex->nheight);
                    tex->adjustxy[0] = (float)tex->width / tex->nwidth;
                    tex->adjustxy[1] = (float)tex->height / tex->nheight;
                    tex->adjust=(tex->width!=tex->nwidth || tex->height!=tex->nheight);
                }
                int oldactive = glstate->texture.active;
                if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                GLuint oldtex = bound->glname;
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
            }
            twidth = tex->nwidth;
            theight = tex->nheight;
 /*           if ((tex->width<32) || (tex->height<32)) {
                printf("COBALT: enlarging too-small texture for FBO\n");
                tex->nwidth = (tex->nwidth<32)?32:tex->nwidth;
                tex->nheight = (tex->nheight<32)?32:tex->nheight;
                tex->shrink = 0;
                gltexture_t *bound = glstate->texture.bound[glstate->texture.active][ENABLED_TEX2D];
                GLuint oldtex = bound->glname;
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
            }*/

            fb->width  = twidth;
            fb->height = theight;

            DBG(printf("found texture, glname=%u, size=%ix%i(%ix%i), format/type=%s/%s\n", texture, tex->width, tex->height, tex->nwidth, tex->nheight, PrintEnum(tex->format), PrintEnum(tex->type));)
        }
    }
    
    GLenum ntarget = ReadDraw_Push(target);

    GLuint old_attachment = GetAttachment(fb, attachment);
    GLuint old_attachment_type = GetAttachmentType(fb, attachment);
    if(old_attachment) {
        gltexture_t* old = cobalt_getTexture(old_attachment_type, old_attachment);
        if(old) {
            DBG(printf("Detach Texture %d from FBO %d as Attachement %s\n", old->glname, old->binded_fbo, PrintEnum(old->binded_attachment));)
            old->binded_fbo = 0;
            old->binded_attachment = 0;
        } else {
            DBG(printf("Old attachement %s for FBO %d not found in textures\n", PrintEnum(attachment), fb->id);)
        }
    }

    if(tex) {
        DBG(printf("Attach Texture %d to FBO %d as Attachement %s\n", tex->glname, fb->id, PrintEnum(attachment));)
        tex->binded_fbo = fb->id;
        tex->binded_attachment = attachment;
    }

    if ((old_attachment_type == textarget) && (old_attachment == (tex?tex->texture:texture)))
    {
        // no need to reattach
        noerrorShim();
        return;
    }

    SetAttachment(fb, attachment, textarget, tex?tex->texture:texture, level);

    if(attachment>=GL_COLOR_ATTACHMENT0 && attachment<(GL_COLOR_ATTACHMENT0+hardext.maxcolorattach) && tex) {
        gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
        if((hardext.npot==1 || hardext.npot==2) && (!tex->actual.wrap_s || !tex->actual.wrap_t || !wrap_npot(tex->actual.wrap_s) || !wrap_npot(tex->actual.wrap_t))) {
            tex->sampler.wrap_s = tex->sampler.wrap_t = GL_CLAMP_TO_EDGE;
            tex->adjust = 0;
        }
        //npot==2 and 3 should support that, but let's ignore that for now and force no mipmap for texture attached to fbo...
        if(!tex->actual.min_filter || !minmag_npot(tex->actual.min_filter) || tex->actual.min_filter!=tex->sampler.min_filter) {
            if(hardext.npot<2) {
                tex->sampler.min_filter = minmag_forcenpot(tex->sampler.min_filter);
                tex->adjust = 0;
                tex->mipmap_need = 0;
                tex->mipmap_auto = 0;
            }
        }
        realize_1texture(map_tex_target(textarget), -1, tex, NULL);
    }

    if(attachment==GL_DEPTH_ATTACHMENT /*&& hardext.depthtex==0*/) {
        noerrorShim();
        if (level!=0) return;
        if(hardext.depthtex && (tex || !texture)) {
            // depth texture supported!
            //check if texture needs to be re-created as true depth texture
            if(tex && !(tex->format==GL_DEPTH_COMPONENT || tex->format==GL_DEPTH_STENCIL)) {
                tex->format = GL_DEPTH_COMPONENT;
                if(tex->type!=GL_UNSIGNED_INT && tex->type!=GL_UNSIGNED_SHORT && tex->type!=GL_FLOAT) tex->type = (hardext.depth24)?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT;
                tex->fpe_format = FPE_TEX_DEPTH;
                realize_textures(0);
                int oldactive = glstate->texture.active;
                if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                GLuint oldtex = bound->glname;
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
            }
            // bind the depth texture...
            host_functions.glFramebufferTexture2D(ntarget, attachment, GL_TEXTURE_2D, texture, 0);
        } else {
            // let's create a renderbuffer and attach it instead of the (presumably) depth texture
            if(tex && !tex->renderdepth) {
                cobalt_glGenRenderbuffers(1, &tex->renderdepth);
                cobalt_glBindRenderbuffer(GL_RENDERBUFFER, tex->renderdepth);
                cobalt_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, twidth, theight);
                cobalt_glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
            cobalt_glFramebufferRenderbuffer(ntarget, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tex?tex->renderdepth:0);
        }
        errorGL();
        ReadDraw_Pop(target);
        return;
    }
    if(attachment==GL_STENCIL_ATTACHMENT /*&& hardext.depthtex==0*/) {
        noerrorShim();
        if (level!=0) return;
        // this one is tricky, as it can be GL_DEPTH_STENCIL with DEPTH+DEPTH_STENCIL extension of GL_STENCIL_INDEX8 with STENCIL extension
        // and having DEPTH+DEPTH_STENCIL extension doesn't grant STENCIL8 texture extension!
        if((tex || !texture) && (hardext.stenciltex || (hardext.depthtex && hardext.depthstencil))) {
            // depth texture supported, so are stencil one then!
            //check if texture needs to be re-created as true depth texture
            if(tex && !(tex->format==GL_STENCIL_INDEX8 || tex->format==GL_DEPTH_STENCIL)) {
                if(tex->format==GL_DEPTH_ATTACHMENT) {
                    //TODO: need to create a new texture, as the depth one is probably used
                    cobalt_glGenTextures(1, &texture);
                    realize_textures(0);
                    int oldactive = glstate->texture.active;
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                    gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                    GLuint oldtex = bound->glname;
                    int nwidth = tex->nwidth;
                    int nheight = tex->nheight;
                    tex = cobalt_getTexture(textarget, texture);
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    tex->format = (hardext.stenciltex)?GL_STENCIL_INDEX8:GL_DEPTH_STENCIL;
                    tex->type = (hardext.stenciltex)?GL_UNSIGNED_BYTE:GL_UNSIGNED_INT_24_8;
                    cobalt_glTexImage2D(GL_TEXTURE_2D, 0, tex->format, nwidth, nheight, 0, tex->format, tex->type, NULL);
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
                } else {
                    tex->format = GL_STENCIL_INDEX8;
                    if(tex->type!=GL_UNSIGNED_BYTE && tex->type!=GL_UNSIGNED_SHORT && tex->type!=GL_FLOAT) tex->type = GL_UNSIGNED_BYTE;
                    tex->fpe_format = FPE_TEX_DEPTH;
                    realize_textures(0);
                    int oldactive = glstate->texture.active;
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                    gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                    GLuint oldtex = bound->glname;
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
                }
            }
            // bind the stencil texture...
            host_functions.glFramebufferTexture2D(ntarget, attachment, GL_TEXTURE_2D, texture, 0);
        } else {
            // let's create a renderbuffer and attach it instead of the (presumably) stencil texture
            if(tex && !tex->renderstencil) {
                cobalt_glGenRenderbuffers(1, &tex->renderstencil);
                cobalt_glBindRenderbuffer(GL_RENDERBUFFER, tex->renderstencil);
                cobalt_glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, twidth, theight);
                cobalt_glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
            cobalt_glFramebufferRenderbuffer(ntarget, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, tex?tex->renderstencil:0);
        }
        errorGL();
        ReadDraw_Pop(target);
        return;
    }
    if(attachment==GL_DEPTH_STENCIL_ATTACHMENT /*&& hardext.depthtex==0*/) {
        noerrorShim();
        if (level!=0) return;
        // let's create a renderbuffer and attach it instead of the (presumably) depth texture
        if(hardext.depthstencil /*&& !(hardext.depthtex && tex)*/) {
            if(hardext.depthtex && (tex || !texture)) {
                if(tex && tex->format!=GL_DEPTH_STENCIL) {
                    tex->format = GL_DEPTH_STENCIL;
                    tex->type = GL_UNSIGNED_INT_24_8;
                    tex->fpe_format = FPE_TEX_DEPTH; // add depth_stencil?
                    int oldactive = glstate->texture.active;
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                    gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                    GLuint oldtex = bound->glname;
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
                }
                host_functions.glFramebufferTexture2D(ntarget, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
                host_functions.glFramebufferTexture2D(ntarget, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
            } else {
                if(tex && !tex->renderdepth) {
                    cobalt_glGenRenderbuffers(1, &tex->renderdepth);
                    cobalt_glBindRenderbuffer(GL_RENDERBUFFER, tex->renderdepth);
                    cobalt_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, twidth, theight);
                    cobalt_glBindRenderbuffer(GL_RENDERBUFFER, 0);
                }
                errorGL();
                cobalt_glFramebufferRenderbuffer(ntarget, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tex?tex->renderdepth:0);
                cobalt_glFramebufferRenderbuffer(ntarget, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, tex?tex->renderdepth:0);
            }
        } else {
            if(hardext.depthtex && (tex || !texture)) {
                // depth texture supported!
                //check if texture needs to be re-created ad true depth texture
                if(tex && tex->format!=GL_DEPTH_COMPONENT) {
                    tex->format = GL_DEPTH_COMPONENT;
                    if(tex->type!=GL_UNSIGNED_INT && tex->type!=GL_UNSIGNED_SHORT && tex->type!=GL_FLOAT) tex->type = (hardext.depth24)?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT;
                    tex->fpe_format = FPE_TEX_DEPTH;
                    int oldactive = glstate->texture.active;
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
                    gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
                    GLuint oldtex = bound->glname;
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, tex->glname);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
                    if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
                    if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
                }
                // bind the depth texture...
                host_functions.glFramebufferTexture2D(ntarget, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
            } else {
                if(tex && !tex->renderdepth) {
                    cobalt_glGenRenderbuffers(1, &tex->renderdepth);
                    cobalt_glBindRenderbuffer(GL_RENDERBUFFER, tex->renderdepth);
                    cobalt_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, twidth, theight);
                    cobalt_glBindRenderbuffer(GL_RENDERBUFFER, 0);
                }
                cobalt_glFramebufferRenderbuffer(ntarget, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tex?tex->renderdepth:0);
            }
            if(tex && !tex->renderstencil) {
                cobalt_glGenRenderbuffers(1, &tex->renderstencil);
                cobalt_glBindRenderbuffer(GL_RENDERBUFFER, tex->renderstencil);
                cobalt_glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, twidth, theight);
                cobalt_glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
            errorGL();
            cobalt_glFramebufferRenderbuffer(ntarget, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, tex?tex->renderstencil:0);

        }
        ReadDraw_Pop(target);
        return;
    }

    twidth = twidth >> level; if(twidth<1) twidth=1;
    theight = theight >> level; if(theight<1) theight=1;
    
	if (level!=0) {
        //bind a scrap texture, we don't want level != 0 binding on GLES
        if(!scrap_tex)
            cobalt_glGenTextures(1, &scrap_tex);
        if ((scrap_width!=twidth) || (scrap_height!=theight)) {
                scrap_width = twidth;
                scrap_height = theight;
                gltexture_t *bound = glstate->texture.bound[glstate->texture.active][ENABLED_TEX2D];
                GLuint oldtex = bound->glname;
                if (oldtex!=scrap_tex) host_functions.glBindTexture(GL_TEXTURE_2D, scrap_tex);
                host_functions.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrap_width, scrap_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                if (oldtex!=scrap_tex) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
        }
        texture = scrap_tex;
    }
    
    errorGL();
    GLenum realtarget = GL_TEXTURE_2D;
    if(textarget>=GL_TEXTURE_CUBE_MAP_POSITIVE_X && textarget<GL_TEXTURE_CUBE_MAP_POSITIVE_X+6)
        realtarget = textarget;
    host_functions.glFramebufferTexture2D(ntarget, map_attachment(attachment), realtarget, texture, 0);
    DBG(CheckGLError(1);)
    ReadDraw_Pop(target);
}

void COBALT_API cobalt_glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level) {
    cobalt_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}
void COBALT_API cobalt_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level, GLint layer) {
    (void)layer;
    cobalt_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void COBALT_API cobalt_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {
    DBG(printf("glGenRenderbuffers(%i, %p)\n", n, renderbuffers);)
    
    errorGL();
    host_functions.glGenRenderbuffers(n, renderbuffers);
    // track the renderbuffers...
    int ret;
    khint_t k;
    khash_t(renderbufferlist_t) *list = glstate->fbo.renderbufferlist;
    for(int i=0; i<n; ++i) {
        k = kh_put(renderbufferlist_t, list, renderbuffers[i], &ret);
        glrenderbuffer_t *rend = kh_value(list, k) = malloc(sizeof(glrenderbuffer_t));
        memset(rend, 0, sizeof(glrenderbuffer_t));
        rend->renderbuffer = renderbuffers[i];
    }
}

void COBALT_API cobalt_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    DBG(printf("glFramebufferRenderbuffer(%s, %s, %s, %u)\n", PrintEnum(target), PrintEnum(attachment), PrintEnum(renderbuffertarget), renderbuffer);)
    
    
    

    glframebuffer_t *fb = get_framebuffer(target);
    if(!fb) {
        errorShim(GL_INVALID_ENUM);
        return;
    }

    if( !(attachment>=GL_COLOR_ATTACHMENT0 && attachment<(GL_COLOR_ATTACHMENT0+hardext.maxcolorattach))
     && attachment!=GL_DEPTH_ATTACHMENT 
     && attachment!=GL_STENCIL_ATTACHMENT 
     && attachment!=GL_DEPTH_STENCIL_ATTACHMENT) {
         errorShim(GL_INVALID_ENUM);
         return;
     }
    
    // get renderbuffer
    glrenderbuffer_t *rend = find_renderbuffer(renderbuffer);
    if(!rend /*|| !rend->renderbuffer*/) {
        errorShim(GL_INVALID_OPERATION);
        return;
    }

    if (attachment >= GL_COLOR_ATTACHMENT0 && (attachment < (GL_COLOR_ATTACHMENT0+hardext.maxcolorattach)) && g_cobalt.fboforcetex) {
        if(rend->renderbuffer) {
            // drop the renderbuffer attachement and create a texture instead...
            int oldactive = glstate->texture.active;
            if(oldactive) cobalt_glActiveTexture(GL_TEXTURE0);
            gltexture_t *bound = glstate->texture.bound[0][ENABLED_TEX2D];
            GLuint oldtex = bound->glname;
            // get size of renderbuffer
            GLenum format = rend->format;
            GLint width = rend->width;
            GLint height = rend->height;
            // create a texture if needed
            if(!rend->secondarytexture) {
                GLuint newtex;
                cobalt_glGenTextures(1, &newtex);
                cobalt_glBindTexture(GL_TEXTURE_2D, newtex);
                cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                cobalt_glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                cobalt_glBindTexture(GL_TEXTURE_2D, oldtex);
                rend->secondarytexture = newtex;
            }
            cobalt_glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, rend->secondarytexture, 0);
            // end of cleanup
            if(oldactive) cobalt_glActiveTexture(GL_TEXTURE0+oldactive);
        } else {
            // renderbuffer is 0, unbind the texture attachement...
            cobalt_glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, 0, 0);
        }
        return;
    }

    if (attachment==GL_DEPTH_STENCIL_ATTACHMENT) {
        // doesn't seems to be supported "as-is" on GLES on PVR
        cobalt_glFramebufferRenderbuffer(target, GL_DEPTH_ATTACHMENT, renderbuffertarget, renderbuffer);
        cobalt_glFramebufferRenderbuffer(target, GL_STENCIL_ATTACHMENT, renderbuffertarget, renderbuffer);
        return;
    }

    
    //TODO: handle target=READBUFFER or DRAWBUFFER...
    if (attachment==GL_STENCIL_ATTACHMENT) {
        if(rend && rend->secondarybuffer)
            renderbuffer = rend->secondarybuffer;
    }

    fb->width  = rend->width;
    fb->height = rend->height;
    if ((GetAttachmentType(fb, attachment) == GL_RENDERBUFFER) && (GetAttachment(fb, attachment)==renderbuffer))
    {
        // no need to reattach
        noerrorShim();
        return;
    }

    SetAttachment(fb, attachment, GL_RENDERBUFFER, renderbuffer, 0);

    /*if ((glstate->fbo.current_fb!=0) && (renderbuffer==0)) {
        //Hack, avoid unbind a renderbuffer on a framebuffer...
        // TODO, avoid binding an already binded RB
        noerrorShim();
        return;
    }*/ // Let it do it now
    
    GLenum ntarget = ReadDraw_Push(target);

    errorGL();
    host_functions.glFramebufferRenderbuffer(ntarget, attachment, renderbuffertarget, renderbuffer);
    DBG(CheckGLError(1);)
    ReadDraw_Pop(target);
}

void COBALT_API cobalt_glDeleteRenderbuffers(GLsizei n, GLuint *renderbuffers) {
    DBG(printf("glDeleteRenderbuffer(%d, %p)\n", n, renderbuffers);)
    
    
    // check if we delete a depthstencil
    if (glstate->fbo.renderbufferlist)
        for (int i=0; i<n; i++) {
            khint_t k;
            glrenderbuffer_t *rend;
            for (int i = 0; i < n; i++) {
                GLuint t = renderbuffers[i];
                if(t) {
                    k = kh_get(renderbufferlist_t, glstate->fbo.renderbufferlist, t);
                    if (k != kh_end(glstate->fbo.renderbufferlist)) {
                        rend = kh_value(glstate->fbo.renderbufferlist, k);
                        if(glstate->fbo.current_rb == rend)
                            glstate->fbo.current_rb = glstate->fbo.default_rb;
                        if(rend->secondarybuffer)
                            host_functions.glDeleteRenderbuffers(1, &rend->secondarybuffer);
                        if(rend->secondarytexture)
                            cobalt_glDeleteTextures(1, &rend->secondarytexture);
                        free(rend);
                        kh_del(renderbufferlist_t, glstate->fbo.renderbufferlist, k);
                    }
                }
            }
        }

    errorGL();
    host_functions.glDeleteRenderbuffers(n, renderbuffers);
}

void COBALT_API cobalt_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    DBG(printf("glRenderbufferStorage(%s, %s, %i, %i)\n", PrintEnum(target), PrintEnum(internalformat), width, height);)
    
    
    

    glrenderbuffer_t *rend = glstate->fbo.current_rb;
    if(!rend->renderbuffer) {
        errorShim(GL_INVALID_OPERATION);
        return;
    }
    
    errorGL();
    width = (hardext.npot>0 && !g_cobalt.potframebuffer)?width:npot(width);
    height = (hardext.npot>0 && !g_cobalt.potframebuffer)?height:npot(height);
    int use_secondarybuffer = 0;
    int use_secondarytexture = 0;
    GLenum format = internalformat;
    // check if internal format is GL_DEPTH_STENCIL_EXT
    if (internalformat == GL_DEPTH_STENCIL)
        internalformat = GL_DEPTH24_STENCIL8;
    // in that case, create first a STENCIL one then a DEPTH one....
    if ((internalformat == GL_DEPTH24_STENCIL8 && (hardext.depthstencil==0 || ((hardext.vendor&VEND_IMGTEC)==VEND_IMGTEC)))) {
        internalformat = (hardext.depth24)?GL_DEPTH_COMPONENT24:GL_DEPTH_COMPONENT16;
        // create a stencil buffer if needed
        if(!rend->secondarybuffer) {
            host_functions.glGenRenderbuffers(1, &rend->secondarybuffer);
        }
        use_secondarybuffer = 1;
    }
    else if (internalformat == GL_DEPTH_COMPONENT || internalformat == GL_DEPTH_COMPONENT32)    // Not much is supported on GLES...
        internalformat = GL_DEPTH_COMPONENT16;
    else if (internalformat == GL_RGB8 && hardext.rgba8==0)
        internalformat = GL_RGB565_OES;
    else if (internalformat == GL_RGBA8 && hardext.rgba8==0)
        internalformat = GL_RGBA4_OES;
    else if (internalformat == GL_RGB5)
        internalformat = GL_RGB565_OES;
    else if (internalformat == GL_R3_G3_B2)
        internalformat = GL_RGB565_OES;
    else if (internalformat == GL_RGB4)
        internalformat = GL_RGBA4_OES;
    else if (internalformat == GL_RGBA) {
        if(hardext.rgba8==0)
            internalformat = GL_RGBA8;
        else
            internalformat = GL_RGBA4_OES;
    }

    if(rend->secondarybuffer) {
        if(use_secondarybuffer) {
            GLuint current_rb = glstate->fbo.current_rb->renderbuffer;
            host_functions.glBindRenderbuffer(GL_RENDERBUFFER, rend->secondarybuffer);
            host_functions.glRenderbufferStorage(target, GL_STENCIL_INDEX8, width, height);
            host_functions.glBindRenderbuffer(GL_RENDERBUFFER, current_rb);
        } else {
            
            host_functions.glDeleteRenderbuffers(1, &rend->secondarybuffer);
            rend->secondarybuffer = 0;
        }
    }

    if(rend->secondarytexture) {
        // should check if texture is still needed?
        gltexture_t *tex = cobalt_getTexture(GL_TEXTURE_2D, rend->secondarytexture);
        
        
        
        int oldactive = glstate->texture.active;
        if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0);
        gltexture_t *bound = glstate->texture.bound[0/*glstate->texture.active*/][ENABLED_TEX2D];
        GLuint oldtex = bound->glname;
        if (oldtex!=rend->secondarytexture) host_functions.glBindTexture(GL_TEXTURE_2D, rend->secondarytexture);
        tex->nwidth = tex->width = width;
        tex->nheight = tex->height = height;
        host_functions.glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->nwidth, tex->nheight, 0, tex->format, tex->type, NULL);
        if (oldtex!=tex->glname) host_functions.glBindTexture(GL_TEXTURE_2D, oldtex);
        if(oldactive) host_functions.glActiveTexture(GL_TEXTURE0+oldactive);
    }

    rend->width  = width;
    rend->height = height;
    rend->format = format;
    rend->actual = internalformat;

    host_functions.glRenderbufferStorage(target, internalformat, width, height);
    DBG(CheckGLError(1);)
}

void COBALT_API cobalt_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {    //STUB
    cobalt_glRenderbufferStorage(target, internalformat, width, height);
}

void COBALT_API cobalt_glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
    DBG(printf("glBindRenderbuffer(%s, %u), binded Fbo=%u\n", PrintEnum(target), renderbuffer, glstate->fbo.current_fb->id);)
    
    
    GLuint current = glstate->fbo.current_rb->renderbuffer;
    if(current==renderbuffer) {
        noerrorShim();
        return;
    }
     glrenderbuffer_t * rend = find_renderbuffer(renderbuffer);
    if(!rend || !rend->renderbuffer) {
        errorShim(GL_INVALID_OPERATION);
        return;
    }
    glstate->fbo.current_rb = rend;
    
    errorGL();
    host_functions.glBindRenderbuffer(target, renderbuffer);
}

GLboolean COBALT_API cobalt_glIsRenderbuffer(GLuint renderbuffer) {
    DBG(printf("glIsRenderbuffer(%u)\n", renderbuffer);)
    noerrorShim();
    return((find_renderbuffer(renderbuffer)!=NULL)?GL_TRUE:GL_FALSE);
}

void COBALT_API cobalt_glGenerateMipmap(GLenum target) {
    DBG(printf("glGenerateMipmap(%s)\n", PrintEnum(target));)
    
    
    const GLuint rtarget = map_tex_target(target);
    realize_bound(glstate->texture.active, target);
    gltexture_t *bound = cobalt_getCurrentTexture(target);
    if(g_cobalt.forcenpot && hardext.npot==1) {
        if(bound->npot) {
            noerrorShim();
            return; // no need to generate mipmap, mipmap is disabled here
        }
    }

    errorGL();
    if(g_cobalt.automipmap != 3) {
        host_functions.glGenerateMipmap(rtarget);
        bound->mipmap_auto = 1;
        /*if(bound->sampler.min_filer != bound->actual.min_filter)  // mainly for S3TC textures...
            cobalt_glTexParameteri(target, GL_TEXTURE_MIN_FILTER, bound->sampler.min_filer);*/
    }
}

void COBALT_API cobalt_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params) {
    DBG(printf("glGetFramebufferAttachmentParameteriv(%s, %s, %s, %p)\n", PrintEnum(target), PrintEnum(attachment), PrintEnum(pname), params);)
    

    glframebuffer_t *fb = get_framebuffer(target);
    if(!fb) {
        errorShim(GL_INVALID_ENUM);
        return;
    }

    if( !(attachment>=GL_COLOR_ATTACHMENT0 && attachment<(GL_COLOR_ATTACHMENT0+hardext.maxcolorattach))
     && attachment!=GL_DEPTH_ATTACHMENT 
     && attachment!=GL_STENCIL_ATTACHMENT 
     && attachment!=GL_DEPTH_STENCIL_ATTACHMENT) {
         errorShim(GL_INVALID_ENUM);
         return;
    }

    if(pname==GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME) {
        noerrorShim();
        *params = GetAttachment(fb, attachment);
        return;
    }

    if(pname==GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE) {
        noerrorShim();
        *params = GetAttachmentType(fb, attachment);
        if(*params!=0 && *params!=GL_RENDERBUFFER)
            *params = GL_TEXTURE;
        return;
    }
    
    if(pname==GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL) {
        GLenum tmp = GetAttachmentType(fb, attachment);
        if(tmp!=0 && tmp!=GL_RENDERBUFFER) {
            noerrorShim();
            *params = GetAttachmentLevel(fb, attachment);
        } else {
            errorShim(GL_INVALID_ENUM);
        }
        return;
    }
    if(pname==GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE) {
        GLenum tmp = GetAttachmentType(fb, attachment);
        if(tmp!=0 && tmp!=GL_RENDERBUFFER) {
            noerrorShim();
            *params = (tmp>=GL_TEXTURE_CUBE_MAP_POSITIVE_X && tmp<=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)?tmp:0;
        } else {
            errorShim(GL_INVALID_ENUM);
        }
        return;        
    }
    // more stuff can be done
    /*
    if(pname==GL_FRAMEBUFFER_ATTACHMENT_LAYERED) {
        GLenum tmp = GetAttachmentType(fb, attachment);
        noerrorShim();
        *params = (tmp==GL_TEXTURE_CUBE_MAP || tmp==GL_TEXTURE_3D)?GL_TRUE:GL_FALSE;
        return;
    }
    */
    // hack to return DEPTH size
    if(attachment==GL_DEPTH_ATTACHMENT && pname==GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE) {
        if(hardext.depthtex==0) {
            errorGL();
            *params = GetAttachment(fb, attachment);
            //TODO: Check all this?!
            if (*params)
                *params = 16;   //Depth buffer is 16 on GLES. No check for 24 bits here...
            return;
        }
        // check if it's depth/stencil 16/8, in that case, lie by spoofing 24/8 or some FNA game will fail
        int depth, stencil;
        GLenum ntarget = ReadDraw_Push(target);
        host_functions.glGetFramebufferAttachmentParameteriv(ntarget, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil);
        host_functions.glGetFramebufferAttachmentParameteriv(ntarget, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth);
        errorGL();
        ReadDraw_Pop(target);
        if(depth==16 && stencil==8)
            depth = 24;
        *params = depth;
        return;
    }

    GLenum ntarget = ReadDraw_Push(target);
    
    errorGL();
    host_functions.glGetFramebufferAttachmentParameteriv(ntarget, attachment, pname, params);
    ReadDraw_Pop(target);
}

void COBALT_API cobalt_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params) {
    DBG(printf("glGetRenderbufferParameteriv(%s, %s, %p)\n", PrintEnum(target), PrintEnum(pname), params);)
    
    
    errorGL();
    host_functions.glGetRenderbufferParameteriv(target, pname, params);
}

void createMainFBO(int width, int height) {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    // If there is already a Framebuffer created, let's delete it.... unless it's already the right size!
    int createIt = 1;
    if (glstate->fbo.mainfbo_fbo) {
        if (width==glstate->fbo.mainfbo_width && height==glstate->fbo.mainfbo_height)
            return;
        //lets adjust the FBO instead of adjusting it
        createIt = 0;
    }
    DBG(printf("COBALT: Create FBO of %ix%i 32bits\n", width, height);)
    // switch to texture unit 0 if needed
    if (glstate->texture.active != 0)
        host_functions.glActiveTexture(GL_TEXTURE0);
    if (glstate->texture.client != 0 && host_functions.glClientActiveTexture)
        host_functions.glClientActiveTexture(GL_TEXTURE0);
        
    glstate->fbo.mainfbo_width = width;
    glstate->fbo.mainfbo_height = height;
    glstate->fbo.mainfbo_nwidth = width = hardext.npot>0?width:npot(width);
    glstate->fbo.mainfbo_nheight = height = hardext.npot>0?height:npot(height);

    // create the texture
    if(createIt)
	    host_functions.glGenTextures(1, &glstate->fbo.mainfbo_tex);
    host_functions.glBindTexture(GL_TEXTURE_2D, glstate->fbo.mainfbo_tex);
    if(createIt) {
        host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    host_functions.glTexImage2D(GL_TEXTURE_2D, 0, g_cobalt.fbo_noalpha?GL_RGB:GL_RGBA, width, height,
					0, g_cobalt.fbo_noalpha?GL_RGB:GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	host_functions.glBindTexture(GL_TEXTURE_2D, 0);
    // create the render buffers
    if(createIt) {
        host_functions.glGenRenderbuffers(1, &glstate->fbo.mainfbo_dep);
        host_functions.glGenRenderbuffers(1, &glstate->fbo.mainfbo_ste);
    }
    host_functions.glBindRenderbuffer(GL_RENDERBUFFER, glstate->fbo.mainfbo_ste);
    host_functions.glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
    host_functions.glBindRenderbuffer(GL_RENDERBUFFER, glstate->fbo.mainfbo_dep);
    host_functions.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    host_functions.glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // create a fbo
    if(createIt)
        host_functions.glGenFramebuffers(1, &glstate->fbo.mainfbo_fbo);
    host_functions.glBindFramebuffer(GL_FRAMEBUFFER, glstate->fbo.mainfbo_fbo);
    
    // re-attach, even if not creating the fbo...
    host_functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glstate->fbo.mainfbo_ste);
    host_functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glstate->fbo.mainfbo_dep);
    
    host_functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glstate->fbo.mainfbo_tex, 0);

	GLenum status = host_functions.glCheckFramebufferStatus(GL_FRAMEBUFFER);

	host_functions.glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Put everything back
    host_functions.glBindTexture(GL_TEXTURE_2D, glstate->texture.bound[0][ENABLED_TEX2D]->glname);
    if (glstate->texture.active != 0)
        host_functions.glActiveTexture(GL_TEXTURE0 + glstate->texture.active);
    if (glstate->texture.client != 0 && host_functions.glClientActiveTexture)
        host_functions.glClientActiveTexture(GL_TEXTURE0 + glstate->texture.client);
    GLuint current_rb = glstate->fbo.current_rb->renderbuffer;
    host_functions.glBindRenderbuffer(GL_RENDERBUFFER, current_rb);
    // Final check, and bind the fbo for future use
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("COBALT: Error while creating main fbo (0x%04X)\n", status);
        deleteMainFBO(glstate);
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, glstate->fbo.current_fb->id);
        
    } else {
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, (glstate->fbo.current_fb->id)?glstate->fbo.current_fb->id:glstate->fbo.mainfbo_fbo);
        // clear color, depth and stencil...
        if (glstate->fbo.current_fb->id==0)
            host_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    
}

void blitMainFBO(int x, int y, int width, int height) {
    if (glstate->fbo.mainfbo_fbo==0)
        return;

    // blit the texture
    if(!width && !height) {
        cobalt_glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        cobalt_glClear(GL_COLOR_BUFFER_BIT);
    }

    GLint vp[4];
    memcpy(vp, &glstate->raster.viewport, sizeof(vp));
    cobalt_glViewport(0, 0, glstate->fbowidth, glstate->fboheight);
    float rx, ry;
    if(!width && !height) {
        width = glstate->fbo.mainfbo_width;
        height = glstate->fbo.mainfbo_height;
        rx = ry = 1.0f;
    } else {
        y = glstate->fboheight - (y+height);
        //y = - y;
        rx = (float)width/glstate->fbo.mainfbo_width;
        ry = (float)height/glstate->fbo.mainfbo_height;
    }
    cobalt_blitTexture(glstate->fbo.mainfbo_tex, 0.f, 0.f,
        glstate->fbo.mainfbo_width, glstate->fbo.mainfbo_height, 
        glstate->fbo.mainfbo_nwidth, glstate->fbo.mainfbo_nheight, 
        rx, ry,
        0, 0, x, y, BLIT_OPAQUE);
    cobalt_glViewport(vp[0], vp[1], vp[2], vp[3]);
}

void bindMainFBO() {
    
    
    if (!glstate->fbo.mainfbo_fbo)
        return;
    if (glstate->fbo.current_fb->id==0) {
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, glstate->fbo.mainfbo_fbo);
        //host_functions.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    }
}

void unbindMainFBO() {
    
    if (!glstate->fbo.mainfbo_fbo)
        return;
    if (glstate->fbo.current_fb->id==0) {
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void deleteMainFBO(void *state) {
    
    
    

    glstate_t *glstate = (glstate_t*)state;

    if (glstate->fbo.mainfbo_dep) {
        host_functions.glDeleteRenderbuffers(1, &glstate->fbo.mainfbo_dep);
        glstate->fbo.mainfbo_dep = 0;
    }
    if (glstate->fbo.mainfbo_ste) {
        host_functions.glDeleteRenderbuffers(1, &glstate->fbo.mainfbo_ste);
        glstate->fbo.mainfbo_ste = 0;
    }
    if (glstate->fbo.mainfbo_tex) {
        host_functions.glDeleteTextures(1, &glstate->fbo.mainfbo_tex);
        glstate->fbo.mainfbo_tex = 0;
    }
    if (glstate->fbo.mainfbo_fbo) {
        host_functions.glDeleteFramebuffers(1, &glstate->fbo.mainfbo_fbo);
        glstate->fbo.mainfbo_fbo = 0;
    }
    
    // all done...
}

void COBALT_API cobalt_glFramebufferTextureLayer(	GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    cobalt_glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texture,	level); // Force Texture2D, ignore layer (should track?)...
}

#ifndef NOX11
void cobalt_SwapBuffers_currentContext();    // defined in glx/glx.c
#endif
void COBALT_API cobalt_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    // mask will be ignored
    // filter will be taken only for ReadFBO has no Texture attached (so readpixel is used)
    DBG(printf("glBlitFramebuffer(%d, %d, %d, %d,  %d, %d, %d, %d,  0x%04X, %s) fbo_read=%d, fbo_draw=%d\n",
        srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, PrintEnum(filter), glstate->fbo.fbo_read->id, glstate->fbo.fbo_draw->id);)

    if((mask&GL_COLOR_BUFFER_BIT)==0)
        return; // cannot copy DEPTH or STENCIL data on GLES, only COLOR_BUFFER...

    if(glstate->fbo.fbo_read == glstate->fbo.fbo_draw && srcX0==dstX0 && srcX1==dstX1 && srcY0==dstY0 && srcY1==dstY1)
        return; // no need to try copying on itself
    
    if(dstX1==dstX0 || dstY1==dstY0)
        return; // nothing to draw
    if(srcX1==srcX0 || srcY1==srcY0)
        return; // nothing to draw

    GLuint texture = (glstate->fbo.fbo_read->id==0 && glstate->fbo.mainfbo_fbo)?glstate->fbo.mainfbo_tex:glstate->fbo.fbo_read->color[0];

    int created = (texture==0 || (glstate->fbo.fbo_read==glstate->fbo.fbo_draw));
    int oldtex = glstate->texture.active;
    DBG(printf("   blit: created=%d, texture=%u, oldtex=%d\n", created, texture, oldtex);)
    if (oldtex)
        cobalt_glActiveTexture(GL_TEXTURE0);
    float nwidth, nheight;
    if (created) {
        gltexture_t *old = glstate->texture.bound[ENABLED_TEX2D][0];
        cobalt_glGenTextures(1, &texture);
        cobalt_glBindTexture(GL_TEXTURE_2D, texture);
        cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter==GL_LINEAR)?GL_LINEAR:GL_NEAREST);
        cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filter==GL_LINEAR)?GL_LINEAR:GL_NEAREST);
        cobalt_glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcX0, srcY0, srcX1-srcX0, srcY1-srcY0, 0);
        srcX1-=srcX0; srcX0=0.f;
        srcY1-=srcY0; srcY0=0.f;
        cobalt_glBindTexture(GL_TEXTURE_2D, old->texture);
    }
    GLenum glname = texture;
    if(texture==glstate->fbo.mainfbo_tex) {
        nwidth = glstate->fbo.mainfbo_nwidth;
        nheight = glstate->fbo.mainfbo_nheight;
    } else {
        gltexture_t *tex = cobalt_getTexture(GL_TEXTURE_2D, texture);
        if(tex) {
            nwidth = tex->nwidth;
            nheight = tex->nheight;
            glname = tex->glname;
            if(!created) {
                if((tex->actual.min_filter!=filter) || (tex->actual.mag_filter!=filter)) {
                    gltexture_t *old = glstate->texture.bound[ENABLED_TEX2D][0];
                    if(old->texture != glname)
                        cobalt_glBindTexture(GL_TEXTURE_2D, glname);
                    cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
                    cobalt_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
                    tex->actual.min_filter = tex->actual.mag_filter = filter;
                    if(old->texture != glname)
                        cobalt_glBindTexture(GL_TEXTURE_2D, old->texture);
                }
            }
        } else {
            // not good if here!
            nwidth = srcX1;
            nheight = srcY1;
        }
    }
    float srcW = srcX1-srcX0;
    float srcH = srcY1-srcY0;
    float zoomx = ((float)(dstX1-dstX0))/srcW;
    float zoomy = ((float)(dstY1-dstY0))/srcH;
    // get the width / height of write FBO
    int fbowidth, fboheight;
    int blitfullscreen = 0;
    if(glstate->fbo.fbo_draw->id==0/* && glstate->fbo.mainfbo_fbo*/) {
        if(g_cobalt.blitfb0/* || (g_cobalt.usefb && !g_cobalt.usefbo)*/)
            blitfullscreen = 1;
        else {
            fbowidth = glstate->fbo.mainfbo_width;
            fboheight = glstate->fbo.mainfbo_height;
            if((glstate->fbo.mainfbo_width==abs(dstX1-dstX0)) && (glstate->fbo.mainfbo_height==abs(dstY1-dstY0))) {
                blitfullscreen = 1;
            } else {
                if (cobalt_getMainFBSize) {
                    cobalt_getMainFBSize(&glstate->fbo.mainfbo_width, &glstate->fbo.mainfbo_height);
                    if((glstate->fbo.mainfbo_width==abs(dstX1-dstX0)) && (glstate->fbo.mainfbo_height==abs(dstY1-dstY0)))
                        blitfullscreen = 1;
                }
            }
        }
    } else {
        fbowidth  = glstate->fbo.fbo_draw->width;
        fboheight = glstate->fbo.fbo_draw->height;
    }
    GLint vp[4];
    memcpy(vp, &glstate->raster.viewport, sizeof(vp));
    cobalt_glViewport(0, 0, fbowidth, fboheight);
    cobalt_blitTexture(glname, srcX0, srcY0, srcW, srcH, nwidth, nheight, zoomx, zoomy, 0, 0, dstX0, dstY0, BLIT_OPAQUE);
    cobalt_glViewport(vp[0], vp[1], vp[2], vp[3]);
    if(created) {
        cobalt_glDeleteTextures(1, &texture);
    }
    if(oldtex)
        cobalt_glActiveTexture(GL_TEXTURE0+oldtex);

#ifndef NOX11
    if(blitfullscreen)  // hack, force a swapbuffer (help wine d3d show stuff on certain games)
        cobalt_SwapBuffers_currentContext();
#endif
}

GLuint cobalt_getCurrentFBO() {
  return (glstate->fbo.current_fb->id)?glstate->fbo.current_fb->id:glstate->fbo.mainfbo_fbo;
}

void cobalt_setCurrentFBO() {
  
  host_functions.glBindFramebuffer(GL_FRAMEBUFFER, (glstate->fbo.current_fb->id)?glstate->fbo.current_fb->id:glstate->fbo.mainfbo_fbo);
}

// DrawBuffers functions are faked unless GL_EXT_draw_buffers is supported
void COBALT_API cobalt_glDrawBuffers(GLsizei n, const GLenum *bufs) {
    DBG( for(int i=0; i<n; ++i){printf("glDrawBuffers(%d, %p) [%i]=%s\n", n, bufs, i,  n?PrintHex(bufs[i]):"nil");} )

    GLenum attachment_list[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT5,GL_COLOR_ATTACHMENT6,GL_COLOR_ATTACHMENT7};

    // Map internally the real buffers used
    for (int i = 0; i < n; ++i) {
        DrawBufs[i] = bufs[i];
    }
    // Prepare the draw command
    for (int i = n; i < 8; ++i) {
        attachment_list[i] = GL_NONE;
    }

    DBG(printf("\n\n");)
    //DBG(printf("glDrawBuffers(%d, %p) [0]=%s\n", n, bufs, n?PrintEnum(bufs[0]):"nil");)
    if(hardext.drawbuffers) {
        
        host_functions.glDrawBuffers(8, attachment_list);
        errorGL();
    } else {
        if(n<0 || n>hardext.maxdrawbuffers) {
            errorShim(GL_INVALID_VALUE);
            return;
        }
    }
    glstate->fbo.fbo_draw->n_draw = n;
    memcpy(glstate->fbo.fbo_draw->drawbuff, bufs, n*sizeof(GLenum));
    noerrorShim();
}
void COBALT_API cobalt_glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs) {
    if(n<0 || n>hardext.maxdrawbuffers) {
        errorShim(GL_INVALID_VALUE);
        return;
    }
    glframebuffer_t* fb = find_framebuffer(framebuffer);
    if(hardext.drawbuffers) {
        GLuint oldf = glstate->fbo.fbo_draw->id;
        cobalt_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->id);
        
        host_functions.glDrawBuffers(n, bufs);
        errorGL();
        cobalt_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldf);
    }
    fb->n_draw = n;
    memcpy(fb->drawbuff, bufs, n*sizeof(GLenum));
    noerrorShim();
}


void COBALT_API cobalt_glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint * value) {
    noerrorShim();
    GLenum attch;


    switch(buffer) {
        case GL_COLOR:
            if(drawbuffer>glstate->fbo.fbo_draw->n_draw)
                return; // GL_NONE...
            attch = glstate->fbo.fbo_draw->drawbuff[buffer];

            if(!(attch>=GL_COLOR_ATTACHMENT0 && attch<GL_COLOR_ATTACHMENT0+hardext.maxdrawbuffers)) {
                errorShim(GL_INVALID_VALUE);
                return;
            } else {
                // TODO Find the draw buffer index associated to the real draw buffer
                
                if(host_functions.glClearBufferiv){
                    host_functions.glClearBufferiv(buffer, drawbuffer, value);
                    break;
                }

                GLfloat oldclear[4];
                
                // select the buffer...
                if(hardext.drawbuffers)
                    host_functions.glDrawBuffers(1, (const GLenum *) &drawbuffer);
                cobalt_glGetFloatv(GL_COLOR_CLEAR_VALUE, oldclear);
                // how to convert the value? Most FB will be 8bits / componant for now...
                cobalt_glClearColor(value[0]/127.0f, value[1]/127.0f, value[2]/127.0f, value[3]/127.0f);
                cobalt_glClear(GL_COLOR_BUFFER_BIT);
                cobalt_glClearColor(oldclear[0], oldclear[1], oldclear[2], oldclear[3]);
                // put back the draw buffers...
                if(hardext.drawbuffers)
                    host_functions.glDrawBuffers(glstate->fbo.fbo_draw->n_draw, glstate->fbo.fbo_draw->drawbuff);
                return;
            }
            break;
        case GL_STENCIL:
            if(drawbuffer==0) {
                GLint old;
                cobalt_glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &old);
                cobalt_glClearStencil(*value);
                cobalt_glClear(GL_STENCIL_BUFFER_BIT);
                cobalt_glClearStencil(old);
                return;
            } else {
                errorShim(GL_INVALID_ENUM);
                return;
            }
        default:
            errorShim(GL_INVALID_ENUM);
    }
    return;
}
void COBALT_API cobalt_glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint * value) {
    noerrorShim();
    GLenum attch;
    switch(buffer) {
        case GL_COLOR:
            if(drawbuffer>glstate->fbo.fbo_draw->n_draw)
                return; // GL_NONE...
            attch = glstate->fbo.fbo_draw->drawbuff[buffer];
            if(!(attch>=GL_COLOR_ATTACHMENT0 && attch<GL_COLOR_ATTACHMENT0+hardext.maxdrawbuffers)) {
                errorShim(GL_INVALID_VALUE);
                return;
            } else {
                // TODO Find the draw buffer index associated to the real draw buffer
                
                if(host_functions.glClearBufferuiv){
                    host_functions.glClearBufferuiv(buffer, drawbuffer, value);
                    break;
                }

                GLfloat oldclear[4];
                
                // select the buffer...
                if(hardext.drawbuffers)
                    host_functions.glDrawBuffers(1, (const GLenum *) &drawbuffer);
                cobalt_glGetFloatv(GL_COLOR_CLEAR_VALUE, oldclear);
                // how to convert the value? Most FB will be 8bits / componant for now...
                cobalt_glClearColor(value[0]/255.0f, value[1]/255.0f, value[2]/255.0f, value[3]/255.0f);
                cobalt_glClear(GL_COLOR_BUFFER_BIT);
                cobalt_glClearColor(oldclear[0], oldclear[1], oldclear[2], oldclear[3]);
                // put back the draw buffers...
                if(hardext.drawbuffers)
                    host_functions.glDrawBuffers(glstate->fbo.fbo_draw->n_draw, glstate->fbo.fbo_draw->drawbuff);
                return;
            }
            break;
        default:
            errorShim(GL_INVALID_ENUM);
    }
    return;
}
void COBALT_API cobalt_glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat * value) {
    noerrorShim();
    GLenum attch;
    switch(buffer) {
        case GL_COLOR:
            if(drawbuffer>glstate->fbo.fbo_draw->n_draw)
                return; // GL_NONE...
            attch = glstate->fbo.fbo_draw->drawbuff[buffer];
            if(!(attch>=GL_COLOR_ATTACHMENT0 && attch<GL_COLOR_ATTACHMENT0+hardext.maxdrawbuffers)) {
                errorShim(GL_INVALID_VALUE);
                return;
            } else {
                // TODO Find the draw buffer index associated to the real draw buffer
                
                if(host_functions.glClearBufferfv){
                    host_functions.glClearBufferfv(buffer, drawbuffer, value);
                    break;
                }

                GLfloat oldclear[4];
                
                // select the buffer...
                if(hardext.drawbuffers)
                    host_functions.glDrawBuffers(1, (const GLenum *) &drawbuffer);
                cobalt_glGetFloatv(GL_COLOR_CLEAR_VALUE, oldclear);
                // how to convert the value? Most FB will be 8bits / componant for now...
                cobalt_glClearColor(value[0], value[1], value[2], value[3]);
                cobalt_glClear(GL_COLOR_BUFFER_BIT);
                cobalt_glClearColor(oldclear[0], oldclear[1], oldclear[2], oldclear[3]);
                // put back the draw buffers...
                if(hardext.drawbuffers)
                    host_functions.glDrawBuffers(glstate->fbo.fbo_draw->n_draw, glstate->fbo.fbo_draw->drawbuff);
                return;
            }
            break;
        case GL_DEPTH:
            if(drawbuffer==0) {
                GLint old;
                cobalt_glGetIntegerv(GL_DEPTH_CLEAR_VALUE, &old);
                cobalt_glClearDepthf(*value);
                cobalt_glClear(GL_DEPTH_BUFFER_BIT);
                cobalt_glClearDepthf(old);
                return;
            } else {
                errorShim(GL_INVALID_ENUM);
                return;
            }
        default:
            errorShim(GL_INVALID_ENUM);
    }
    return;
}
void COBALT_API cobalt_glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
    if(buffer!=GL_DEPTH_STENCIL || drawbuffer!=0) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    GLint olds, oldd;
    cobalt_glGetIntegerv(GL_DEPTH_CLEAR_VALUE, &oldd);
    cobalt_glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &olds);
    cobalt_glClearDepthf(depth);
    cobalt_glClearStencil(stencil);
    cobalt_glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    cobalt_glClearDepthf(oldd);
    cobalt_glClearStencil(olds);
}

void COBALT_API cobalt_glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value) {
    GLuint oldf = glstate->fbo.fbo_draw->id;
    GLenum target = (glstate->fbo.fbo_draw==glstate->fbo.fbo_read)?GL_FRAMEBUFFER:GL_DRAW_FRAMEBUFFER;
    cobalt_glBindFramebuffer(target, framebuffer);
    cobalt_glClearBufferiv(buffer, drawbuffer, value);
    cobalt_glBindFramebuffer(target, oldf);
}
void COBALT_API cobalt_glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value) {
    GLuint oldf = glstate->fbo.fbo_draw->id;
    GLenum target = (glstate->fbo.fbo_draw==glstate->fbo.fbo_read)?GL_FRAMEBUFFER:GL_DRAW_FRAMEBUFFER;
    cobalt_glBindFramebuffer(target, framebuffer);
    cobalt_glClearBufferuiv(buffer, drawbuffer, value);
    cobalt_glBindFramebuffer(target, oldf);
}
void COBALT_API cobalt_glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value) {
    GLuint oldf = glstate->fbo.fbo_draw->id;
    GLenum target = (glstate->fbo.fbo_draw==glstate->fbo.fbo_read)?GL_FRAMEBUFFER:GL_DRAW_FRAMEBUFFER;
    cobalt_glBindFramebuffer(target, framebuffer);
    cobalt_glClearBufferfv(buffer, drawbuffer, value);
    cobalt_glBindFramebuffer(target, oldf);
}
void COBALT_API cobalt_glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
    GLuint oldf = glstate->fbo.fbo_draw->id;
    GLenum target = (glstate->fbo.fbo_draw==glstate->fbo.fbo_read)?GL_FRAMEBUFFER:GL_DRAW_FRAMEBUFFER;
    cobalt_glBindFramebuffer(target, framebuffer);
    cobalt_glClearBufferfi(buffer, drawbuffer, depth, stencil);
    cobalt_glBindFramebuffer(target, oldf);
}

void COBALT_API cobalt_glColorMaskIndexed(GLuint framebuffer, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    GLuint oldf = glstate->fbo.fbo_draw->id;
    GLenum target = (glstate->fbo.fbo_draw==glstate->fbo.fbo_read)?GL_FRAMEBUFFER:GL_DRAW_FRAMEBUFFER;
    cobalt_glBindFramebuffer(target, framebuffer);
    cobalt_glColorMask(red, green, blue, alpha);
    cobalt_glBindFramebuffer(target, oldf);
}

void cobalt_saveCurrentFBO()
{
    // this, in fact, bind FBO to 0 if it wasn't
    GLuint framebuffer = (glstate->fbo.current_fb)?glstate->fbo.current_fb->id:0;
    if(framebuffer==0)
        framebuffer = glstate->fbo.mainfbo_fbo;
    if(framebuffer) {
        
        if(hardext.vendor&VEND_ARM)
            cobalt_glFinish(); //MALI seems to need a flush commandbefore unbinding the Framebuffer here
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void cobalt_restoreCurrentFBO()
{
    // Bind back current FBO if it wasn't 0
    GLuint framebuffer = (glstate->fbo.current_fb)?glstate->fbo.current_fb->id:0;
    if(framebuffer==0)
        framebuffer = glstate->fbo.mainfbo_fbo;
    if(framebuffer) {
        
        host_functions.glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    }
}

// direct wrapper

AliasExport(void,glGenFramebuffers,,(GLsizei n, GLuint *ids));
AliasExport(void,glDeleteFramebuffers,,(GLsizei n, GLuint *framebuffers));
AliasExport(GLboolean,glIsFramebuffer,,(GLuint framebuffer));
AliasExport(GLenum,glCheckFramebufferStatus,,(GLenum target));
AliasExport(void,glBindFramebuffer,,(GLenum target, GLuint framebuffer));
AliasExport(void,glFramebufferTexture1D,,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level));
AliasExport(void,glFramebufferTexture2D,,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level));
AliasExport(void,glFramebufferTexture3D,,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level, GLint layer));
AliasExport(void,glGenRenderbuffers,,(GLsizei n, GLuint *renderbuffers));
AliasExport(void,glFramebufferRenderbuffer,,(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
AliasExport(void,glDeleteRenderbuffers,,(GLsizei n, GLuint *renderbuffers));
AliasExport(void,glRenderbufferStorage,,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height));
AliasExport(void,glBindRenderbuffer,,(GLenum target, GLuint renderbuffer));
AliasExport(GLboolean,glIsRenderbuffer,,(GLuint renderbuffer));
AliasExport(void,glGenerateMipmap,,(GLenum target));
AliasExport(void,glGetFramebufferAttachmentParameteriv,,(GLenum target, GLenum attachment, GLenum pname, GLint *params));
AliasExport(void,glGetRenderbufferParameteriv,,(GLenum target, GLenum pname, GLint * params));
AliasExport(void,glFramebufferTextureLayer,,(	GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer));
AliasExport(void,glBlitFramebuffer,,(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter));

// EXT direct wrapper
AliasExport(void,glGenFramebuffers,EXT,(GLsizei n, GLuint *ids));
AliasExport(void,glDeleteFramebuffers,EXT,(GLsizei n, GLuint *framebuffers));
AliasExport(GLboolean,glIsFramebuffer,EXT,(GLuint framebuffer));
AliasExport(GLenum,glCheckFramebufferStatus,EXT,(GLenum target));
AliasExport(void,glBindFramebuffer,EXT,(GLenum target, GLuint framebuffer));
AliasExport(void,glFramebufferTexture1D,EXT,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level));
AliasExport(void,glFramebufferTexture2D,EXT,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level));
AliasExport(void,glFramebufferTexture3D,EXT,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level, GLint layer));
AliasExport(void,glGenRenderbuffers,EXT,(GLsizei n, GLuint *renderbuffers));
AliasExport(void,glFramebufferRenderbuffer,EXT,(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
AliasExport(void,glDeleteRenderbuffers,EXT,(GLsizei n, GLuint *renderbuffers));
AliasExport(void,glRenderbufferStorage,EXT,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height));
AliasExport(void,glBindRenderbuffer,EXT,(GLenum target, GLuint renderbuffer));
AliasExport(GLboolean,glIsRenderbuffer,EXT,(GLuint renderbuffer));
AliasExport(void,glGenerateMipmap,EXT,(GLenum target));
AliasExport(void,glGetFramebufferAttachmentParameteriv,EXT,(GLenum target, GLenum attachment, GLenum pname, GLint *params));
AliasExport(void,glGetRenderbufferParameteriv,EXT,(GLenum target, GLenum pname, GLint * params));
AliasExport(void,glFramebufferTextureLayer,EXT,(	GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer));
AliasExport(void,glBlitFramebuffer,EXT,(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter));

// Multisample stub
AliasExport(void,glRenderbufferStorageMultisample,,(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height));

// DrawBuffers
AliasExport(void,glDrawBuffers,,(GLsizei n, const GLenum *bufs));
AliasExport(void,glDrawBuffers,ARB,(GLsizei n, const GLenum *bufs));
AliasExport(void,glNamedFramebufferDrawBuffers,,(GLuint framebuffer, GLsizei n, const GLenum *bufs));
AliasExport(void,glNamedFramebufferDrawBuffers,EXT,(GLuint framebuffer, GLsizei n, const GLenum *bufs));

// ClearBuffer...
AliasExport(void,glClearBufferiv,,(GLenum buffer, GLint drawbuffer, const GLint * value));
AliasExport(void,glClearBufferuiv,,(GLenum buffer, GLint drawbuffer, const GLuint * value));
AliasExport(void,glClearBufferfv,,(GLenum buffer, GLint drawbuffer, const GLfloat * value));
AliasExport(void,glClearBufferfi,,(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil));

AliasExport(void,glClearNamedFramebufferiv,,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value));
AliasExport(void,glClearNamedFramebufferuiv,,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value));
AliasExport(void,glClearNamedFramebufferfv,,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value));
AliasExport(void,glClearNamedFramebufferfi,,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil));

AliasExport(void,glClearNamedFramebufferiv,EXT,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value));
AliasExport(void,glClearNamedFramebufferuiv,EXT,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value));
AliasExport(void,glClearNamedFramebufferfv,EXT,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value));
AliasExport(void,glClearNamedFramebufferfi,EXT,(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil));

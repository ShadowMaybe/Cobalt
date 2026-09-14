#include "../host.h"
//
// Created by maks on 03.01.2024.
//

#include "buffer_copier.h"
static bool buffer_copier_is_init = false;
static GLuint tempfb;
static GLuint temp_texture;
static GLuint destfb;

void buffer_copier_init() {
    if(buffer_copier_is_init) return;
    
    
    
    
    host_functions.glGenTextures(1, &temp_texture);
    host_functions.glGenFramebuffers(1, &tempfb);
    host_functions.glGenFramebuffers(1, &destfb);
    host_functions.glBindTexture(GL_TEXTURE_2D, temp_texture);
    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    host_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    buffer_copier_is_init = true;
}
void buffer_copier_store(GLint x, GLint y, GLsizei w, GLsizei h) {
    
    int current_texbind;
    host_functions.glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texbind);
    buffer_copier_init();
    
    
    host_functions.glBindTexture(GL_TEXTURE_2D, temp_texture);
    host_functions.glTexImage2D(GL_TEXTURE_2D, 0,
                      GL_DEPTH_COMPONENT,
                      w, h, 0, GL_DEPTH_COMPONENT,
                      hardext.depth24 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
                      NULL);
    host_functions.glBindTexture(GL_TEXTURE_2D, current_texbind);
    
    
    
    host_functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempfb);
    host_functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, glstate->fbo.fbo_read->id);
    host_functions.glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, temp_texture, 0);
    host_functions.glBlitFramebuffer(x, y, x+w, y+h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    host_functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    host_functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glstate->fbo.current_fb->id);
    return;
}
void buffer_copier_release(GLenum target, GLuint what_target, GLint x, GLint y, GLint w, GLint h) {
    if(!buffer_copier_is_init) return;
    gltexture_t* texture = glstate->texture.bound[glstate->texture.active][what_target];
    
    
    
    host_functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destfb);
    host_functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, tempfb);
    host_functions.glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, texture->glname, 0);
    host_functions.glBlitFramebuffer(0, 0, w, h, x, y, x+w, y+h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    host_functions.glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    host_functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    host_functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glstate->fbo.current_fb->id);
    return;
}

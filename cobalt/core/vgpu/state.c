//
// Created by serpentspirale on 09/09/23.
//

#include "state.h"
#include "../const.h"

#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7

int MaxDrawBuffers = 8;
int MaxColorAttachs = 8;

GLfloat ClearColorValue[4] = {0.0, 0.0, 0.0, 0.0};
GLfloat ClearDepthValue[4] = {0.0, 0.0, 0.0, 0.0};
GLint ClearStencilValue[4] = {0, 0, 0, 0};

GLvoid * depthData = NULL;
GLuint depthWidth = 0;
GLuint depthHeight = 0;

GLenum Attachs[8] = {
        //GL_NONE,
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5,
        GL_COLOR_ATTACHMENT6,
        GL_COLOR_ATTACHMENT7,
};

int DrawBufs_Num = 0;
GLenum DrawBufs[8] = {
        //Default state according to the docs
        GL_COLOR_ATTACHMENT0,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
        GL_NONE,
};

GLenum map_attachment(GLenum attachment){
    //TODO return on non color attachment
    GLenum attachment_list[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT5,GL_COLOR_ATTACHMENT6,GL_COLOR_ATTACHMENT7};
    for (int i = 0; i < 8; ++i) {
    if(DrawBufs[i] == attachment) {
        return attachment_list[i];
        }
    }
    return attachment; // Default, unrelated attachment
}

//
// Created by serpentspirale on 09/09/23.
//

#include "../gles.h"
#include "../const.h"

#ifndef COBALT_STATE_H
#define COBALT_STATE_H

extern int MaxDrawBuffers;
extern int MaxColorAttachs;

extern GLfloat ClearColorValue[4];
extern GLfloat ClearDepthValue[4];
extern GLint ClearStencilValue[4];

extern GLvoid * depthData;
extern GLuint depthWidth;
extern GLuint depthHeight;

extern GLenum Attachs[8];
extern int DrawBufs_Num;
extern GLenum DrawBufs[8];


GLenum map_attachment(GLenum attachment);


#endif //COBALT_STATE_H

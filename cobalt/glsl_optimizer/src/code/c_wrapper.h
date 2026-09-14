//
// Created by serpentspirale on 17/06/23.
//



#ifndef COBALT_C_WRAPPER_H
#define COBALT_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

char *optimize_shader(char *source, int isVertex, int vGLSLVersion, int vTargetGLSLVersion, int second_pass );

#ifdef __cplusplus
} /* extern C */
#endif


#endif //COBALT_C_WRAPPER_H

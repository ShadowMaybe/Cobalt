#ifndef _EGL_EGL_H_
#define _EGL_EGL_H_

#ifdef NOEGL
# error Building EGL wrapper without EGL
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdbool.h>
#include <stdlib.h>

EGLint cobalt_eglGetError(void);
EGLDisplay cobalt_eglGetDisplay(EGLNativeDisplayType display_id);
EGLBoolean cobalt_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLBoolean cobalt_eglTerminate(EGLDisplay dpy);
const char * cobalt_eglQueryString(EGLDisplay dpy, EGLint name);
EGLBoolean cobalt_eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean cobalt_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean cobalt_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);

EGLSurface cobalt_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
EGLSurface cobalt_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLSurface cobalt_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
EGLBoolean cobalt_eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLBoolean cobalt_eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
EGLBoolean cobalt_eglBindAPI(EGLenum api);
EGLenum cobalt_eglQueryAPI(void);

EGLBoolean cobalt_eglWaitClient(void);
EGLBoolean cobalt_eglReleaseThread(void);
EGLSurface cobalt_eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
EGLBoolean cobalt_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
EGLBoolean cobalt_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean cobalt_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean cobalt_eglSwapInterval(EGLDisplay dpy, EGLint interval);
EGLContext cobalt_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
EGLBoolean cobalt_eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLBoolean cobalt_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLContext cobalt_eglGetCurrentContext(void);
EGLSurface cobalt_eglGetCurrentSurface(EGLint readdraw);
EGLDisplay cobalt_eglGetCurrentDisplay(void);
EGLDisplay cobalt_eglGetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
EGLBoolean cobalt_eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
EGLBoolean cobalt_eglWaitGL(void);
EGLBoolean cobalt_eglWaitNative(EGLint engine);
EGLBoolean cobalt_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
EGLBoolean cobalt_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
void* cobalt_eglGetProcAddress(const char *name);

void egl_getMainFBSize(EGLint* width, EGLint* height);

EGLSyncKHR cobalt_eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
EGLint cobalt_eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);

// Undocumented libmali internals, needed for ODROID Go Ultra
NativePixmapType cobalt_egl_create_pixmap_ID_mapping(void *pixmap);
NativePixmapType cobalt_egl_destroy_pixmap_ID_mapping(int id);

#endif // _EGL_EGL_H


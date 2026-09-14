#include "glx.h"
#include "../gl/init.h"
#include "../gl/logs.h"


#if defined(NOX11) && defined(GLX_STUBS)
//#define WARN_GLX_STUB() LOGD("Warning, stubbed %s\n", __FUNCTION__); return NULL;
#define WARN_GLX_STUB() return NULL;

GLXContext  cobalt_glXCreateContext(Display *display, XVisualInfo *visual, GLXContext shareList, Bool isDirect){ WARN_GLX_STUB(); }
GLXContext  cobalt_glXCreateContextAttribsARB(Display *display, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list){ WARN_GLX_STUB(); }
void  cobalt_glXDestroyContext(Display *display, GLXContext ctx){ WARN_GLX_STUB(); }
Display  *cobalt_glXGetCurrentDisplay(){ WARN_GLX_STUB(); }
XVisualInfo  *cobalt_glXChooseVisual(Display *display, int screen, int *attributes){ WARN_GLX_STUB(); }
Bool  cobalt_glXMakeCurrent(Display *display, GLXDrawable drawable, GLXContext context){ WARN_GLX_STUB(); }
Bool  cobalt_glXMakeContextCurrent(Display *display, int drawable, int readable, GLXContext context){ WARN_GLX_STUB(); }
void  cobalt_glXSwapBuffers(Display *display, GLXDrawable drawable){ WARN_GLX_STUB(); }
int  cobalt_glXGetConfig(Display *display, XVisualInfo *visual, int attribute, int *value){ WARN_GLX_STUB(); }
int  cobalt_glXQueryContext( Display *dpy, GLXContext ctx, int attribute, int *value){ WARN_GLX_STUB(); }
GLXContext  cobalt_glXGetCurrentContext(){ WARN_GLX_STUB(); }
GLXFBConfig  *cobalt_glXChooseFBConfig(Display *display, int screen, const int *attrib_list, int *count){ WARN_GLX_STUB(); }
GLXFBConfig  *cobalt_glXChooseFBConfigSGIX(Display *display, int screen, const int *attrib_list, int *count){ WARN_GLX_STUB(); }
GLXFBConfig  *cobalt_glXGetFBConfigs(Display *display, int screen, int *count){ WARN_GLX_STUB(); }
int  cobalt_glXGetFBConfigAttrib(Display *display, GLXFBConfig config, int attribute, int *value){ WARN_GLX_STUB(); }
XVisualInfo  *cobalt_glXGetVisualFromFBConfig(Display *display, GLXFBConfig config){ WARN_GLX_STUB(); }
GLXContext  cobalt_glXCreateNewContext(Display *display, GLXFBConfig config, int render_type, GLXContext share_list, Bool is_direct){ WARN_GLX_STUB(); }
void  cobalt_glXSwapIntervalEXT(Display *display, int drawable, int interval){ WARN_GLX_STUB(); }
void  cobalt_glXCopyContext(Display *display, GLXContext src, GLXContext dst, GLuint mask){ WARN_GLX_STUB(); }
Window  cobalt_glXCreateWindow(Display *display, GLXFBConfig config, Window win, int *attrib_list){ WARN_GLX_STUB(); }
void  cobalt_glXDestroyWindow(Display *display, void *win){ WARN_GLX_STUB(); }
GLXDrawable  cobalt_glXGetCurrentDrawable(){ WARN_GLX_STUB(); }
Bool  cobalt_glXIsDirect(Display * display, GLXContext ctx){ WARN_GLX_STUB(); }
void  cobalt_glXUseXFont(Font font, int first, int count, int listBase){ WARN_GLX_STUB(); }
int  cobalt_glXQueryDrawable(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value){ WARN_GLX_STUB(); }
void  cobalt_glXDestroyPbuffer(Display * dpy, GLXPbuffer pbuf){ WARN_GLX_STUB(); }
GLXPbuffer  cobalt_glXCreatePbuffer(Display * dpy, GLXFBConfig config, const int * attrib_list){ WARN_GLX_STUB(); }
GLXPixmap  cobalt_glXCreateGLXPixmap(Display *display, XVisualInfo * visual, Pixmap pixmap){ WARN_GLX_STUB(); }
GLXPixmap  cobalt_glXCreatePixmap(Display * dpy, GLXFBConfig config, Pixmap pixmap, const int * attrib_list){ WARN_GLX_STUB(); }
void  cobalt_glXDestroyGLXPixmap(Display *display, void *pixmap){ WARN_GLX_STUB(); }
void  cobalt_glXDestroyPixmap(Display *display, void *pixmap){ WARN_GLX_STUB(); }
GLXContext  cobalt_glXCreateContextAttribs(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list){ WARN_GLX_STUB(); }
void  cobalt_glXSwapIntervalMESA(int interval){ WARN_GLX_STUB(); }
void  cobalt_glXSwapIntervalSGI(int interval){ WARN_GLX_STUB(); }

AliasExport(GLXContext,glXCreateContext,,(Display *display, XVisualInfo *visual, GLXContext shareList, Bool isDirect));
AliasExport(GLXContext,glXCreateContextAttribs,ARB,(Display *display, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list));
AliasExport(void,glXDestroyContext,,(Display *display, GLXContext ctx));
AliasExport(Display*,glXGetCurrentDisplay,,());
AliasExport(XVisualInfo*,glXChooseVisual,,(Display *display, int screen, int *attributes));
AliasExport(Bool,glXMakeCurrent,,(Display *display, GLXDrawable drawable, GLXContext context));
AliasExport(Bool,glXMakeContextCurrent,,(Display *display, int drawable, int readable, GLXContext context));
AliasExport(void,glXSwapBuffers,,(Display *display, GLXDrawable drawable));
AliasExport(int,glXGetConfig,,(Display *display, XVisualInfo *visual, int attribute, int *value));
AliasExport(int,glXQueryContext,,( Display *dpy, GLXContext ctx, int attribute, int *value));
AliasExport(GLXContext,glXGetCurrentContext,,());
AliasExport(GLXFBConfig*,glXChooseFBConfig,,(Display *display, int screen, const int *attrib_list, int *count));
AliasExport(GLXFBConfig*,glXChooseFBConfig,SGIX,(Display *display, int screen, const int *attrib_list, int *count));
AliasExport(GLXFBConfig*,glXGetFBConfigs,,(Display *display, int screen, int *count));
AliasExport(int,glXGetFBConfigAttrib,,(Display *display, GLXFBConfig config, int attribute, int *value));
AliasExport(XVisualInfo*,glXGetVisualFromFBConfig,,(Display *display, GLXFBConfig config));
AliasExport(GLXContext,glXCreateNewContext,,(Display *display, GLXFBConfig config, int render_type, GLXContext share_list, Bool is_direct));
AliasExport(void,glXSwapIntervalEXT,,(Display *display, int drawable, int interval));
AliasExport(void,glXCopyContext,,(Display *display, GLXContext src, GLXContext dst, GLuint mask));
AliasExport(Window,glXCreateWindow,,(Display *display, GLXFBConfig config, Window win, int *attrib_list));
AliasExport(void,glXDestroyWindow,,(Display *display, void *win));
AliasExport(GLXDrawable,glXGetCurrentDrawable,,());
AliasExport(Bool,glXIsDirect,,(Display * display, GLXContext ctx));
AliasExport(void,glXUseXFont,,(Font font, int first, int count, int listBase));
AliasExport(int,glXQueryDrawable,,(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value));
AliasExport(void,glXDestroyPbuffer,,(Display * dpy, GLXPbuffer pbuf));
AliasExport(GLXPbuffer,glXCreatePbuffer,,(Display * dpy, GLXFBConfig config, const int * attrib_list));
AliasExport(GLXPixmap,glXCreateGLXPixmap,,(Display *display, XVisualInfo * visual, Pixmap pixmap));
AliasExport(GLXPixmap,glXCreatePixmap,,(Display * dpy, GLXFBConfig config, Pixmap pixmap, const int * attrib_list));
AliasExport(void,glXDestroyGLXPixmap,,(Display *display, void *pixmap));
AliasExport(void,glXDestroyPixmap,,(Display *display, void *pixmap));
AliasExport(GLXContext,glXCreateContextAttribs,,(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list));
#endif
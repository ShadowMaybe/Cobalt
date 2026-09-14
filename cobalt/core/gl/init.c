#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <stdio.h>
#include <direct.h>
#define getcwd(a,b) _getcwd(a,b)
#define snprintf _snprintf
#endif
#include "../../version.h"
#include "../glx/glx_gbm.h"
#include "../glx/streaming.h"
#include "build_info.h"
#include "debug.h"
#include "loader.h"
#include "logs.h"
#include "fpe_cache.h"
#include "init.h"
#include "envvars.h"
#if defined(__EMSCRIPTEN__)
#define NO_INIT_CONSTRUCTOR
#endif

void gl_init();
void gl_close();

#ifdef COBALT_COMPILE_SHARED
#ifdef AMIGAOS4
void agl_reset_internals();
#endif
void fpe_shader_reset_internals();
#endif

g_cobalt_t g_cobalt = {0};

#if defined(PANDORA) || defined(CHIP) || defined(GOA_CLONE)
static void fast_math() {
  // enable Cortex A8 RunFast
   int v = 0;
   __asm__ __volatile__ (
     "vmrs %0, fpscr\n"
     "orr  %0, #((1<<25)|(1<<24))\n" // default NaN, flush-to-zero
     "vmsr fpscr, %0\n"
     //"vmrs %0, fpscr\n"
     : "=&r"(v));
}
#endif

#ifndef DEFAULT_ES
#if defined(PANDORA) || defined(ANDROID)
#define DEFAULT_ES 1
#else
#define DEFAULT_ES 2
#endif
#endif

void load_libs();
void glx_init();

static int inited = 0;

EXPORT
void set_getmainfbsize(void (COBALT_API  *new_getMainFBSize)(int* w, int* h)) {
    cobalt_getMainFBSize = (void*)new_getMainFBSize;
}

EXPORT
void set_getprocaddress(void *(COBALT_API  *new_proc_address)(const char *)) {
    gles_getProcAddress = new_proc_address;
}

#ifdef NO_INIT_CONSTRUCTOR
EXPORT
#else
#if defined(_WIN32) || defined(__CYGWIN__)
#define BUILD_WINDOWS_DLL
// dll can't initialize emulator in startup code :(
static unsigned char dll_inited;
EXPORT
#endif
#if !defined(_MSC_VER) || defined(__clang__)
__attribute__((constructor(101)))
#endif
#endif
void cobalt_init() {
#ifdef BUILD_WINDOWS_DLL
    if(!dll_inited) {
       LOGE("Windows ES emulator's can't be initialized from DllMain (directX limitation)\n");
       return;
    }
#endif
    // only init 1 time
    if(inited++) return;
    // default init of globals
    memset(&g_cobalt, 0, sizeof(g_cobalt));
    g_cobalt.mergelist = 1;
    g_cobalt.queries = 1;
    g_cobalt.beginend = 1;
    #ifdef PYRA
    GetEnvVarInt("COBALT_DEEPBIND", &g_cobalt.deepbind, 0);
    #else
    GetEnvVarInt("COBALT_DEEPBIND", &g_cobalt.deepbind, 1);
    #endif
    // overrides by env. variables
		#ifdef COBALT_COMPILE_SHARED
			GetEnvVarInt("COBALT_NOBANNER",&g_cobalt.nobanner,1);
    #else
    	g_cobalt.nobanner = IsEnvVarTrue("COBALT_NOBANNER");
		#endif

		COBALT_LOGD("Initialising cobalt\n");

    if(!g_cobalt.nobanner) print_build_infos();

    #define env(name, global, message)                    \
        if(IsEnvVarTrue(#name)) {\
          COBALT_LOGD(message "\n");         \
          global = true;                                \
        }

    env(COBALT_XREFRESH, g_cobalt.xrefresh, "xrefresh will be called on cleanup");
    env(COBALT_STACKTRACE, g_cobalt.stacktrace, "stacktrace will be printed on crash");


    switch(ReturnEnvVarInt("COBALT_FB")) {
    	case 1:
        COBALT_LOGD("framebuffer output enabled\n");
        g_cobalt.usefb = 1;
    	  break;
    	case 2:
        COBALT_LOGD("using framebuffer + fbo\n");
        g_cobalt.usefb = 1;
        g_cobalt.usefbo = 1;
    	  break;
#ifndef NOX11
    	case 3:
        COBALT_LOGD("using pbuffer\n");
        g_cobalt.usefb = 0;
        g_cobalt.usepbuffer = 1;
    	  break;
#endif
    	case 4:
#ifdef NO_GBM
        COBALT_LOGD("GBM Support not builded, cannot use it\n");
#else
        COBALT_LOGD("using GBM\n");
        g_cobalt.usefb = 0;
        g_cobalt.usegbm = 1;
#endif
    	  break;
    	default:
    	  break;
    }
    env(COBALT_BLITFB0, g_cobalt.blitfb0, "Blit to FB 0 force a SwapBuffer");
    env(COBALT_FPS, g_cobalt.showfps, "fps counter enabled");
#if defined(USE_FBIO) || defined(PYRA)
    env(COBALT_VSYNC, g_cobalt.vsync, "vsync enabled");
#endif
#ifdef PANDORA
		if(GetEnvVarFloat("COBALT_GAMMA",&g_cobalt.gamma,0.0f)) {
      COBALT_LOGD("Set gamma to %.2f\n", g_cobalt.gamma);
		}
#endif
    env(COBALT_NOBGRA, g_cobalt.nobgra, "Ignore BGRA texture capability");
    env(COBALT_NOTEXRECT, g_cobalt.notexrect, "Don't export Text Rectangle extension");
    if(g_cobalt.usefbo) {
      env(COBALT_FBONOALPHA, g_cobalt.fbo_noalpha, "Main FBO have no alpha channel");
    }

		g_cobalt.es=ReturnEnvVarInt("COBALT_ES");
    switch(g_cobalt.es) {
      case 1:
      case 2:
      case 3:
        break;
      default:
        // automatic ES backend selection
        g_cobalt.es = DEFAULT_ES;
        break;
    }

    g_cobalt.gl=ReturnEnvVarInt("COBALT_GL");
    switch(g_cobalt.gl) {
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 20:
      case 21:
      case 30:
      case 31:
      case 32:
      case 33:
      case 40:
      case 41:
      case 42:
      case 43:
      case 44:
      case 45:
        break;
      default:
        // automatic GL version selection
        g_cobalt.gl = (g_cobalt.es==1)?15:21;  // forcing GL 1.5 for es1.1 and GL 2.1 for es2.0
        break;
    }

    COBALT_LOGD("Using GLES %s backend\n", (g_cobalt.es==1)?"1.1":"2.0");

    env(COBALT_NODEPTHTEX, g_cobalt.nodepthtex, "Disable usage of Depth Textures");

    const char* env_drmcard = GetEnvVar("COBALT_DRMCARD");
    if(env_drmcard) {
#ifdef NO_GBM
        COBALT_LOGD("Warning, GBM not compiled in, cannot use COBALT_DRMCARD\n");
#else
        strncpy(g_cobalt.drmcard, env_drmcard, 50);
    } else {
        strcpy(g_cobalt.drmcard, "/dev/dri/card0");
#endif
    }

#if !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
    load_libs();
#endif

#if (defined(NOEGL) && !defined(ANDROID) && !defined(__APPLE__)) || defined(__EMSCRIPTEN__)
    int cobalt_notest = !gles_getProcAddress;
#else
    int cobalt_notest = IsEnvVarTrue("COBALT_NOTEST");
#endif
    env(COBALT_NOHIGHP, g_cobalt.nohighp, "Do not use HIGHP in fragment shader even if detected");

    g_cobalt.floattex=ReturnEnvVarIntDef("COBALT_FLOAT", 1);
    switch(g_cobalt.floattex) {
      case 0:
        COBALT_LOGD("Float and Half-Float texture support disabled\n");
        break;
      case 2:
        COBALT_LOGD("Float and Half-float texture support forced\n");
        break;
      default:
        g_cobalt.floattex = 1;
        break;
    }

    GetHardwareExtensions(cobalt_notest);

#if !defined(NO_LOADER) && !defined(NO_GBM)
    if(g_cobalt.usegbm)
        LoadGBMFunctions();
    if(g_cobalt.usegbm && !(gbm && drm)) {
        COBALT_LOGD("cannot use GBM, disabling\n");
        g_cobalt.usegbm = 0;  // should do some smarter fallback?
    }
#else
    g_cobalt.usegbm = 0;
#endif
#if !defined(NOX11)
    glx_init();
#endif

    gl_init();

#ifdef COBALT_COMPILE_SHARED
    fpe_shader_reset_internals();
#ifdef AMIGAOS4
    agl_reset_internals();
#endif
#endif

    env(COBALT_RECYCLEFBO, g_cobalt.recyclefbo, "Recycling of FBO enabled");

    // Texture hacks
    g_cobalt.automipmap=ReturnEnvVarInt("COBALT_MIPMAP");
    switch(g_cobalt.automipmap) {
      case 1:
        COBALT_LOGD("AutoMipMap forced\n");
        break;
      case 2:
        COBALT_LOGD("guess AutoMipMap\n");
        break;
      case 3:
        COBALT_LOGD("ignore MipMap\n");
        break;
      case 4:
        COBALT_LOGD("ignore AutoMipMap on non-squared textures\n");
        break;
      case 5:
        COBALT_LOGD("Calculate sub-mipmap in case some are missing\n");
        break;
      default:
        g_cobalt.automipmap = 0;
        break;
    }

    if(IsEnvVarTrue("COBALT_TEXCOPY")) {
      g_cobalt.texcopydata = 1;
		  COBALT_LOGD("Texture copy enabled\n");
    }

    g_cobalt.texshrink=ReturnEnvVarInt("COBALT_SHRINK");
    switch(g_cobalt.texshrink) {
      case 10:
        COBALT_LOGD("Texture shink, mode 10 selected (advertise 8192 max texture size, but >2048 are quadshrinked and > 512 are shrinked), but not for empty texture\n");
        break;
      case 11:
        COBALT_LOGD("Texture shink, mode 11 selected (advertise a max texture size *2, but every texture with one dimension > max texture size will get shrinked to max texture size), but not for empty texture\n");
        break;
      case 1:
        COBALT_LOGD("Texture shink, mode 1 selected (everything / 2)\n");
        break;
      case 2:
        COBALT_LOGD("Texture shink, mode 2 selected (only > 512 /2 )\n");
        break;
      case 3:
        COBALT_LOGD("Texture shink, mode 3 selected (only > 256 /2 )\n");
        break;
      case 4:
        COBALT_LOGD("Texture shink, mode 4 selected (only > 256 /2, >=1024 /4 )\n");
        break;
      case 5:
        COBALT_LOGD("Texture shink, mode 5 selected (every > 256 is downscaled to 256 ), but not for empty texture\n");
        break;
      case 6:
        COBALT_LOGD("Texture shink, mode 6 selected (only > 128 /2, >=512 is downscaled to 256 ), but not for empty texture\n");
        break;
      case 7:
        COBALT_LOGD("Texture shink, mode 7 selected (only > 512 /2 ), but not for empty texture\n");
        break;
      case 8:
        COBALT_LOGD("Texture shink, mode 8 selected (advertise 8192 max texture size, but >2048 are shrinked to 2048)\n");
        break;
      case 9:
        COBALT_LOGD("Texture shink, mode 9 selected (advertise 8192 max texture size, but >4096 are quadshrinked and > 512 are shrinked), but not for empty texture\n");
        break;
      default:
      	g_cobalt.texshrink=0;
        break;
    }

    env(COBALT_TEXDUMP, g_cobalt.texdump, "Texture dump enabled");
    env(COBALT_ALPHAHACK, g_cobalt.alphahack, "Alpha Hack enabled");

#ifdef TEXSTREAM
    switch(ReturnEnvVarInt("COBALT_STREAM")) {
      case 1:
        g_cobalt.texstream = InitStreamingCache();
        COBALT_LOGD("Streaming texture %s\n",(g_cobalt.texstream)?"enabled":"not available");
        //FreeStreamed(AddStreamed(1024, 512, 0));
        break;
      case 2:
        g_cobalt.texstream = InitStreamingCache()?2:0;
        COBALT_LOGD("Streaming texture %s\n",(g_cobalt.texstream)?"forced":"not available");
        //FreeStreamed(AddStreamed(1024, 512, 0));
        break;
      default:
        break;
    }
#endif

    env(COBALT_NOLUMALPHA, g_cobalt.nolumalpha, "GL_LUMINANCE_ALPHA hardware support disabled");
    env(COBALT_BLENDHACK, g_cobalt.blendhack, "Change Blend GL_SRC_ALPHA, GL_ONE to GL_ONE, GL_ONE");
    env(COBALT_BLENDCOLOR, g_cobalt.blendcolor, "Export a (faked) glBlendColor");
    env(COBALT_NOERROR, g_cobalt.noerror, "glGetError() always return GL_NOERROR");

    g_cobalt.silentstub = 1;
    if(IsEnvVarInt("COBALT_SILENTSTUB",0)) {
      g_cobalt.silentstub = 0;
      COBALT_LOGD("Stub/non present functions are printed");
    }

    env(COBALT_VABGRA, g_cobalt.vabgra, "Export GL_ARB_vertex_array_bgra extension");

    const char *env_version = GetEnvVar("COBALT_VERSION");
    if (env_version) {
        COBALT_LOGD("Override version string with \"%s\" (should be in the form of \"1.x\")\n", env_version);
    }
    if(env_version) {
        snprintf(g_cobalt.version, 49, "%s cobalt wrapper %d.%d.%d", env_version, MAJOR, MINOR, REVISION);
        COBALT_LOGD("Targeting OpenGL %s\n", env_version);
    } else {
        snprintf(g_cobalt.version, 49, "%d.%d cobalt wrapper %d.%d.%d", g_cobalt.gl/10, g_cobalt.gl%10, MAJOR, MINOR, REVISION);
        COBALT_LOGD("Targeting OpenGL %d.%d\n", g_cobalt.gl/10, g_cobalt.gl%10);
    }

    if(hardext.srgb && IsEnvVarTrue("COBALT_SRGB")) {
        g_cobalt.glx_surface_srgb = 2;
        COBALT_LOGD("enabling sRGB support\n");
    }

    if(IsEnvVarTrue("COBALT_FASTMATH")) {
#if defined(PANDORA) || defined(CHIP) || defined(GOA_CLONE)
        COBALT_LOGD("Enable FastMath for cortex-a8\n");
        fast_math();
#else
        COBALT_LOGD("No FastMath on this platform\n");
#endif
    }

    switch(hardext.npot) {
        case 0: g_cobalt.npot = 0; break;
        case 1:
        case 2: g_cobalt.npot = 1; break;
        case 3: g_cobalt.npot = 2; break;
    }
    switch(ReturnEnvVarInt("COBALT_NPOT")) {
      case 1:
        if(g_cobalt.npot<1) {
    	    g_cobalt.npot = 1;
		      COBALT_LOGD("Expose limited NPOT extension\n");
        }
        break;
      case 2:
        if(g_cobalt.npot<3) {
    	    g_cobalt.npot = 2;
		      COBALT_LOGD("Expose GL_ARB_texture_non_power_of_two extension\n");
        }
        break;
    }

    if(IsEnvVarFalse("COBALT_GLQUERIES")) {
        g_cobalt.queries = 0;
        COBALT_LOGD("Don't expose fake glQueries functions\n");
    }
    if(IsEnvVarTrue("COBALT_NODOWNSAMPLING")) {
        g_cobalt.nodownsampling = 1;
        COBALT_LOGD("No downsampling of DXTc textures\n");
    }
    env(COBALT_NOTEXMAT, g_cobalt.texmat, "Don't handle Texture Matrice internaly");
    env(COBALT_NOVAOCACHE, g_cobalt.novaocache, "Don't use VAO cache");
    if(IsEnvVarTrue("COBALT_NOINTOVLHACK")) {
        g_cobalt.nointovlhack = 1;
        COBALT_LOGD("No hack in shader converter to define overloaded function with int\n");
    }
    if(IsEnvVarTrue("COBALT_NOSHADERLOD")) {
        g_cobalt.noshaderlod = 1;
        COBALT_LOGD("No GL_EXT_shader_texture_lod used even if present\n");
        hardext.shaderlod=0;
    }

    int env_begin_end;
    if(GetEnvVarInt("COBALT_BEGINEND",&env_begin_end,0)) {
	    switch(env_begin_end) {
	      case 0:
	        g_cobalt.beginend = 0;
	        g_cobalt.mergelist = 0;
	        COBALT_LOGD("Don't try to merge subsequent glBegin/glEnd blocks\n");
	        break;
	      case 1:
	      case 2:
	        g_cobalt.beginend = 1;
	        COBALT_LOGD("Try to merge subsequent glBegin/glEnd blocks, even if there is a glColor / glNormal in between\n");
	        break;
	    }
	  }

    if(GetEnvVarBool("COBALT_AVOID16BITS", &g_cobalt.avoid16bits, (hardext.vendor&VEND_IMGTEC)?0:1)) {
      if(g_cobalt.avoid16bits) {
        COBALT_LOGD("Avoid 16bits textures\n");
      } else {
        COBALT_LOGD("Don't avoid 16bits textures\n");
      }
    }

    if(GetEnvVarInt("COBALT_AVOID24BITS",&g_cobalt.avoid24bits,0)) {
      switch(g_cobalt.avoid24bits) {
	      case 0:
	        COBALT_LOGD("Don't try to avoid 24bits textures\n");
	        break;
	      case 1:
          g_cobalt.avoid24bits = 2;
          COBALT_LOGD("Avoid 24bits textures\n");
	        break;
	      default:
          g_cobalt.avoid24bits = 0;
	        break;
    	}
    }

    env(COBALT_FORCE16BITS, g_cobalt.force16bits, "Force 16bits textures");
    env(COBALT_POTFRAMEBUFFER, g_cobalt.potframebuffer, "Force framebuffers to be on POT size");

    int env_forcenpot=ReturnEnvVarIntDef("COBALT_FORCENPOT",0);
    if(env_forcenpot==0 && (hardext.esversion==2 && (hardext.npot==1 || hardext.npot==2))) {
      COBALT_LOGD("Not forcing NPOT support\n");
    } else if(env_forcenpot!=0 || (hardext.esversion==2 && (hardext.npot==1 || hardext.npot==2))) {
        if(hardext.npot==3) {
            COBALT_LOGD("NPOT texture handled in hardware\n");
        } else if(hardext.npot==1) {
            g_cobalt.forcenpot = 1;
            COBALT_LOGD("Forcing NPOT support by disabling MIPMAP support for NPOT textures \n");
        } else {
            COBALT_LOGD("WARNING, No Limited or Full NPOT support in hardware, Forcing NPOT have no effect!\n");
        }
    }

		#if defined(COBALT_COMPILE_SHARED) && defined(AMIGAOS4) // temporary workaround for not-working envs
   		g_cobalt.maxbatch = 40;
		#else
   		g_cobalt.maxbatch = 0;
   	#endif
    g_cobalt.minbatch = 0;
    int tmp = 0, tmp2 = 0;
    switch(GetEnvVarFmt("COBALT_BATCH","%d-%d",&tmp,&tmp2)) {
      case 2:
        g_cobalt.maxbatch = tmp2;
        g_cobalt.minbatch = tmp;
        if(g_cobalt.minbatch>g_cobalt.maxbatch) {
            g_cobalt.maxbatch = tmp;
            g_cobalt.minbatch = tmp2;
        }
        break;
      case 1:
        g_cobalt.maxbatch = 10*10*tmp;
        g_cobalt.minbatch = 0;
        break;
    }
    if(g_cobalt.maxbatch==0) {
        COBALT_LOGD("Not trying to batch small subsequent glDrawXXXX\n");
    } else {
        COBALT_LOGD("Trying to batch subsequent glDrawXXXX of size between %d and %d vertices\n", g_cobalt.minbatch, g_cobalt.maxbatch);
    }

    if(hardext.esversion==1) g_cobalt.usevbo=0;   // VBO on ES1.1 backend will be too messy, so disabling
    else {
	    g_cobalt.usevbo = ReturnEnvVarIntDef("COBALT_USEVBO",1);
	    switch(g_cobalt.usevbo) {
	      case 0:
	        COBALT_LOGD("Use of VBO disabled\n");
	        break;
	      case 1:
	        COBALT_LOGD("try to use VBO\n");
	        break;
	      case 2:
	        COBALT_LOGD("try to use VBO (also with glLockArrays)\n");
	        break;
	      case 3:
	        COBALT_LOGD("try to use VBO (special glLockArrays case for idtech3 engine)\n");
	      	break;
	      default:
	      	g_cobalt.usevbo=1;
	      	break;
	    }
	  }

    g_cobalt.fbomakecurrent = 0;
    if((hardext.vendor & VEND_ARM) || (g_cobalt.usefb))
        g_cobalt.fbomakecurrent = 1;
    switch(ReturnEnvVarIntDef("COBALT_FBOMAKECURRENT",-1)) {
      case 0:
        if(g_cobalt.fbomakecurrent) {
          g_cobalt.fbomakecurrent = 0;
          COBALT_LOGD("glXMakeCurrent FBO workaround disabled\n");
        }
        break;
      case 1:
        g_cobalt.fbomakecurrent = 1;
        break;
    }
    if(g_cobalt.fbomakecurrent) {
        COBALT_LOGD("glXMakeCurrent FBO workaround enabled\n");
    }


    g_cobalt.fbounbind = 0;
    if((hardext.vendor & VEND_ARM) || (hardext.vendor & VEND_IMGTEC))
        g_cobalt.fbounbind = 1;
    switch(ReturnEnvVarIntDef("COBALT_FBOUNBIND",-1)) {
      case 0:
        if(g_cobalt.fbounbind) {
          g_cobalt.fbounbind = 0;
          COBALT_LOGD("FBO workaround for using binded texture disabled\n");
        }
        break;
      case 1:
        g_cobalt.fbounbind = 1;
        break;
    }
    if(g_cobalt.fbounbind) {
        COBALT_LOGD("FBO workaround for using binded texture enabled\n");
    }

    g_cobalt.fboforcetex = 1;
    GetEnvVarInt("COBALT_FBOFORCETEX", &g_cobalt.fboforcetex, g_cobalt.fboforcetex);
    if(g_cobalt.fboforcetex)
      COBALT_LOGD("Force texture for Attachment color0 on FBO\n");
    g_cobalt.blitfullscreen = 1;
    GetEnvVarInt("COBALT_BLITFULLSCREEN", &g_cobalt.blitfullscreen, g_cobalt.blitfullscreen);
    if(g_cobalt.blitfullscreen)
      COBALT_LOGD("Hack to trigger a SwapBuffers when a Full Framebuffer Blit on default FBO is done\n");

    env(COBALT_COMMENTS, g_cobalt.comments, "Keep comments in converted Shaders");

    env(COBALT_NOARBPROGRAM, g_cobalt.noarbprogram, "Not exposing ARB Program extensions");

    if(hardext.npot==3)
        g_cobalt.defaultwrap=0;
    else
        g_cobalt.defaultwrap=1;

    if(GetEnvVarInt("COBALT_DEFAULTWRAP",&g_cobalt.defaultwrap,(hardext.npot==3) ? 0 : 1)) {
    	switch(g_cobalt.defaultwrap) {
    		case 0:
          COBALT_LOGD("Default wrap mode is GL_REPEAT\n");
    			break;
    		case 1:
          COBALT_LOGD("Default wrap mode is GL_CLAMP_TO_EDGE\n");
    			break;
    		case 2:
    		default:
    			g_cobalt.defaultwrap=2;
          COBALT_LOGD("Default wrap mode is GL_CLAMP_TO_EDGE, enforced\n");
    			break;
    	}
    }


    GetEnvVarBool("COBALT_NOTEXARRAY",&g_cobalt.notexarray,0);
    if(g_cobalt.notexarray) {
        COBALT_LOGD("No Texture Array in Shaders\n");
    }

    env(COBALT_LOGSHADERERROR, g_cobalt.logshader, "Log to the console Error compiling shaders");
    env(COBALT_SHADERNOGLES, g_cobalt.shadernogles, "Remove GLES part in shader");
    env(COBALT_NOES2COMPAT, g_cobalt.noes2, "Don't expose GLX_EXT_create_context_es2_profile extension");
    env(COBALT_NORMALIZE, g_cobalt.normalize, "Force normals to be normalized on FPE shaders");

    g_cobalt.dbgshaderconv=ReturnEnvVarIntDef("COBALT_DBGSHADERCONV",0);
    if(g_cobalt.dbgshaderconv) {
      if(g_cobalt.dbgshaderconv==1)
          g_cobalt.dbgshaderconv=15;
      if(!(g_cobalt.dbgshaderconv&3))   // neither vertex or fragment
          g_cobalt.dbgshaderconv|=3;    // select both
      if(!(g_cobalt.dbgshaderconv&12))  // neither before or after
          g_cobalt.dbgshaderconv|=12;   // select both
      COBALT_LOGD_NOPREFIX("Log to the console all shaders before and after conversion: ");
      if(g_cobalt.dbgshaderconv&4)
          COBALT_LOGD_NOPREFIX("Before  ");
      if(g_cobalt.dbgshaderconv&8)
          COBALT_LOGD_NOPREFIX("After  ");
      if(g_cobalt.dbgshaderconv&1)
          COBALT_LOGD_NOPREFIX("Vertex  ");
      if(g_cobalt.dbgshaderconv&2)
          COBALT_LOGD_NOPREFIX("Fragment  ");
      COBALT_LOGD_NOPREFIX("\n");
    }

    // VGPU pipeline stuff

    GetEnvVarInt("COBALT_VGPU_DUMP", &g_cobalt.vgpu_dump, 0);
    if(g_cobalt.vgpu_dump > 0)
        COBALT_LOGD_NOPREFIX("Dump the content of VGPU shader conversion\n");
    if(g_cobalt.vgpu_dump > 1)
        COBALT_LOGD_NOPREFIX("Dump the shaders at every vgpu stage\n");

    GetEnvVarInt("COBALT_VGPU_OPT", &g_cobalt.vgpu_optimization, 2);
    if (!g_cobalt.vgpu_optimization) {
        COBALT_LOGD_NOPREFIX("Do not optimize the shaders\n");
    }

    GetEnvVarInt("COBALT_VGPU_STUBLAND", &g_cobalt.vgpu_stubland, 0);
    if (g_cobalt.vgpu_stubland) {
        COBALT_LOGD_NOPREFIX("Use descriptive stubs\n");
    }

    env(COBALT_VGPU_FORCE, g_cobalt.vgpu_force_conv, "Force VGPU pipeline to convert every shader")
    env(COBALT_VGPU_BACKPORT, g_cobalt.vgpu_backport, "Attempt HARD to backport shaders to #version 100")
    env(COBALT_BUFFER_FORCE_BIND, g_cobalt.buffer_force_bind, "Will rebind buffers before each use !")
    g_cobalt.vgpu_precision = ReturnEnvVarIntDef("COBALT_VGPU_PRECISION", 0);
    if(g_cobalt.vgpu_precision != 0) {
        COBALT_LOGD("VGPU default precision overridden ! (%i)", g_cobalt.vgpu_precision);
    }


    env(COBALT_NOCLEAN, g_cobalt.noclean, "Don't clean Context when destroy");

    g_cobalt.glxrecycle = 1;
#ifndef NOEGL
    if((g_cobalt.usepbuffer) || (g_cobalt.usefb))
        g_cobalt.glxrecycle = 0;

    int env_glxrecycle=ReturnEnvVarIntDef("COBALT_GLXRECYCLE",-1);
    if(g_cobalt.glxrecycle && env_glxrecycle==0 && !((g_cobalt.usepbuffer) || (g_cobalt.usefb))) {
        g_cobalt.glxrecycle = 0;
        COBALT_LOGD("glX Will NOT try to recycle EGL Surface\n");
    }
    if(env_glxrecycle==1)
        g_cobalt.glxrecycle = 1;
    if(g_cobalt.glxrecycle) {
        COBALT_LOGD("glX Will try to recycle EGL Surface\n");
    }
    env(COBALT_GLXNATIVE, g_cobalt.glxnative, "Don't filter GLXConfig with GLX_X_NATIVE_TYPE");
#endif
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))!= NULL)
        COBALT_LOGD("Current folder is:%s\n", cwd);

    if(hardext.shader_fbfetch) {
      env(COBALT_SHADERBLEND, g_cobalt.shaderblend, "Blend will be handle in shaders");
    }
    if(hardext.prgbin_n>0 && !g_cobalt.notexarray) {
        env(COBALT_NOPSA, g_cobalt.nopsa, "Don't use PrecompiledShaderArchive");
        if(g_cobalt.nopsa==0) {
            cwd[0]='\0';
            // TODO: What to do on ANDROID and EMSCRIPTEN?
            const char* custom_psa = GetEnvVar("COBALT_PSA_FOLDER");

            if(custom_psa)
                strcpy(cwd, custom_psa);

#if defined(__linux__)
            const char* home = GetEnvVar("HOME");
            if(home && !custom_psa)
                strcpy(cwd, home);
#elif defined AMIGAOS4
            if(!custom_psa)
              strcpy(cwd, "PROGDIR:");
#endif

            if(strlen(cwd)) {
#if defined(__linux__) || defined(ANDROID) || defined(__ANDROID__)
                if(cwd[strlen(cwd)]!='/')
                    strcat(cwd, "/");
#endif
                strcat(cwd, ".cobalt.psa");
                fpe_InitPSA(cwd);
                fpe_readPSA();
            }
        }
    } else 
      COBALT_LOGD("Not using PSA (prgbin_n=%d, notexarray=%d)\n", hardext.prgbin_n, g_cobalt.notexarray);

    env(COBALT_SKIPTEXCOPIES, g_cobalt.skiptexcopies, "Texture Copies will be skipped");
    if(GetEnvVarFloat("COBALT_FB_TEX_SCALE",&g_cobalt.fbtexscale,0.0f)) {
      COBALT_LOGD("Framebuffer Textures will be scaled by %.2f\n", g_cobalt.fbtexscale);
		}
}


#ifndef NOX11
void FreeFBVisual();
#endif
#ifdef NO_INIT_CONSTRUCTOR
EXPORT
#else
#ifdef BUILD_WINDOWS_DLL
EXPORT // symmetric for init -- trivialize application code
#endif
#if !defined(_MSC_VER) || defined(__clang__)
__attribute__((destructor))
#endif
#endif
void cobalt_shutdown() {
		#ifdef COBALT_COMPILE_SHARED
	    COBALT_LOGD("Shuting down request\n");
    	if(--inited) return;
    #endif
    COBALT_LOGD("Shuting down\n");
    #ifndef NOX11
    FreeFBVisual();
    #endif
    gl_close();
    fpe_writePSA();
    fpe_FreePSA();
		#if defined(COBALT_COMPILE_SHARED) && defined(AMIGAOS4)
	    os4CloseLib();
	  #endif
}

#ifdef BUILD_WINDOWS_DLL
#if !defined(_MSC_VER) || defined(__clang__)
__attribute__((constructor(103)))
#endif
void dll_init_done()
{ dll_inited = 1; }
#endif

#if defined(_MSC_VER) && !defined(NO_INIT_CONSTRUCTOR) && !defined(__clang__)
#pragma const_seg(".CRT$XCU")
void (*const cobalt_ctors[])() = { cobalt_init, dll_init_done };
#pragma const_seg(".CRT$XTX")
void (*const cobalt_dtor)() = cobalt_shutdown;
#pragma const_seg()
#endif

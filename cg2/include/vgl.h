#ifndef __VGL_H__
#define __VGL_H__

// #define USE_GL3W

#ifdef USE_GL3W

#include <GL3/gl3.h>
#include <GL3/gl3w.h>

#else

#define GLEW_STATIC

#include <GL/glew.h>

#ifdef _MSC_VER
#  ifdef _DEBUG
       pragma comment (lib, "libglew32d.lib")
#  else
#      pragma comment (lib, "libglew32.lib")
#endif
#endif

#define FREEGLUT_STATIC

#include <GL/freeglut.h>

#ifdef _MSC_VER
#  ifdef _DEBUG
#	pragma comment (lib, "freeglut_staticd.lib")
#  else
#	pragma comment (lib, "freeglut_static.lib")
#  endif
#endif

#endif

#define BUFFER_OFFSET(x)  ((const void*) (x))

#endif /* __VGL_H__ */

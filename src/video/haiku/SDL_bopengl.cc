/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_HAIKU && SDL_VIDEO_OPENGL

#include "SDL_bopengl.h"

#include <unistd.h>
#include <KernelKit.h>
#include <OpenGLKit.h>
#include "SDL_BWin.h"
#include "../../main/haiku/SDL_BApp.h"

#ifdef __cplusplus
extern "C" {
#endif

static SDL_INLINE SDL_BWin *_ToBeWin(SDL_Window *window) {
    return ((SDL_BWin*)(window->driverdata));
}

int
HAIKU_GLES_LoadLibrary(_THIS, const char *path) {
    if (_this->egl_data == NULL) {
        printf("HAIKU_GLES_LoadLibrary(\"%s\")\n", path);
        path = "libEGL.so";
        int res = SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
        printf("  res: %d\n", res);
        printf("  _this->egl_data: %p\n", _this->egl_data);
        printf("  _this->egl_data->dll_handle: %p\n", _this->egl_data->dll_handle);
    }
    return 0;
}

int HAIKU_GLES_SwapWindow(_THIS, SDL_Window * window) {
    return SDL_EGL_SwapBuffers(_this, _ToBeWin(window)->EglSurface());
}

int HAIKU_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context) {
    return SDL_EGL_MakeCurrent(_this, window ? _ToBeWin(window)->EglSurface() : EGL_NO_SURFACE, context);
}

SDL_GLContext HAIKU_GLES_CreateContext(_THIS, SDL_Window * window) {
    printf("HAIKU_GLES_CreateContext\n");
   	int32 prevRefs = atomic_add(&_ToBeWin(window)->EglSurfaceRefs(), 1);
    printf("  prevRefs: %" B_PRIu32 "\n", prevRefs);
    if (prevRefs == 0) {
        if (SDL_EGL_ChooseConfig(_this) != 0) {
            printf("SDL_EGL_ChooseConfig failed\n");
            return NULL;
        }
        _ToBeWin(window)->EglSurface() = _this->egl_data->eglCreateWindowSurface(
            _this->egl_data->egl_display,
            _this->egl_data->egl_config,
            (NativeWindowType)_ToBeWin(window)->GetView()->GetBitmapHook(), NULL);
        if (_ToBeWin(window)->EglSurface() == EGL_NO_SURFACE) {
            SDL_EGL_SetError("unable to create an EGL window surface", "eglCreateWindowSurface");
            atomic_add(&_ToBeWin(window)->EglSurfaceRefs(), -1);
            return NULL;
        }
        printf("_ToBeWin(window)->EglSurface(): %p\n", _ToBeWin(window)->EglSurface());
    }
    return SDL_EGL_CreateContext(_this, _ToBeWin(window)->EglSurface());
}

void HAIKU_GLES_DeleteContext(_THIS, SDL_GLContext context) {
   EGLContext egl_context = (EGLContext) context;

   if (egl_context == NULL || egl_context == EGL_NO_CONTEXT) {
       return;
   }

   _this->egl_data->eglDestroyContext(_this->egl_data->egl_display, egl_context);

#if 0
   int32 prevRefs = atomic_add(&_ToBeWin(window)->EglSurfaceRefs(), -1);
   if (prevRefs == 1) {
       eglDestroySurface(_ToBeWin(window)->EglSurface());
       _ToBeWin(window)->EglSurface() = NULL;
   }
#endif
}


#ifdef __cplusplus
}
#endif

#endif /* SDL_VIDEO_DRIVER_HAIKU && SDL_VIDEO_OPENGL */

/* vi: set ts=4 sw=4 expandtab: */

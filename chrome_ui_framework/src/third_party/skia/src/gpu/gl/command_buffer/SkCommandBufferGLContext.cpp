
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <EGL/egl.h>

#include "SkOnce.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/command_buffer/SkCommandBufferGLContext.h"
#include "../ports/SkOSLibrary.h"

typedef EGLDisplay (*GetDisplayProc)(EGLNativeDisplayType display_id);
typedef EGLBoolean (*InitializeProc)(EGLDisplay dpy, EGLint *major, EGLint *minor);
typedef EGLBoolean (*TerminateProc)(EGLDisplay dpy);
typedef EGLBoolean (*ChooseConfigProc)(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
typedef EGLBoolean (*GetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value);
typedef EGLSurface (*CreateWindowSurfaceProc)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
typedef EGLSurface (*CreatePbufferSurfaceProc)(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list);
typedef EGLBoolean (*DestroySurfaceProc)(EGLDisplay dpy, EGLSurface surface);
typedef EGLContext (*CreateContextProc)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
typedef EGLBoolean (*DestroyContextProc)(EGLDisplay dpy, EGLContext ctx);
typedef EGLBoolean (*MakeCurrentProc)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
typedef EGLBoolean (*SwapBuffersProc)(EGLDisplay dpy, EGLSurface surface);
typedef __eglMustCastToProperFunctionPointerType (*GetProcAddressProc)(const char* procname);

static GetDisplayProc gfGetDisplay = nullptr;
static InitializeProc gfInitialize = nullptr;
static TerminateProc gfTerminate = nullptr;
static ChooseConfigProc gfChooseConfig = nullptr;
static GetConfigAttrib gfGetConfigAttrib = nullptr;
static CreateWindowSurfaceProc gfCreateWindowSurface = nullptr;
static CreatePbufferSurfaceProc gfCreatePbufferSurface = nullptr;
static DestroySurfaceProc gfDestroySurface = nullptr;
static CreateContextProc gfCreateContext = nullptr;
static DestroyContextProc gfDestroyContext = nullptr;
static MakeCurrentProc gfMakeCurrent = nullptr;
static SwapBuffersProc gfSwapBuffers = nullptr;
static GetProcAddressProc gfGetProcAddress = nullptr;

static void* gLibrary = nullptr;
static bool gfFunctionsLoadedSuccessfully = false;

static void load_command_buffer_functions() {
    if (!gLibrary) {
#if defined _WIN32
        gLibrary = DynamicLoadLibrary("command_buffer_gles2.dll");
#else
        gLibrary = DynamicLoadLibrary("libcommand_buffer_gles2.so");
#endif // defined _WIN32
        if (gLibrary) {
            gfGetDisplay = (GetDisplayProc)GetProcedureAddress(gLibrary, "eglGetDisplay");
            gfInitialize = (InitializeProc)GetProcedureAddress(gLibrary, "eglInitialize");
            gfTerminate = (TerminateProc)GetProcedureAddress(gLibrary, "eglTerminate");
            gfChooseConfig = (ChooseConfigProc)GetProcedureAddress(gLibrary, "eglChooseConfig");
            gfGetConfigAttrib = (GetConfigAttrib)GetProcedureAddress(gLibrary, "eglGetConfigAttrib");
            gfCreateWindowSurface = (CreateWindowSurfaceProc)GetProcedureAddress(gLibrary, "eglCreateWindowSurface");
            gfCreatePbufferSurface = (CreatePbufferSurfaceProc)GetProcedureAddress(gLibrary, "eglCreatePbufferSurface");
            gfDestroySurface = (DestroySurfaceProc)GetProcedureAddress(gLibrary, "eglDestroySurface");
            gfCreateContext = (CreateContextProc)GetProcedureAddress(gLibrary, "eglCreateContext");
            gfDestroyContext = (DestroyContextProc)GetProcedureAddress(gLibrary, "eglDestroyContext");
            gfMakeCurrent = (MakeCurrentProc)GetProcedureAddress(gLibrary, "eglMakeCurrent");
            gfSwapBuffers = (SwapBuffersProc)GetProcedureAddress(gLibrary, "eglSwapBuffers");
            gfGetProcAddress = (GetProcAddressProc)GetProcedureAddress(gLibrary, "eglGetProcAddress");

            gfFunctionsLoadedSuccessfully = gfGetDisplay && gfInitialize && gfTerminate &&
                                            gfChooseConfig && gfCreateWindowSurface &&
                                            gfCreatePbufferSurface && gfDestroySurface &&
                                            gfCreateContext && gfDestroyContext && gfMakeCurrent &&
                                            gfSwapBuffers && gfGetProcAddress;

        }                        
    }                            
}

static GrGLFuncPtr command_buffer_get_gl_proc(void* ctx, const char name[]) {
    GrGLFuncPtr proc = (GrGLFuncPtr) GetProcedureAddress(ctx, name);
    if (proc) {
        return proc;
    }
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    return gfGetProcAddress(name);
}

SK_DECLARE_STATIC_ONCE(loadCommandBufferOnce);
void LoadCommandBufferOnce() {
    SkOnce(&loadCommandBufferOnce, load_command_buffer_functions);
}

const GrGLInterface* GrGLCreateCommandBufferInterface() {
    LoadCommandBufferOnce();
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    return GrGLAssembleGLESInterface(gLibrary, command_buffer_get_gl_proc);
}

SkCommandBufferGLContext::SkCommandBufferGLContext()
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {

    static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    initializeGLContext(nullptr, configAttribs, surfaceAttribs);
}

SkCommandBufferGLContext::SkCommandBufferGLContext(void* nativeWindow, int msaaSampleCount) {
    static const EGLint surfaceAttribs[] = { EGL_NONE };

    EGLint configAttribs[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     8,
        EGL_STENCIL_SIZE,   8,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES,        msaaSampleCount,
        EGL_NONE
    };
    if (msaaSampleCount == 0) {
        configAttribs[12] = EGL_NONE;
    }

    initializeGLContext(nativeWindow, configAttribs, surfaceAttribs);
}

void SkCommandBufferGLContext::initializeGLContext(void* nativeWindow, const int* configAttribs,
                                                   const int* surfaceAttribs) {
    LoadCommandBufferOnce();
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }

    fDisplay = gfGetDisplay(static_cast<EGLNativeDisplayType>(EGL_DEFAULT_DISPLAY));
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Could not create EGL display!");
        return;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    gfInitialize(fDisplay, &majorVersion, &minorVersion);

    EGLConfig surfaceConfig = static_cast<EGLConfig>(fConfig);
    EGLint numConfigs;
    gfChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs);

    if (nativeWindow) {
        fSurface = gfCreateWindowSurface(fDisplay, surfaceConfig, 
                                         (EGLNativeWindowType)nativeWindow, surfaceAttribs);
    } else {
        fSurface = gfCreatePbufferSurface(fDisplay, surfaceConfig, surfaceAttribs);
    }

    static const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    fContext = gfCreateContext(fDisplay, surfaceConfig, nullptr, contextAttribs);

    gfMakeCurrent(fDisplay, fSurface, fSurface, fContext);

    SkAutoTUnref<const GrGLInterface> gl(GrGLCreateCommandBufferInterface());
    if (nullptr == gl.get()) {
        SkDebugf("Could not create CommandBuffer GL interface!\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Could not validate CommandBuffer GL interface!\n");
        this->destroyGLContext();
        return;
    }

    this->init(gl.detach());
}

SkCommandBufferGLContext::~SkCommandBufferGLContext() {
    this->teardown();
    this->destroyGLContext();
}

void SkCommandBufferGLContext::destroyGLContext() {
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (fDisplay) {
        gfMakeCurrent(fDisplay, 0, 0, 0);

        if (fContext) {
            gfDestroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (fSurface) {
            gfDestroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        gfTerminate(fDisplay);
        fDisplay = EGL_NO_DISPLAY;
    }
}

void SkCommandBufferGLContext::onPlatformMakeCurrent() const {
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (!gfMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void SkCommandBufferGLContext::onPlatformSwapBuffers() const {
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (!gfSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete gfSwapBuffers.\n");
    }
}

GrGLFuncPtr SkCommandBufferGLContext::onPlatformGetProcAddress(const char* name) const {
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    return gfGetProcAddress(name);
}

void SkCommandBufferGLContext::presentCommandBuffer() {
    if (this->gl()) {
        this->gl()->fFunctions.fFlush();
    }

    this->onPlatformSwapBuffers();
}

bool SkCommandBufferGLContext::makeCurrent() {
    return gfMakeCurrent(fDisplay, fSurface, fSurface, fContext) != EGL_FALSE;
}

int SkCommandBufferGLContext::getStencilBits() {
    EGLint result = 0;
    EGLConfig surfaceConfig = static_cast<EGLConfig>(fConfig);
    gfGetConfigAttrib(fDisplay, surfaceConfig, EGL_STENCIL_SIZE, &result);
    return result;
}

int SkCommandBufferGLContext::getSampleCount() {
    EGLint result = 0;
    EGLConfig surfaceConfig = static_cast<EGLConfig>(fConfig);
    gfGetConfigAttrib(fDisplay, surfaceConfig, EGL_SAMPLES, &result);
    return result;
}

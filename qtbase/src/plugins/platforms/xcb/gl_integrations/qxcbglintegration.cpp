/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qxcbglintegration.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQpaGl, "qt.qpa.gl")

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <EGL/egl.h>

int DetectOpenGLCapability::EGLtestGLESCapability()
{
     Display    *x_display;
     Window      win;
     EGLDisplay  egl_display;
     EGLContext  egl_context;
     EGLSurface  egl_surface;
     EGLenum     current_api;

     x_display = XOpenDisplay(nullptr);
     if (x_display == nullptr) {
         return 0;
     }
     Window root = DefaultRootWindow(x_display);
     XSetWindowAttributes swa;
     swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
     win = XCreateWindow(x_display, root, 0, 0, 800, 480,   0,
                                                  CopyFromParent, InputOutput,
                                                  CopyFromParent, CWEventMask,
                                                  &swa );
     egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
     if (egl_display == EGL_NO_DISPLAY ) {
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return 0;
     }
     if (!eglInitialize(egl_display, nullptr, nullptr)) {
        eglTerminate(egl_display);
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return 0;
     }
     current_api = eglQueryAPI();
     eglBindAPI(EGL_OPENGL_ES_API);
     if (current_api == EGL_NONE) {
        // do not support OpenGL ES at all
        return 0;
     }
     EGLint attr[] = {       // some attributes to set up our egl-interface
         EGL_BUFFER_SIZE, 16,
         EGL_RENDERABLE_TYPE,
         EGL_OPENGL_ES_BIT,
         EGL_NONE
     };
     EGLConfig  ecfg;
     EGLint     num_config;
     if (!eglChooseConfig(egl_display, attr, &ecfg, 1, &num_config)) {
        eglTerminate(egl_display);
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return 0;
     }
     if (num_config != 1) {
        eglTerminate(egl_display);
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return 0;
     }
     egl_surface = eglCreateWindowSurface ( egl_display, ecfg, win, NULL );
     if (egl_surface == EGL_NO_SURFACE) {
        eglDestroySurface(egl_display, egl_surface);
        eglTerminate(egl_display);
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return 0;
     }

     EGLint ctxattr[] = {
         EGL_CONTEXT_CLIENT_VERSION, 2,
         EGL_NONE
     };
     egl_context = eglCreateContext (egl_display, ecfg, EGL_NO_CONTEXT, ctxattr);
     if (egl_context == EGL_NO_CONTEXT) {
        eglDestroySurface(egl_display, egl_surface);
        eglTerminate(egl_display);
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return 0;
     }

     eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
     const QString strShadinglanguage = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
     const QString strVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
     const QString strRenderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
     eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

     eglDestroyContext(egl_display, egl_context);
     eglDestroySurface(egl_display, egl_surface);
     eglBindAPI(current_api);
     eglTerminate(egl_display);
     XDestroyWindow(x_display, win);
     XCloseDisplay(x_display);

    if (strVersion.isNull() || strRenderer.isEmpty() || strShadinglanguage.isEmpty())
        return 0;
    if (strVersion.contains(QStringLiteral("Mesa")) || strRenderer.contains(QStringLiteral("llvm")))
        return 1;
    else
        return 2;
}
int DetectOpenGLCapability::GLXtestOpenGLCapability()
{
    Display *dpy;
    Window root;
    XVisualInfo *vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    Window win;
    GLXFBConfig *fc;
    GLXContext glc;
    int att[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT,
              GLX_DOUBLEBUFFER, True,
              GLX_DEPTH_SIZE, 16,
              None};
    int nelements;

    dpy = XOpenDisplay(nullptr);

    if (dpy == nullptr) {
        return 0;
    }

    fc = glXChooseFBConfig(dpy, 0, att, &nelements);

    if (fc == nullptr) {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        return 0;
    }

    vi = glXGetVisualFromFBConfig(dpy, *fc);

    if (vi == nullptr) {
        XFree(fc);
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        return 0;
    }

    root = DefaultRootWindow(dpy);
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;
    win = XCreateWindow(dpy, root, 0, 0, 1, 1,
                 0, vi->depth,
                 InputOutput, vi->visual,
                 CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);

    glc = glXCreateNewContext(dpy, *fc, GLX_RGBA_TYPE, nullptr, GL_TRUE);
    if (!glc){
        XFree(fc);
        XFree(vi);
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        return 0;
    }

    glXMakeContextCurrent(dpy, win, win, glc);
    const QString strShadinglanguage = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    const QString strVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    const QString strRenderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    glXMakeCurrent(dpy, None, nullptr);

    XFree(fc);
    XFree(vi);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    if (strVersion.isNull() || strRenderer.isEmpty() || strShadinglanguage.isEmpty())
        return 0;
    if (strRenderer.contains(QStringLiteral("softpipe")) || strRenderer.contains(QStringLiteral("llvmpipe")))
        return 1;
    else
        return 2;
}

DetectOpenGLCapability::DetectOpenGLCapability()
{
    m_renderableType = QSurfaceFormat::DefaultRenderableType;
    //test opengl
    int openglCap = GLXtestOpenGLCapability();
    //test gles
    int glesCap = EGLtestGLESCapability();
    if (openglCap == 2) {
        m_renderableType =  QSurfaceFormat::OpenGL;
    }
    else if (glesCap == 2){
        m_renderableType =  QSurfaceFormat::OpenGLES;
    }
    else if (openglCap == 1) {
        m_renderableType =  QSurfaceFormat::OpenGL;
    }
    else if (glesCap == 1){
        m_renderableType =  QSurfaceFormat::OpenGLES;
    }
    else {
        qWarning() << "this platform don't support GL";
    }
}

QXcbGlIntegration::QXcbGlIntegration()
{
}
QXcbGlIntegration::~QXcbGlIntegration()
{
}

bool QXcbGlIntegration::handleXcbEvent(xcb_generic_event_t *event, uint responseType)
{
    Q_UNUSED(event);
    Q_UNUSED(responseType);
    return false;
}

QT_END_NAMESPACE

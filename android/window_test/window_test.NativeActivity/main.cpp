// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

#include "android_native_app_glue.h"

#include <flame/window.h>

struct android_app* state_;

EGLDisplay display_;
EGLSurface surface_;
EGLContext context_;
int32_t width_;
int32_t height_;

static void engine_init_display() {
	const EGLint attribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);
	std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
	assert(supportedConfigs);
	eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
	assert(numConfigs);
	auto i = 0;
	for (; i < numConfigs; i++) {
		auto& cfg = supportedConfigs[i];
		EGLint r, g, b, d;
		if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r) &&
			eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
			eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b) &&
			eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
			r == 8 && g == 8 && b == 8 && d == 0) {

			config = supportedConfigs[i];
			break;
		}
	}
	if (i == numConfigs) {
		config = supportedConfigs[0];
	}

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	surface = eglCreateWindowSurface(display, config, state_->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	display_ = display;
	context_ = context;
	surface_ = surface;
	width_ = w;
	height_ = h;

	LOGW("width %d", w);
	LOGW("height %d", h);

	auto opengl_info = { GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS };
	for (auto name : opengl_info) {
		auto info = glGetString(name);
		LOGI("OpenGL Info: %s", info);
	}

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	return;
}

static void engine_draw_frame() {
	if (display_ == NULL) {
		return;
	}

	static float v = 0.f;
	glClearColor(v, v, v, 1);
	v += 0.1f;
	if (v >= 1.f)
		v = 0.f;
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(display_, surface_);
}

using namespace flame;

void android_main(struct android_app* state) {
	auto app = Application::create();
	Window::create(app, state, engine_init_display);

	state_ = state;

	app->run([]() {
		engine_draw_frame();
	});
}

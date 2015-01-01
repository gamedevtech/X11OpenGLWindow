// g++ -o X11Window X11.cpp -lX11 -lGL -lGLEW -L/usr/X11/lib -I/opt/X11/include
#include <iostream>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <sys/time.h>
#include <unistd.h>

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
#define FPS 30
#define TEST_LOCAL

extern bool Initialize(int w, int h);
extern bool Update(float deltaTime);
extern void Render();
extern void Resize(int w, int h);
extern void Shutdown();

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

#define SKIP_TICKS      (1000 / FPS)

static double GetMilliseconds() {
	static timeval s_tTimeVal;
	gettimeofday(&s_tTimeVal, NULL);
	double time = s_tTimeVal.tv_sec * 1000.0; // sec to ms
	time += s_tTimeVal.tv_usec / 1000.0; // us to ms
	return time;
}

static bool isExtensionSupported(const char *extList, const char *extension) {
	return strstr(extList, extension) != 0;
}

int main(int argc, char** argv) {
	Display* display;
	Window window;
	Screen* screen;
	int screenId;
	XEvent ev;

	// Open the display
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		std::cout << "Could not open display\n";
		return 1;
	}
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);
	
	// Check GLX version
	GLint majorGLX, minorGLX = 0;
	glXQueryVersion(display, &majorGLX, &minorGLX);
	if (majorGLX <= 1 && minorGLX < 2) {
		std::cout << "GLX 1.2 or greater is required.\n";
		XCloseDisplay(display);
		return 1;
	}

	GLint glxAttribs[] = {
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		None
	};
	
	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(display, screenId, glxAttribs, &fbcount);
	if (fbc == 0) {
		std::cout << "Failed to retrieve framebuffer.\n";
		XCloseDisplay(display);
		return 1;
	}

	// Pick the FB config/visual with the most samples per pixel
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
		if ( vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

			if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) ) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree( vi );
	}
	GLXFBConfig bestFbc = fbc[ best_fbc ];
	XFree( fbc ); // Make sure to free this!

	
	XVisualInfo* visual = glXGetVisualFromFBConfig( display, bestFbc );
	if (visual == 0) {
		std::cout << "Could not create correct visual window.\n";
		XCloseDisplay(display);
		return 1;
	}
	
	if (screenId != visual->screen) {
		std::cout << "screenId(" << screenId << ") does not match visual->screen(" << visual->screen << ").\n";
		XCloseDisplay(display);
		return 1;

	}

	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Redirect Close
	Atom atomWmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &atomWmDeleteWindow, 1);

	// Create GLX OpenGL context
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	GLXContext context = 0;
	const char *glxExts = glXQueryExtensionsString( display,  screenId );
	if (!isExtensionSupported( glxExts, "GLX_ARB_create_context")) {
		std::cout << "GLX_ARB_create_context not supported\n";
		context = glXCreateNewContext( display, bestFbc, GLX_RGBA_TYPE, 0, True );
	}
	else {
		context = glXCreateContextAttribsARB( display, bestFbc, 0, true, context_attribs );
	}
	XSync( display, False );

	// Verifying that context is a direct context
	if (!glXIsDirect (display, context)) {
		std::cout << "Indirect GLX rendering context obtained\n";
	}
	else {
		std::cout << "Direct GLX rendering context obtained\n";
	}
	glXMakeCurrent(display, window, context);

	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	if (!Initialize(WINDOW_WIDTH, WINDOW_HEIGHT)) {
		glXDestroyContext(display, context);
		XFree(visual);
		XFreeColormap(display, windowAttribs.colormap);
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		return 1;
	}

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

	double prevTime = GetMilliseconds();
	double currentTime = GetMilliseconds();
	double deltaTime = 0.0;

	timeval time;
	long sleepTime = 0;
	gettimeofday(&time, NULL);
	long nextGameTick = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	// Enter message loop
	while (true) {
		if (XPending(display) > 0) {
			XNextEvent(display, &ev);
			if (ev.type == Expose) {
				XWindowAttributes attribs;
				XGetWindowAttributes(display, window, &attribs);
				Resize(attribs.width, attribs.height);
			}
			if (ev.type == ClientMessage) {
				if (ev.xclient.data.l[0] == atomWmDeleteWindow) {
					break;
				}
			}
			else if (ev.type == DestroyNotify) { 
				break;
			}
		}

		currentTime = GetMilliseconds();
		deltaTime = double(currentTime - prevTime) * 0.001;
		prevTime = currentTime;

		if (!Update((float)deltaTime)) {
			break;
		}
		Render();

		// Present frame
		glXSwapBuffers(display, window);

		gettimeofday(&time, NULL);
		nextGameTick += SKIP_TICKS;
		sleepTime = nextGameTick - ((time.tv_sec * 1000) + (time.tv_usec / 1000));
		usleep((unsigned int)(sleepTime / 1000));
	}

	std::cout << "Shutting Down\n";
	Shutdown();

	// Cleanup GLX
	glXDestroyContext(display, context);

	// Cleanup X11
	XFree(visual);
	XFreeColormap(display, windowAttribs.colormap);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}

#ifdef TEST_LOCAL
bool Initialize(int w, int h) {
	glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
	glViewport(0, 0, w, h);
	return true;
}

bool Update(float deltaTime) {
	return true;
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
		glColor3f(  1.0f,  0.0f, 0.0f);
		glVertex3f( 0.0f, -1.0f, 0.0f);
		glColor3f(  0.0f,  1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, 0.0f);
		glColor3f(  0.0f,  0.0f, 1.0f);
		glVertex3f( 1.0f,  1.0f, 0.0f);
	glEnd();
}

void Resize(int w, int h) {
	glViewport(0, 0, w, h);
}

void Shutdown() {

}
#endif
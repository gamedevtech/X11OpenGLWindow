#Introduction
Most tutorials seem to start off with a discussion of how hard X is, how it will mess up your head and how only a masochist would ever write X code with alternates such as SDL out there. This, in my opinion is just wrong. In the trinity of major operating systems i think X is the sanest, most reliable and cleanest designed window manager. With that in mind, lets create an OpenGL enabled X11 window for video games.

#Part 1, Creating a Window
The X windowing system uses a client/server architecture. A single machine can have X running in multiple instances. 

##Creating an X Window
The first thing to do when opening a window under x is to tell it where the screen is. Altough there are several ways of telling the client where the server is, the most fullproof is the **DISPLAY** environment variable, or using *NULL* for default.

The function ```XOpenDisplay(char* display)``` makes the connection to the X server. It takes one argument, a string using the display format described above, or *NULL* to use the default. It returns a pointer to the display (of type *Display*) on success or *NULL* on error.

With the pointer the display, find the screen using the ```DefaultScreenOfDisplay(Display* display)``` function. This function takes one parameter, the pointer to the *Display* object that was found using **XOpenDisplay**. **DefaultScreenOfDisplay** returns a pointer of type *Screen* on success, *NULL* on error. 

To find the screen ID of the screen use the ```DefaultScreen(Display* display)``` function, this is almost identical to **DefaultScreenOfDisplay** except that it retuns an integer.

With just a display and a screen a window can be created with the **XCreateSimpleWindow** function. The prototype for this function is as follows
```
Window XCreateSimpleWindow(Display* display, Window* parent, int x, int y, uint borderw, ulong border, ulong background);
```
The first argument *display* is a pointer to the display object acquired using **XOpenDisplay**. The next argument, parent is a pointer to the window opening this one. If this is the first window of a program opening, use the ```RootWindowOfScreen(Screen* screen)``` function. This function takes one argument, the pointer to the screen that was acquired using **DefaultScreenOfDisplay**

The next four arguments are the location and sizeof the window. the *border-width* argument specifies the width of the window border in pixels. The next argument *border* is the color of the border. The last paramater *background* is simply the background color of the window.

There are several utility functions to acquire system colors (of type ulong). ```BlackPixel(Display* display, int screenId)``` and ```WhitePixel(Display* display, int screenId)``` are great examples. All of these functions take two arguments, a pointer to the display to use and the screenId being used.

The next step is to clear the window using ```XClearWindow(Display* display, Window* window)```, this function takes two arguments the display and the window to clear. Finally raise the window using the ```XMapRaised(Display* display, Window* window)``` function, which uses the same arguments as **XClearWindow**

At this point the window will not show up yet. First it must process messages (such as show window) that are sent to it. To process messages, start a "message loop", in it call ```XNextEvent(Display* display, XEvent* e)```. This function will return the current active event, or block until the next event is recieved. It take a *Display* object as it's first argument and writes to an *XEvent* pointer that is it's second argument.

After the window is finished, force the window to close with the ```XDestroyWindow(Display* display, Window window)``` function and cleanup the display using the ```XCloseDisplay(Display* display)``` function, which takes one argument, the display to close.

###Sample Code (C)
```
#include <X11/Xlib.h>

int main(int argc, char** argv) {
	Display* display;
	Window window;
	Screen* screen;
	int screenId;
	XEvent ev;

	// Open the display
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("%s\n", "Could not open display");
		return 1;
	}
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);

	// Open the window
	window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, 320, 200, 1, BlackPixel(display, screenId), WhitePixel(display, screenId));

    // Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);
	
	// Enter message loop
	while (true) {
		XNextEvent(display, &ev);
		// Do something
	}

	// Cleanup
	XDestroyWindow(display, window);
	XFree(screen);
	XCloseDisplay(display);
	return 1;
}
```
Compile the program using the following command line
```
LINUX: gcc -o XSampleWindow XSampleWindow.c -lX11 -L/usr/X11R6/lib -I/usr/X11R6/include

OSX: gcc -o XSampleWindow XSampleWindow.c -lX11 -L/usr/X11/lib -I/opt/X11/include
```

#Part 2, Processing Events
To register for events use the ```XSelectInput(Display* display, uint mask)``` function, it takes as arguments the display object, the window object and a bitmask representing what events the message loop will subscribe to. This function should be called before the *XMapWindow* function.

TODO: Event table listing
http://tronche.com/gui/x/xlib/events/processing-overview.html

##Keyboard Input
To process keyboard input subscribe to three event masks *KeyPressMask* *KeyReleaseMask* and *KeymapStateMask*. The first two events will fire when a key is pressed or released. X handles mapping characters to key numbers, this mapping can change at any time if the user remaps keys. the KeyMask event is fired when the keymapping of the system changes. 

Once subscribed to ```KeyPressMask | KeyReleaseMask | KeymapStateMask``` three events types will be fired, they are *KeyPress*, *KeyRelease* and *KeymapNotify*. Check the *type* variable of the *XEvent* object against these types.

In case the *KeyMapNotify* event is recieved, call the ```XRefreshKeyboardMapping(XMappingEvent* event_map);``` function. The function takes one argument, a *XMappingEvent* pointer.

If the *KeyPress* or *KeyRelease* events get fired use the ```int XLookupString(XKeyEvent *event_struct, char *buffer_return, int bytes_buffer, KeySym *keysym_return, XComposeStatus *status_in_out)``` function to translate the event into a **KeySym** (a *KeySym* is a typedef for an unsigned long coresponding to the keyID of the key pressed) and a string. *buffer_return* will hold the translated character string, *keysym_return* will hold the *KeySym* value. **keysymdef.h** holds a list of #define strings for each key.

###Sample Code (C++)
```
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <iostream>

int main(int argc, char** argv) {
	Display* display;
	Window window;
	Screen* screen;
	int screenId;
	XEvent ev;

	// Open the display
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("%s\n", "Could not open display");
		return 1;
	}
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);

	// Open the window
	window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, 320, 200, 1, BlackPixel(display, screenId), WhitePixel(display, screenId));

	XSelectInput(display, window, KeyPressMask | KeyReleaseMask | KeymapStateMask);

    // Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);
	
	// Variables used in message loop
	char str[25] = {0}; 
    KeySym keysym = 0;
    int len = 0;
    bool running = true;

	// Enter message loop
	while (running) {
		XNextEvent(display, &ev);
		switch(ev.type) {
            case KeymapNotify:
                XRefreshKeyboardMapping(&ev.xmapping);
            break;
            case KeyPress:
                len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
                if (len > 0) {
                    std::cout << "Key pressed: " << str << " - " << len << " - " << keysym <<'\n';
                }
                if (keysym == XK_Escape) {
                    running = false;
                }
            break;
            case KeyRelease:
                len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
                if (len > 0) {
                    std::cout << "Key released: " << str << " - " << len << " - " << keysym <<'\n';
                }
            break;
        }
	}

	// Cleanup
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 1;
}
```
Compile the program using the following command line
```
LINUX: g++ -o XKeyIn XKeyIn.cpp -lX11 -L/usr/X11R6/lib -I/usr/X11R6/include

OSX: g++ -o XKeyIn XKeyIn.cpp -lX11 -L/usr/X11/lib -I/opt/X11/include
```

##Mouse Input
To process mouse events subscribe to the *PointerMotionMask*, *ButtonPressMask*, *ButtonReleaseMask*, *EnterWindowMask* and *LeaveWindowMask* event masks. These masks will fire the following events: *MotionNotify*, *ButtonPress*, *ButtonRelease*, *EnterNotify* and *LeaveNotify*.

Masks:
```PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask```

On *ButtonPress* and *ButtonRelease* the event object will contains an *xbutton* field. This field has a button variable, which maps as follows: 1 - left mouse, 2 - middle mouse, 3 - right mouse, 4 - scroll up, 5 - scroll down.

On *MotionNotify* the event object will contain a *xmotion* field. This field has two variables *x* and *y*. The *x* and *y* variables contain the mouses position in window space.

###Sample Code (C++)
```
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <iostream>

int main(int argc, char** argv) {
	Display* display;
	Window window;
	Screen* screen;
	int screenId;
	XEvent ev;

	// Open the display
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("%s\n", "Could not open display");
		return 1;
	}
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);

	// Open the window
	window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, 320, 200, 1, BlackPixel(display, screenId), WhitePixel(display, screenId));

	XSelectInput(display, window, PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);
	
	// Variables used in message loop
	bool running = true;
	int x, y;

	// Enter message loop
	while (running) {
		XNextEvent(display, &ev);
		switch(ev.type) {
			case ButtonPress:
				if (ev.xbutton.button == 1) {
					std::cout << "Left mouse down\n";
				}
				else if (ev.xbutton.button == 2) {
					std::cout << "Middle mouse down\n";
				}
				else if (ev.xbutton.button == 3) {
					std::cout << "Right mouse down\n";
				}
				else if (ev.xbutton.button == 4) {
					std::cout << "Mouse scroll up\n";
				} 
				else if (ev.xbutton.button == 5) {
					std::cout << "Mouse scroll down\n";
				}
			break;
			case ButtonRelease:
				if (ev.xbutton.button == 1) {
					std::cout << "Left mouse up\n";
				}
				else if (ev.xbutton.button == 2) {
					std::cout << "Middle mouse up\n";
				}
				else if (ev.xbutton.button == 3) {
					std::cout << "Right mouse up\n";
					running = false;
				}
			break;
			case MotionNotify:
				x = ev.xmotion.x;
				y = ev.xmotion.y;
				std::cout << "Mouse X:" << x << ", Y: " << y << "\n";
			break;
			case EnterNotify:
				std::cout << "Mouse enter\n";
			break;
			case LeaveNotify:
				std::cout << "Mouse leave\n";
			break;
		}
	}

	// Cleanup
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 1;
}
```

##Window Size & Title
Set the window's title using the ```XStoreName(Display* d, Window w, char *window_name)``` function. The first argument is the display link, the second argument is the window object and the third argument is the new window title.

Query the size of the current window with the ```XGetWindowAttributes(Display* d, Window w, XWindowAttributes* winAttribs);``` function. The first argument is the display link, the second is the window object. The third argument is a *XWindowAttributes* object, this object has two fields *width* and *height*.

When a window is resized the *Expose* event is fired, listen for this event by subscribing the *ExposureMask* event mask. When processing the *Expose* event, query the window size with the *XGetWindowAttributes* funcion described above.

At runtime the width, height and position of the window can be changed with the ```XConfigureWindow(Display* d, Window w, uint value_mask, XWindowChanges *values;``` function. The first argument is the display link, the second argument is the window. The third argument is an unsigned int, this is a bitmask representing which fields of the fourth argument to use. The fourth argument is a pointer to a *XWindowChanges* structure. Valid values for the mask are: *CWX*, *CWY*, *CWWidth*, *CWHeight*, *CWBorderWidth*, *CWSibling* and *CWStackMode*. The *XWindowChanges* struct has the following fields: ```int x, y```, ```int width, height```, ```int border_width```, ```Window sibling``` and ```int stack_mode```.

###Sample Code (C++)
```
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <iostream>

int main(int argc, char** argv) {
	Display* display;
	Window window;
	Screen* screen;
	int screenId;
	XEvent ev;

	// Open the display
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("%s\n", "Could not open display");
		return 1;
	}
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);

	// Open the window
	window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, 320, 200, 1, BlackPixel(display, screenId), WhitePixel(display, screenId));

	XSelectInput(display, window, ExposureMask);

	// Name the window
	XStoreName(display, window, "Named Window");

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);
	
	// How large is the window
	XWindowAttributes attribs;
    XGetWindowAttributes(display, window, &attribs);
    std::cout << "Window Width: " << attribs.width << ", Height: " << attribs.height << "\n";

    // Resize window
    unsigned int change_values = CWWidth | CWHeight;
    XWindowChanges values;
    values.width = 800;
    values.height = 600;
    XConfigureWindow(display, window, change_values, &values);

	// Variables used in message loop
	bool running = true;
	int x, y;

	// Enter message loop
	while (running) {
		XNextEvent(display, &ev);
		switch(ev.type) {
			case Expose:
				std::cout << "Expose event fired\n";
				XGetWindowAttributes(display, window, &attribs);
    			std::cout << "\tWindow Width: " << attribs.width << ", Height: " << attribs.height << "\n";
			break;
		}
	}

	// Cleanup
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 1;
}
```

#Part 3, OpenGL
OpenGL on X11 is done trough the X11 library, include the ```<GL/glx.h>``` header.
##Enabling OpenGL
Before showing the window with *XMapRaised* it's a good idea to check the version of GLX available. Note, the **GLX version is not the same as the OpenGL version**. This can be done with the ```glXQueryVersion(Display* display, GLint* major, GLint* minor)``` function. The first argument is a pointer to the display object, the second two arguments are integer pointer that will have the major and minor version number written to them. If the minimum version of OpenGL is not supported, exit the program. A good minimum verions is 1.2
```
GLint majorGLX, minorGLX = 0;
glXQueryVersion(display, &majorGLX, &minorGLX);
if (majorGLX <= 1 && minorGLX < 2) {
    std::cout << "GLX 1.2 or greater is required.\n";
    XCloseDisplay(display);
    return 1;
}
else {
    std::cout << "GLX version: " << majorGLX << "." << minorGLX << '\n';
}
```
To create an OpenGL window a *XVisualInfo* object is needed. This object describes the minimum requirements to create a window. It is created with the ```glXChooseVisual(Display *display, int screen, int *attribList)``` function. If a window matching these minimum requirements can not be created *NULL* is returned. The first argument is the display object, the second is the screen id. The third argument is an array of flags used for creating the window. The creation code below makes a double buffered window with a 24 bit depth and 8bit stencil buffer.
```
GLint glxAttribs[] = {
	GLX_RGBA,
	GLX_DOUBLEBUFFER,
	GLX_DEPTH_SIZE,     24,
	GLX_STENCIL_SIZE,   8,
	GLX_RED_SIZE,       8,
	GLX_GREEN_SIZE,     8,
	GLX_BLUE_SIZE,      8,
	GLX_SAMPLE_BUFFERS, 0,
	GLX_SAMPLES,        0,
	None
};
XVisualInfo* visual = glXChooseVisual(display, screenId, glxAttribs);

if (visual == 0) {
	std::cout << "Could not create correct visual window.\n";
	XCloseDisplay(display);
	return 1;
}
```
When creating an OpenGL window the **XCreateSimpleWindow** function can no longer be used to create a window, instead the ```XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);``` function must be used. The first argument is the display, the second argument is the parent of the window being created. Use the ```RootWindow(Display* display, int screen_number``` function to get this window. The x, y, width, height and border width paramaters are self explanatory. For the depth of the window use the depth field of the *XVisualInfo* class, the visual mask should be set to the *InputOutput* constant. Lastly a **XSetWindowAttributes** struct must be created. This struct describes all of the attributes the window will have. The following fields should be set: *border_pixel*, *background_pixel*, *colormap* and *event_mask*. The *colormap* field is very important, get it using the ```XCreateColormap(Display *display, Window w, Visual *visual, int alloc);``` function. The display and window are the same as always. The *Visual* object is contained within the *visual* field of the *XVisualInfo* object. Lastly the *alloc* argument should be set to the **AllocNone** constant.
```
XSetWindowAttributes windowAttribs;
windowAttribs.border_pixel = BlackPixel(display, screenId);
windowAttribs.background_pixel = WhitePixel(display, screenId);
windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
windowAttribs.event_mask = ExposureMask;
window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, 320, 200, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);
```
After the window is created, an OpenGL context can be acquired with the ```glXCreateContext(Display* display, XVisualInfo* visual, GLXContext shareList, Bool direct);
``` function. The display and visual are the objects from before. The share list should be *NULL* and the direct should be set to true. Make the context active with the ```glXMakeCurrent(Display* display, Window window, GLXContext  context)``` function. After a context is active the ```glGetString``` function can be used to retrieve information about the active OpenGL instance.
```
GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
glXMakeCurrent(display, window, context);

std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
```
After a context is set active let the application enter it's update loop. Present the context of the active back buffer with the ```glXSwapBuffers(Display* display, Window window)``` function.
```
glClear(GL_COLOR_BUFFER_BIT);

glBegin(GL_TRIANGLES);
	glColor3f(  1.0f,  0.0f, 0.0f);
	glVertex3f( 0.0f, -1.0f, 0.0f);
	glColor3f(  0.0f,  1.0f, 0.0f);
	glVertex3f(-1.0f,  1.0f, 0.0f);
	glColor3f(  0.0f,  0.0f, 1.0f);
	glVertex3f( 1.0f,  1.0f, 0.0f);
glEnd();

// Present frame
glXSwapBuffers(display, window);
```
Finally, once the update loop has finishes the context must be destroyed with the ```glXDestroyContext(Display* display, GLXContext context)``` function. The *Visual* needs to be cleaned up with **XFree** and the display and color map association should be destroyed with **XFreeColormap**
###Sample Code (C++)
```
#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>

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
	else {
		std::cout << "GLX version: " << majorGLX << "." << minorGLX << '\n';
	}

	// GLX, create XVisualInfo, this is the minimum visuals we want
	GLint glxAttribs[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE,     24,
		GLX_STENCIL_SIZE,   8,
		GLX_RED_SIZE,       8,
		GLX_GREEN_SIZE,     8,
		GLX_BLUE_SIZE,      8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES,        0,
		None
	};
	XVisualInfo* visual = glXChooseVisual(display, screenId, glxAttribs);
	
	if (visual == 0) {
		std::cout << "Could not create correct visual window.\n";
		XCloseDisplay(display);
		return 1;
	}

	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, 320, 200, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Create GLX OpenGL context
	GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, context);

	std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
	std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

	// Set GL Sample stuff
	glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

	// Enter message loop
	while (true) {
		XNextEvent(display, &ev);
		if (ev.type == Expose) {
			XWindowAttributes attribs;
			XGetWindowAttributes(display, window, &attribs);
			glViewport(0, 0, attribs.width, attribs.height);
		}
		
		// OpenGL Rendering
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
			glColor3f(  1.0f,  0.0f, 0.0f);
			glVertex3f( 0.0f, -1.0f, 0.0f);
			glColor3f(  0.0f,  1.0f, 0.0f);
			glVertex3f(-1.0f,  1.0f, 0.0f);
			glColor3f(  0.0f,  0.0f, 1.0f);
			glVertex3f( 1.0f,  1.0f, 0.0f);
		glEnd();

		// Present frame
		glXSwapBuffers(display, window);
	}

	// Cleanup GLX
	glXDestroyContext(display, context);

	// Cleanup X11
	XFree(visual);
	XFreeColormap(display, windowAttribs.colormap);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
```

##Getting a modern context
Finding information on how to get a modern OpenGL context on X11 was rather difficult. Much of this part is adapted from https://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX). First, lets define a type for the ```glXCreateContextAttribsARB``` function and create a function that checks for supported extensions, this function will take two arguments, the extension list and target extension that is being queried.
```
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool isExtensionSupported(const char *extList, const char *extension) {
	const char *start;
	const char *where, *terminator;

	/* Extension names should not have spaces. */
	where = strchr(extension, ' ');
	if (where || *extension == '\0')
	return false;

	/* It takes a bit of care to be fool-proof about parsing the
	 OpenGL extensions string. Don't be fooled by sub-strings,
	 etc. */
	for (start=extList;;) {
	where = strstr(start, extension);

	if (!where) {
	 	break;
	}

	terminator = where + strlen(extension);

	if ( where == start || *(where - 1) == ' ' ) {
		if ( *terminator == ' ' || *terminator == '\0' ) {
			return true;
		}
	}	

	start = terminator;
	}

	return false;
}
```
The *glxAttribs* variable is going to be different for a modern context.
```
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
;
```
Once the desired attributes are defined, a compatible frame buffer needs to be created. This is done with the ```glXChooseFBConfig(Display* display, int screen, GLint* attribs, int* outCount``` funciton. This function will return an array of framebuffer configuration objects, as good as or better than what was specified in the *attribs* field. After the array is acquired it's ok to use the first one in the array.
```
int fbcount;
GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), glxAttribs, &fbcount);
if (fbc == 0) {
	std::cout << "Failed to retrieve framebuffer.\n";
	XCloseDisplay(display);
	return 1;
}
std::cout << "Found " << fbcount << " matching framebuffers.\n";
```
**glXChooseVisual** can no longer be used to acquire a *XVisualInfo* object, instead ```glXGetVisualFromFBConfig(Display* display, GLXFBConfig fbConfig)``` must be used to take the best framebuffer into account.
```
XVisualInfo* visual = glXGetVisualFromFBConfig( display, bestFbc );
if (visual == 0) {
	std::cout << "Could not create correct visual window.\n";
	XCloseDisplay(display);
	return 1;
}
```
A pointer to the **glXCreateContextAttribsARB** function needs to be acquired trough the ```glXGetProcAddressARB(const GLubyte * strFuncName)``` function.
```
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
```
To actually create the new context use the ```glXCreateNewContext``` function, this function is only available if the **GLX_ARB_create_context** extension is present. If the *glXCreateNewContext* function is not avilable fall back on the ```glXCreateContextAttribsARB``` function, if that is not available either the legacy ```glXCreateContext``` is always an option.
```
int context_attribs[] = {
	GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	GLX_CONTEXT_MINOR_VERSION_ARB, 2,
	GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
	None
};

const char *glxExts = glXQueryExtensionsString( display,  screenId );
GLXContext context = 0;
if (!isExtensionSupported( glxExts, "GLX_ARB_create_context")) {
	context = glXCreateNewContext( display, bestFbc, GLX_RGBA_TYPE, 0, True );
}
else {
	context = glXCreateContextAttribsARB( display, bestFbc, 0, true, context_attribs );
}
XSync( display, False );
```
It wouldn't make much sense to run a non direct OpenGL context for games, glx provides the ```glXIsDirect(Display* display, GLXContext context)``` function to check for this.
```
// Verifying that context is a direct context
if (!glXIsDirect (display, context)) {
	std::cout << "Indirect GLX rendering context obtained\n";
}
else {
	std::cout << "Direct GLX rendering context obtained\n";
}
```
###Sample Code (C++)
```
#include <iostream>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glx.h>

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool isExtensionSupported(const char *extList, const char *extension) {
	const char *start;
	const char *where, *terminator;

	where = strchr(extension, ' ');
	if (where || *extension == '\0') {
		return false;
	}

	for (start=extList;;) {
		where = strstr(start, extension);

		if (!where) {
		 	break;
		}

		terminator = where + strlen(extension);

		if ( where == start || *(where - 1) == ' ' ) {
			if ( *terminator == ' ' || *terminator == '\0' ) {
				return true;
			}
		}	

		start = terminator;
	}

	return false;
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
	else {
		std::cout << "GLX client version: " << glXGetClientString(display, GLX_VERSION) << '\n';
		std::cout << "GLX client vendor: " << glXGetClientString(display, GLX_VENDOR) << "\n";
		std::cout << "GLX client extensions:\n\t" << glXGetClientString(display, GLX_EXTENSIONS) << "\n";

		std::cout << "GLX server version: " << glXQueryServerString(display, screenId, GLX_VERSION) << "\n";
		std::cout << "GLX server vendoe: " << glXQueryServerString(display, screenId, GLX_VENDOR) << "\n";
		std::cout << "GLX server extensions:\n\t " << glXQueryServerString(display, screenId, GLX_EXTENSIONS) << "\n";
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
	std::cout << "Found " << fbcount << " matching framebuffers.\n";

	// Pick the FB config/visual with the most samples per pixel
	std::cout << "Getting best XVisualInfo\n";
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
		if ( vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );
			//std::cout << "  Matching fbconfig " << i << ", SAMPLE_BUFFERS = " << samp_buf << ", SAMPLES = " << samples << ".\n";

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
	std::cout << "Best visual info index: " << best_fbc << "\n";
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
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, 320, 200, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Create GLX OpenGL context
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

	const char *glxExts = glXQueryExtensionsString( display,  screenId );
	std::cout << "Late extensions:\n\t" << glxExts << "\n\n";
	if (glXCreateContextAttribsARB == 0) {
		std::cout << "glXCreateContextAttribsARB() not found.\n";
	}
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	GLXContext context = 0;
	if (!isExtensionSupported( glxExts, "GLX_ARB_create_context")) {
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

	std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
	std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

	// Set GL Sample stuff
	glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

	// Enter message loop
	while (true) {
		XNextEvent(display, &ev);
		if (ev.type == Expose) {
			XWindowAttributes attribs;
			XGetWindowAttributes(display, window, &attribs);
			glViewport(0, 0, attribs.width, attribs.height);
		}
		
		// OpenGL Rendering
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
			glColor3f(  1.0f,  0.0f, 0.0f);
			glVertex3f( 0.0f, -1.0f, 0.0f);
			glColor3f(  0.0f,  1.0f, 0.0f);
			glVertex3f(-1.0f,  1.0f, 0.0f);
			glColor3f(  0.0f,  0.0f, 1.0f);
			glVertex3f( 1.0f,  1.0f, 0.0f);
		glEnd();

		// Present frame
		glXSwapBuffers(display, window);
	}

	// Cleanup GLX
	glXDestroyContext(display, context);

	// Cleanup X11
	XFree(visual);
	XFreeColormap(display, windowAttribs.colormap);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
```
#Part 4, Putting it all together
Before putting all the pieces together, there is one more problem with the current implementation of the window. In order to close the window properly the **DestroyNotify** message needs to be handled. If *DestroyNotify* is recieved, the main loop should stop. To make the close button work, the **DeleteWindow** message must be treated as a client message; to remap it place the following code after the window was created, but before it is raised:
```
Atom atomWmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
XSetWMProtocols(display, window, &atomWmDeleteWindow, 1);
```
After the *DeleteWindow* message has been remapped as a client message subscribe to **StructureNotifyMask**. Finally, make sure to handle the new cases in the message loop.
```
XNextEvent(display, &ev);
if (ev.type == ClientMessage) {
	if (ev.xclient.data.l[0] == atomWmDeleteWindow) {
		break;
	}
}
else if (ev.type == DestroyNotify) { 
    break;
}
```
###Sample Code (C++)
```
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

		// Limit Framerate
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
```

#Sources
* http://xopendisplay.hilltopia.ca/2009/Jan/Xlib-tutorial-part-1----Beginnings.html
* http://tronche.com/gui/x/xlib/display/opening.html
* http://techpubs.sgi.com/library/dynaweb_docs/0640/SGI_Developer/books/OpenGL_Porting/sgi_html/ch04.html
* http://www.unix-manuals.com/tutorials/xlib/xlib.html
* http://nehe.gamedev.net/tutorial/simpleglxwindow_class/35003/
* http://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html
* https://github.com/blackberry/GamePlay/blob/master/gameplay%2Fsrc%2FPlatformLinux.cpp
* http://www-h.eng.cam.ac.uk/help/tpl/graphics/X/X11R5/node25.html
* http://tronche.com/gui/x/xlib/input/pointer-grabbing.html
* http://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XStoreName.html
* http://tronche.com/gui/x/xlib/window/configure.html#XWindowChanges
* http://tronche.com/gui/x/xlib/window/XConfigureWindow.html
* http://www.x.org/archive/X11R7.5/doc/man/man3/XVisualInfo.3.html
* http://msdn.microsoft.com/en-us/library/windows/desktop/dd318252(v=vs.85).aspx
* https://gist.github.com/kovrov/1304027
* https://www.youtube.com/watch?v=qWiqRIoKChg
* https://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
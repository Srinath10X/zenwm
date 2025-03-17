#include <core/events.hpp>
#include <core/input.hpp>
#include <core/types.hpp>

int main(int argc, char *argv[]) {
  /*
   *	Open a connection to the X server.
   *	XOpenDisplay(nullptr) connects to the default display (usually ":0").
   *	If it fails, it returns NULL, and the program exits with -1.
   */
  if (!(display = XOpenDisplay(nullptr))) return -1;

  /*
   *	Get the default screen number for the display.
   *	In X11, a display can have multiple screens, but usually, there's just one.
   */
  int screen = DefaultScreen(display);

  /*
   *	Get the root window of the default screen.
   *	The root window is the background window that covers the entire screen.
   */
  root = RootWindow(display, screen);

  /*	Get the width and height of the screen in pixels.	*/
  screen_width = XDisplayWidth(display, screen);
  screen_height = XDisplayHeight(display, screen);

  /*
   *	Select input events on the root window.
   *	'SubstructureRedirectMask' needed for a window manager to control window positioning and resizing.
   */
  XSelectInput(display, root, SubstructureRedirectMask);

  /*
   *	XCreateFontCursor(display, 68) creates a cursor with shape 68 (usually a left pointer).
   *	XDefineCursor(display, root, cursor) sets this cursor as the default for the root window.
   */
  XDefineCursor(display, root, XCreateFontCursor(display, 68));

  /*	Get the input from the root window and process it (mostly keyboard and mouse events).	*/
  getInput(root);

  /*
   *	Event loop to process events from the X server and call the appropriate event handlers.
   */
  while (running && !XNextEvent(display, &event))
    if (events[event.type]) events[event.type](&event);

  return 0;
}

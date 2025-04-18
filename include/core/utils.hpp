#pragma once

#include <X11/Xlib.h>
#include <functional>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MOD_CLEAN(mask) ((mask) & ~(numlock | LockMask) & (ShiftMask | Mod4Mask | Mod1Mask))

inline Window root;
inline XEvent event;
inline Display *display;
inline XButtonEvent mouse;

typedef struct Client {
  struct Client *next, *prev;

  int fs, x, y;
  Window window;
  unsigned width, height;
} Client;

using Command = const char *[];
using Event = unsigned int;
using EventHandler = void (*)(XEvent *);
using ClientFunction = std::function<void(Client *)>;

inline int xerror(Display *, XErrorEvent *) { return 0; }
inline void ws_save(int W, Client *list, Client *ws_list[]) { ws_list[W] = list; }
inline void ws_select(int W, Client *&list, int &ws, Client *ws_list[]) { list = ws_list[ws = W]; }

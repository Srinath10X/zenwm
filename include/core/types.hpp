#pragma once

#include <X11/Xlib.h>

#define SIZE_OF(arr) (sizeof(arr) / sizeof(*(arr)))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MOD_CLEAN(mask) ((mask) & ~(numlock | LockMask))

inline bool running = true;
inline unsigned numlock = 0;

inline Window root;
inline XEvent event;
inline Display *display;
inline unsigned screen_height, screen_width;

typedef struct Arg {
  Window window;
  unsigned index;
  const char **command;
} Arg;

typedef struct XClient {
  struct XClient *next, *prev;

  int fs, x, y;
  Window window;
  unsigned height, width;
} XClient;

typedef struct Key {
  Arg argument;
  KeySym keysym;
  unsigned modkey;

  void (*action)(const Arg *arg);
} Key;

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <unordered_map>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MOD_CLEAN(mask) ((mask) & ~(numlock | LockMask))

typedef struct {
  const char **com;
  const int i;
  const Window w;
} Arg;

struct key {
  unsigned int mod;
  KeySym keysym;
  void (*function)(const Arg arg);
  const Arg arg;
};

typedef struct client {
  struct client *next, *prev;
  int f, wx, wy;
  unsigned int ww, wh;
  Window w;
} client;

void ws_select(int W, client *&list, int &ws, client *ws_list[]) { list = ws_list[ws = W]; }

void ws_save(int W, client *list, client *ws_list[]) { ws_list[W] = list; }

void win_iterate(client *list, std::function<void(client *)> func) {
  if (!list) return;
  client *t = nullptr;
  client *c = list;
  while (c && t != list->prev) {
    func(c);
    t = c;
    c = c->next;
  }
}

void win_get_size(Display *d, Window W, int *gx, int *gy, unsigned int *gw, unsigned int *gh) {
  Window root_;
  unsigned int border_, depth_;
  XGetGeometry(d, W, &root_, gx, gy, gw, gh, &border_, &depth_);
}

void button_press(XEvent *e);
void button_release(XEvent *e);
void configure_request(XEvent *e);
void input_grab(Window root);
void key_press(XEvent *e);
void map_request(XEvent *e);
void mapping_notify(XEvent *e);
void notify_destroy(XEvent *e);
void notify_enter(XEvent *e);
void notify_motion(XEvent *e);
void run(const Arg arg);
void win_add(Window w);
void win_center(const Arg arg);
void win_del(Window w);
void win_fs(const Arg arg);
void win_focus(client *c);
void win_kill(const Arg arg);
void win_prev(const Arg arg);
void win_next(const Arg arg);
void win_to_ws(const Arg arg);
void ws_go(const Arg arg);
void quit(const Arg arg);

static int xerror(Display *, XErrorEvent *) { return 0; }

static client *list = nullptr, *ws_list[10] = {nullptr}, *cur;
static int ws = 1, sw, sh, wx, wy;
static unsigned int ww, wh, numlock = 0;

static Display *d;
static XButtonEvent mouse;
static Window root;
static int running = 1;

using EventHandler = void (*)(XEvent *);
extern std::unordered_map<int, EventHandler> events;

// clang-format off
std::unordered_map<int, EventHandler> events = {
    {ButtonPress, button_press},
    {ButtonRelease, button_release},
    {ConfigureRequest, configure_request},
    {KeyPress, key_press},
    {MapRequest, map_request},
    {MappingNotify, mapping_notify},
    {DestroyNotify, notify_destroy},
    {EnterNotify, notify_enter},
    {MotionNotify, notify_motion}
};
// clang-format on

#include "config.h"

void win_focus(client *c) {
  cur = c;
  XSetInputFocus(d, cur->w, RevertToParent, CurrentTime);
  XRaiseWindow(d, cur->w); // Ensure focused window is raised
}

void notify_destroy(XEvent *e) {
  win_del(e->xdestroywindow.window);
  if (list) win_focus(list->prev);
}

void notify_enter(XEvent *e) {
  while (XCheckTypedEvent(d, EnterNotify, e))
    ;
  win_iterate(list, [&](client *c) {
    if (c->w == e->xcrossing.window) win_focus(c);
  });
}

void notify_motion(XEvent *e) {
  if (!mouse.subwindow || cur->f) return;

  while (XCheckTypedEvent(d, MotionNotify, e))
    ;

  int xd = e->xmotion.x_root - mouse.x_root;
  int yd = e->xmotion.y_root - mouse.y_root;

  XMoveResizeWindow(d, mouse.subwindow, wx + (mouse.button == 1 ? xd : 0), wy + (mouse.button == 1 ? yd : 0),
                    MAX(1, ww + (mouse.button == 3 ? xd : 0)), MAX(1, wh + (mouse.button == 3 ? yd : 0)));
}

void key_press(XEvent *e) {
  KeySym keysym = XkbKeycodeToKeysym(d, e->xkey.keycode, 0, 0);
  for (unsigned int i = 0; i < sizeof(keys) / sizeof(*keys); ++i)
    if (keys[i].keysym == keysym && MOD_CLEAN(keys[i].mod) == MOD_CLEAN(e->xkey.state)) keys[i].function(keys[i].arg);
}

void button_press(XEvent *e) {
  if (!e->xbutton.subwindow) return;

  win_get_size(d, e->xbutton.subwindow, &wx, &wy, &ww, &wh);
  XRaiseWindow(d, e->xbutton.subwindow);
  mouse = e->xbutton;
}

void button_release(XEvent *e) { mouse.subwindow = 0; }

void win_add(Window w) {
  client *c = new (std::nothrow) client;
  if (!c) exit(1);

  // Initialize all members to avoid garbage values
  c->f = 0;
  c->wx = 0;
  c->wy = 0;
  c->ww = 0;
  c->wh = 0;
  c->w = w;

  if (list) {
    list->prev->next = c;
    c->prev = list->prev;
    list->prev = c;
    c->next = list;
  } else {
    list = c;
    list->prev = list->next = list;
  }
  ws_save(ws, list, ws_list);
}

void win_del(Window w) {
  client *x = nullptr;
  win_iterate(list, [&](client *c) {
    if (c->w == w) x = c;
  });

  if (!list || !x) return;
  if (x->prev == x) list = nullptr;
  if (list == x) list = x->next;
  if (x->next) x->next->prev = x->prev;
  if (x->prev) x->prev->next = x->next;

  delete x;
  ws_save(ws, list, ws_list);
}

void win_kill(const Arg arg) {
  if (cur) XKillClient(d, cur->w);
}

void win_center(const Arg arg) {
  if (!cur) return;

  int dummy_x, dummy_y;
  win_get_size(d, cur->w, &dummy_x, &dummy_y, &ww, &wh);
  XMoveWindow(d, cur->w, (sw - ww) / 2, (sh - wh) / 2);
}

void win_fs(const Arg arg) {
  if (!cur) return;

  if ((cur->f = !cur->f)) {
    win_get_size(d, cur->w, &cur->wx, &cur->wy, &cur->ww, &cur->wh);
    XMoveResizeWindow(d, cur->w, 0, 0, sw, sh);
  } else {
    XMoveResizeWindow(d, cur->w, cur->wx, cur->wy, cur->ww, cur->wh);
  }
}

void win_to_ws(const Arg arg) {
  int tmp = ws;
  if (arg.i == tmp) return;

  ws_select(arg.i, list, ws, ws_list);
  win_add(cur->w);
  ws_save(arg.i, list, ws_list);

  ws_select(tmp, list, ws, ws_list);
  win_del(cur->w);
  XUnmapWindow(d, cur->w);
  ws_save(tmp, list, ws_list);

  if (list) win_focus(list);
}

void win_prev(const Arg arg) {
  if (!cur) return;
  XRaiseWindow(d, cur->prev->w);
  win_focus(cur->prev);
}

void win_next(const Arg arg) {
  if (!cur) return;
  XRaiseWindow(d, cur->next->w);
  win_focus(cur->next);
}

void ws_go(const Arg arg) {
  int tmp = ws;
  if (arg.i == ws) return;

  ws_list[ws] = list;
  ws = arg.i;
  list = ws_list[ws];

  win_iterate(list, [&](client *c) {
    char *winame = nullptr;
    if (XFetchName(d, c->w, &winame) && winame != nullptr) {
      if (strncmp(winame, barname, strlen(barname)) != 0) {
        XMapWindow(d, c->w);
      }
      XFree(winame);
    } else {
      XMapWindow(d, c->w);
    }
  });

  ws_list[tmp] = ws_list[tmp];
  win_iterate(ws_list[tmp], [&](client *c) {
    char *winame = nullptr;
    if (XFetchName(d, c->w, &winame) && winame != nullptr) {
      if (strncmp(winame, barname, strlen(barname)) != 0) {
        XUnmapWindow(d, c->w);
      }
      XFree(winame);
    } else {
      XUnmapWindow(d, c->w);
    }
  });

  if (list)
    win_focus(list);
  else
    cur = nullptr;
}

void configure_request(XEvent *e) {
  XConfigureRequestEvent *ev = &e->xconfigurerequest;
  XWindowChanges changes = {.x = ev->x,
                            .y = ev->y,
                            .width = ev->width,
                            .height = ev->height,
                            .border_width = ev->border_width,
                            .sibling = ev->above,
                            .stack_mode = ev->detail};

  XConfigureWindow(d, ev->window, ev->value_mask, &changes);
}

void map_request(XEvent *e) {
  Window w = e->xmaprequest.window;
  XSelectInput(d, w, StructureNotifyMask | EnterWindowMask);

  int dummy_x, dummy_y;
  win_get_size(d, w, &dummy_x, &dummy_y, &ww, &wh);
  win_add(w);
  cur = list->prev;

  if (dummy_x + dummy_y == 0) win_center({});

  XMapWindow(d, w);
  XRaiseWindow(d, w);
  win_focus(list->prev);
}

void mapping_notify(XEvent *e) {
  XMappingEvent *ev = &e->xmapping;
  if (ev->request == MappingKeyboard || ev->request == MappingModifier) {
    XRefreshKeyboardMapping(ev);
    input_grab(root);
  }
}

void run(const Arg arg) {
  if (fork()) return;
  if (d) close(ConnectionNumber(d));

  setsid();
  execvp(const_cast<char *>(arg.com[0]), const_cast<char **>(arg.com));
}

void input_grab(Window root) {
  unsigned int i, j, modifiers[] = {0, LockMask};
  XModifierKeymap *modmap = XGetModifierMapping(d);
  KeyCode code;

  XUngrabKey(d, AnyKey, AnyModifier, root);

  for (i = 0; i < sizeof(keys) / sizeof(*keys); i++) {
    if ((code = XKeysymToKeycode(d, keys[i].keysym))) {
      for (j = 0; j < sizeof(modifiers) / sizeof(*modifiers); j++) {
        XGrabKey(d, code, keys[i].mod | modifiers[j], root, True, GrabModeAsync, GrabModeAsync);
      }
    }
  }

  for (i = 1; i < 4; i += 2) {
    for (j = 0; j < sizeof(modifiers) / sizeof(*modifiers); j++) {
      XGrabButton(d, i, MOD | modifiers[j], root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                  GrabModeAsync, GrabModeAsync, None, None);
    }
  }

  XFreeModifiermap(modmap);
}

void quit(const Arg arg) {
  if (!arg.i) running = 0;
}

int main() {
  XEvent ev;
  if (!(d = XOpenDisplay(nullptr))) exit(1);

  std::signal(SIGCHLD, SIG_IGN);
  XSetErrorHandler(xerror);

  int s = DefaultScreen(d);
  root = RootWindow(d, s);
  sw = XDisplayWidth(d, s);
  sh = XDisplayHeight(d, s);

  XSelectInput(d, root, SubstructureRedirectMask);
  XDefineCursor(d, root, XCreateFontCursor(d, 68));
  input_grab(root);

  while (running && !XNextEvent(d, &ev))
    if (events[ev.type]) events[ev.type](&ev);

  return 0;
}

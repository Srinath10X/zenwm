#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <core/events.hpp>
#include <core/types.hpp>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

void win_iterate(Client *list, ClientFunction func) {
  if (!list) return;

  Client *c = list;
  do {
    func(c);
    c = c->next;
  } while (c != list);
}

void win_get_size(Display *d, Window W, int *x, int *y, unsigned *w, unsigned *h) {
  XWindowAttributes attr;
  if (XGetWindowAttributes(d, W, &attr)) {
    *x = attr.x;
    *y = attr.y;
    *w = attr.width;
    *h = attr.height;
  }
}

static bool running = true;
static unsigned ww, wh, numlock = 0;
static int ws = 1, screen_width, screen_height, wx, wy;
static Client *list = nullptr, *ws_list[10] = {nullptr}, *cur;

// clang-format off
std::unordered_map<Event, EventHandler> events = {
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

#include <config.h>

void win_focus(Client *c) {
  cur = c;
  XSetInputFocus(display, cur->window, RevertToParent, CurrentTime);
}

void notify_destroy(XEvent *e) {
  win_del(e->xdestroywindow.window);
  if (list) win_focus(list->prev);
}

void notify_enter(XEvent *e) {
  while (XCheckTypedEvent(display, EnterNotify, e))
    ;
  win_iterate(list, [&](Client *c) {
    if (c->window == e->xcrossing.window) win_focus(c);
  });
}

void notify_motion(XEvent *e) {
  if (!mouse.subwindow || cur->fs) return;

  while (XCheckTypedEvent(display, MotionNotify, e))
    ;

  int xd = e->xmotion.x_root - mouse.x_root;
  int yd = e->xmotion.y_root - mouse.y_root;

  XMoveResizeWindow(display, mouse.subwindow, wx + (mouse.button == 1 ? xd : 0), wy + (mouse.button == 1 ? yd : 0),
                    MAX(1, ww + (mouse.button == 3 ? xd : 0)), MAX(1, wh + (mouse.button == 3 ? yd : 0)));
}

void key_press(XEvent *e) {
  KeySym keysym = XkbKeycodeToKeysym(display, e->xkey.keycode, 0, 0);
  for (unsigned i = 0; i < sizeof(keys) / sizeof(*keys); ++i)
    if (keys[i].keysym == keysym && MOD_CLEAN(keys[i].mod) == MOD_CLEAN(e->xkey.state)) keys[i].function(keys[i].arg);
}

void button_press(XEvent *e) {
  if (e->xbutton.subwindow) {
    mouse = e->xbutton;
    XRaiseWindow(display, e->xbutton.subwindow);
    win_get_size(display, e->xbutton.subwindow, &wx, &wy, &ww, &wh);
  }
}

void button_release(XEvent *e) { mouse.subwindow = 0; }

void win_add(Window w) {
  Client *c = new (std::nothrow) Client;
  if (!c) return;

  c->x = 0;
  c->y = 0;
  c->fs = 0;
  c->width = 0;
  c->height = 0;
  c->window = w;

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
  Client *x = nullptr;
  win_iterate(list, [&](Client *c) {
    if (c->window == w) x = c;
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
  if (cur) XKillClient(display, cur->window);
}

void win_center(const Arg arg) {
  if (!cur) return;

  int dummy_x, dummy_y;
  win_get_size(display, cur->window, &dummy_x, &dummy_y, &ww, &wh);
  XMoveWindow(display, cur->window, (screen_width - ww) / 2, (screen_height - wh) / 2);
}

void win_fs(const Arg arg) {
  if (!cur) return;

  if ((cur->fs = !cur->fs)) {
    win_get_size(display, cur->window, &cur->x, &cur->y, &cur->width, &cur->height);
    XMoveResizeWindow(display, cur->window, 0, 0, screen_width, screen_height);
  } else {
    XMoveResizeWindow(display, cur->window, cur->x, cur->y, cur->width, cur->height);
  }
}

void win_to_ws(const Arg arg) {
  int tmp = ws;
  if (arg.i == tmp) return;

  ws_select(arg.i, list, ws, ws_list);
  win_add(cur->window);
  ws_save(arg.i, list, ws_list);

  ws_select(tmp, list, ws, ws_list);
  win_del(cur->window);
  XUnmapWindow(display, cur->window);
  ws_save(tmp, list, ws_list);

  if (list) win_focus(list);
}

void win_prev(const Arg arg) {
  if (!cur) return;
  XRaiseWindow(display, cur->prev->window);
  win_focus(cur->prev);
}

void win_next(const Arg arg) {
  if (!cur) return;
  XRaiseWindow(display, cur->next->window);
  win_focus(cur->next);
}

void ws_go(const Arg arg) {
  int tmp = ws;
  if (arg.i == ws) return;

  ws_list[ws] = list;
  ws = arg.i;
  list = ws_list[ws];

  win_iterate(list, [&](Client *c) {
    char *winame = nullptr;
    if (XFetchName(display, c->window, &winame) && winame != nullptr) {
      if (strncmp(winame, barname, strlen(barname)) != 0) {
        XMapWindow(display, c->window);
      }
      XFree(winame);
    } else {
      XMapWindow(display, c->window);
    }
  });

  ws_list[tmp] = ws_list[tmp];
  win_iterate(ws_list[tmp], [&](Client *c) {
    char *winame = nullptr;
    if (XFetchName(display, c->window, &winame) && winame != nullptr) {
      if (strncmp(winame, barname, strlen(barname)) != 0) {
        XUnmapWindow(display, c->window);
      }
      XFree(winame);
    } else {
      XUnmapWindow(display, c->window);
    }
  });

  if (list)
    win_focus(list);
  else
    cur = nullptr;
}

void configure_request(XEvent *e) {
  XConfigureRequestEvent *ev = &e->xconfigurerequest;
  XConfigureWindow(display, ev->window, ev->value_mask, (XWindowChanges *)ev);
}

void map_request(XEvent *e) {
  Window w = e->xmaprequest.window;
  XSelectInput(display, w, StructureNotifyMask | EnterWindowMask);

  int dummy_x, dummy_y;
  win_get_size(display, w, &dummy_x, &dummy_y, &ww, &wh);
  win_add(w);
  cur = list->prev;

  if (dummy_x + dummy_y == 0) win_center({});

  XMapWindow(display, w);
  XRaiseWindow(display, w);
  win_focus(list->prev);
}

void mapping_notify(XEvent *e) {
  XMappingEvent *ev = &e->xmapping;
  if (ev->request == MappingKeyboard || ev->request == MappingModifier) {
    getInput(root);
    XRefreshKeyboardMapping(ev);
  }
}

void run(const Arg arg) {
  if (!fork()) {
    setsid();
    execvp(const_cast<char *>(arg.com[0]), const_cast<char **>(arg.com));
  }
}

void grabKeyboard(Window root) {
  XUngrabKey(display, AnyKey, AnyModifier, root);

  for (unsigned i = 0; i < sizeof(keys) / sizeof(*keys); i++) {
    KeyCode code = XKeysymToKeycode(display, keys[i].keysym);
    if (code) XGrabKey(display, code, MOD_CLEAN(keys[i].mod), root, True, GrabModeAsync, GrabModeAsync);
  }
}

void grabMouse(Window root) {
  for (unsigned i = 1; i < 4; i += 2) {
    /* clang-format off */
		XGrabButton(
			display, i, MOD_CLEAN(MOD), root, True,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			GrabModeAsync, GrabModeAsync, None, None
		);
    /* clang-format on */
  }
}

void getInput(Window root) {
  XModifierKeymap *modmap = XGetModifierMapping(display);

  grabMouse(root);
  grabKeyboard(root);

  XFreeModifiermap(modmap);
}

void quit(const Arg arg) { running = false; }

int main() {
  if (!(display = XOpenDisplay(nullptr))) exit(1);

  std::signal(SIGCHLD, SIG_IGN);
  XSetErrorHandler(xerror);

  int screen = DefaultScreen(display);
  root = RootWindow(display, screen);
  screen_width = XDisplayWidth(display, screen);
  screen_height = XDisplayHeight(display, screen);

  XSelectInput(display, root, SubstructureRedirectMask);
  XDefineCursor(display, root, XCreateFontCursor(display, 68));
  getInput(root);

  while (running && !XNextEvent(display, &event))
    if (events[event.type]) events[event.type](&event);

  return 0;
}

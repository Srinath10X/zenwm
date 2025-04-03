#pragma once

#include <core/utils.hpp>

typedef struct {
  const char **com;
  const int i;
  const Window w;
} Arg;

typedef struct {
  unsigned int mod;
  KeySym keysym;
  void (*function)(const Arg arg);
  const Arg arg;
} Key;

void getInput(Window root);
void run(const Arg arg);
void win_add(Window w);
void win_center(const Arg arg);
void win_del(Window w);
void win_fs(const Arg arg);
void win_focus(Client *c);
void win_kill(const Arg arg);
void win_prev(const Arg arg);
void win_next(const Arg arg);
void win_to_ws(const Arg arg);
void ws_go(const Arg arg);
void quit(const Arg arg);

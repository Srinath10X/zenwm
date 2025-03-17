#pragma once

#include <X11/XKBlib.h>
#include <core/types.hpp>
#include <unordered_map>

inline XButtonEvent mouse;

void key_press(XEvent *e);
void button_press(XEvent *e);
void button_release(XEvent *e);

void notify_enter(XEvent *e);
void notify_motion(XEvent *e);
void notify_destroy(XEvent *e);

void map_request(XEvent *e);
void mapping_notify(XEvent *e);
void configure_request(XEvent *e);

using EventHandler = void (*)(XEvent *);
extern std::unordered_map<int, EventHandler> events;

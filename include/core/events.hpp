#pragma once

#include <X11/Xlib.h>

void key_press(XEvent *e);

void button_press(XEvent *e);
void button_release(XEvent *e);

void notify_enter(XEvent *e);
void notify_motion(XEvent *e);
void notify_destroy(XEvent *e);

void map_request(XEvent *e);
void mapping_notify(XEvent *e);
void configure_request(XEvent *e);

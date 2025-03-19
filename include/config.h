#pragma once

#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <core/types.hpp>

#define MOD Mod4Mask

static const char *barname = "polybar";

static Command term = {"kitty", 0};
static Command menu = {"rofi", "-show", "drun", 0};
static Command briup = {"brightnessctl", "set", "1%+", 0};
static Command bridown = {"brightnessctl", "set", "1%-", 0};
static Command volmute = {"pamixer", "-t", 0};
static Command volup = {"pamixer", "-i", "5", "--allow-boost", 0};
static Command voldown = {"pamixer", "-d", "5", "--allow-boost", 0};

static Key keys[] = {
    {MOD, XK_q, win_kill, {0}},
    {MOD, XK_c, win_center, {0}},
    {MOD, XK_f, win_fs, {0}},

    {Mod1Mask, XK_Tab, win_next, {0}},
    {Mod1Mask | ShiftMask, XK_Tab, win_prev, {0}},

    {MOD, XK_o, run, {.com = menu}},
    {MOD, XK_Return, run, {.com = term}},
    {MOD, XK_m, quit, {0}},

    {MOD, XK_1, ws_go, {.i = 1}},
    {MOD | ShiftMask, XK_1, win_to_ws, {.i = 1}},
    {MOD, XK_2, ws_go, {.i = 2}},
    {MOD | ShiftMask, XK_2, win_to_ws, {.i = 2}},
    {MOD, XK_3, ws_go, {.i = 3}},
    {MOD | ShiftMask, XK_3, win_to_ws, {.i = 3}},
    {MOD, XK_4, ws_go, {.i = 4}},
    {MOD | ShiftMask, XK_4, win_to_ws, {.i = 4}},
    {MOD, XK_5, ws_go, {.i = 5}},
    {MOD | ShiftMask, XK_5, win_to_ws, {.i = 5}},
    {MOD, XK_6, ws_go, {.i = 6}},
    {MOD | ShiftMask, XK_6, win_to_ws, {.i = 6}},

    {0, XF86XK_AudioLowerVolume, run, {.com = voldown}},
    {0, XF86XK_AudioRaiseVolume, run, {.com = volup}},
    {0, XF86XK_AudioMute, run, {.com = volmute}},
    {0, XF86XK_MonBrightnessUp, run, {.com = briup}},
    {0, XF86XK_MonBrightnessDown, run, {.com = bridown}},
};

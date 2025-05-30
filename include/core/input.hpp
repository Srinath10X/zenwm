#pragma once

#include <X11/keysym.h>
#include <core/types.hpp>

void detectNumlock();
void getInput(Window root);
void grabMouse(Window root);
void grabKeyboard(Window root);

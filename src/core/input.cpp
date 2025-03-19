#include <config.h>
#include <core/input.hpp>
#include <core/types.hpp>

void grabKeyboard(Window root) {
  XUngrabKey(display, AnyKey, AnyModifier, root);

  for (unsigned i = 0; i < ARRAY_SIZE(keys); i++) {
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

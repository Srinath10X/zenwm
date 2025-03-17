#include <config.h>
#include <core/events.hpp>
#include <core/types.hpp>

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

void key_press(XEvent *event) {
  KeySym keysym = XkbKeycodeToKeysym(display, event->xkey.keycode, 0, 0);

  for (size_t i = 0; i < SIZE_OF(keys); ++i) {
    bool is_key_match = (keys[i].keysym == keysym);
    bool is_modifier_match = (MOD_CLEAN(keys[i].modkey) == MOD_CLEAN(event->xkey.state));

    if (is_key_match && is_modifier_match) keys[i].action(&keys[i].argument);
  }
}

void button_press(XEvent *e) {}

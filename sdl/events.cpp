#include "std_oak_header.hpp"
#include <SDL2/SDL.h>

// Struct definitions
struct sdl_key_event_data
{
    i128 keycode;
};

struct sdl_mouse_button_event_data
{
    i128 x;
    i128 y;
    i8 code;
};

struct sdl_mouse_move_event_data
{
    i128 x;
    i128 y;
};

struct unit
{
};

// Enumeration definitions
struct sdl_event
{
    enum
    {
        sdl_key_down_event,
        sdl_key_up_event,
        sdl_mouse_move_event,
        sdl_mouse_button_down_event,
        sdl_mouse_button_up_event,
        sdl_none,

    } __info;
    union
    {
        sdl_key_event_data sdl_key_down_event_data;
        sdl_key_event_data sdl_key_up_event_data;
        sdl_mouse_move_event_data sdl_mouse_move_event_data;
        sdl_mouse_button_event_data sdl_mouse_button_down_event_data;
        sdl_mouse_button_event_data sdl_mouse_button_up_event_data;
        unit sdl_none_data;

    } __data;
};
void wrap_sdl_key_down_event(sdl_event *self, sdl_key_event_data data);
void wrap_sdl_key_up_event(sdl_event *self, sdl_key_event_data data);
void wrap_sdl_mouse_move_event(sdl_event *self, sdl_mouse_move_event_data data);
void wrap_sdl_mouse_button_down_event(sdl_event *self, sdl_mouse_button_event_data data);
void wrap_sdl_mouse_button_up_event(sdl_event *self, sdl_mouse_button_event_data data);
void wrap_sdl_none(sdl_event *self);

sdl_event sdl_poll_event()
{
    sdl_event out;

    SDL_Event event;
    if (!SDL_PollEvent(&event))
    {
        // Nothing in queue; Return sdl_none
        wrap_sdl_none(&out);
    }
    else if (event.type == SDL_KEYDOWN)
    {
        sdl_key_event_data to_wrap;
        to_wrap.keycode = event.key.keysym.sym;
        wrap_sdl_key_down_event(&out, to_wrap);
    }
    else if (event.type == SDL_KEYUP)
    {
        sdl_key_event_data to_wrap;
        to_wrap.keycode = event.key.keysym.sym;
        wrap_sdl_key_down_event(&out, to_wrap);
    }
    else if (event.type == SDL_MOUSEMOTION)
    {
        sdl_mouse_move_event_data to_wrap;

        to_wrap.x = event.motion.x;
        to_wrap.y = event.motion.y;

        wrap_sdl_mouse_move_event(&out, to_wrap);
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        sdl_mouse_button_event_data to_wrap;

        to_wrap.x = event.button.x;
        to_wrap.y = event.button.y;
        to_wrap.code = event.button.button;

        wrap_sdl_mouse_button_down_event(&out, to_wrap);
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        sdl_mouse_button_event_data to_wrap;

        to_wrap.x = event.button.x;
        to_wrap.y = event.button.y;
        to_wrap.code = event.button.button;

        wrap_sdl_mouse_button_down_event(&out, to_wrap);
    }

    return out;
}

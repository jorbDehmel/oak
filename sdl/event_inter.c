#include "oak/std_oak_header.h"
#include "keys.h"
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
    char __a;
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
        struct sdl_key_event_data sdl_key_down_event_data;
        struct sdl_key_event_data sdl_key_up_event_data;
        struct sdl_mouse_move_event_data sdl_mouse_move_event_data;
        struct sdl_mouse_button_event_data sdl_mouse_button_down_event_data;
        struct sdl_mouse_button_event_data sdl_mouse_button_up_event_data;
        struct unit sdl_none_data;

    } __data;
};

void wrap_sdl_key_down_event_FN_PTR_sdl_event_JOIN_sdl_key_event_data_MAPS_void(struct sdl_event *self, struct sdl_key_event_data data);
void wrap_sdl_key_up_event_FN_PTR_sdl_event_JOIN_sdl_key_event_data_MAPS_void(struct sdl_event *self, struct sdl_key_event_data data);
void wrap_sdl_mouse_move_event_FN_PTR_sdl_event_JOIN_sdl_mouse_move_event_data_MAPS_void(struct sdl_event *self, struct sdl_mouse_move_event_data data);
void wrap_sdl_mouse_button_down_event_FN_PTR_sdl_event_JOIN_sdl_mouse_button_event_data_MAPS_void(struct sdl_event *self, struct sdl_mouse_button_event_data data);
void wrap_sdl_mouse_button_up_event_FN_PTR_sdl_event_JOIN_sdl_mouse_button_event_data_MAPS_void(struct sdl_event *self, struct sdl_mouse_button_event_data data);
void wrap_sdl_none_FN_PTR_sdl_event_MAPS_void(struct sdl_event *self);

struct sdl_eventsdl_poll_event_FN_MAPS_sdl_event()
{
    struct sdl_eventout;

    SDL_Event event;
    if (!SDL_PollEvent(&event))
    {
        // Nothing in queue; Return sdl_none
        wrap_sdl_none_FN_PTR_sdl_event_MAPS_void(&out);
    }
    else if (event.type == SDL_KEYDOWN)
    {
        sdl_key_event_data to_wrap;
        to_wrap.keycode = event.key.keysym.sym;
        wrap_sdl_key_down_event_FN_PTR_sdl_event_JOIN_sdl_key_event_data_MAPS_void(&out, to_wrap);
    }
    else if (event.type == SDL_KEYUP)
    {
        sdl_key_event_data to_wrap;
        to_wrap.keycode = event.key.keysym.sym;
        wrap_sdl_key_up_event_FN_PTR_sdl_event_JOIN_sdl_key_event_data_MAPS_void(&out, to_wrap);
    }
    else if (event.type == SDL_MOUSEMOTION)
    {
        sdl_mouse_move_event_data to_wrap;

        to_wrap.x = event.motion.x;
        to_wrap.y = event.motion.y;

        wrap_sdl_mouse_move_event_FN_PTR_sdl_event_JOIN_sdl_mouse_move_event_data_MAPS_void(&out, to_wrap);
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        sdl_mouse_button_event_data to_wrap;

        to_wrap.x = event.button.x;
        to_wrap.y = event.button.y;
        to_wrap.code = event.button.button;

        wrap_sdl_mouse_button_down_event_FN_PTR_sdl_event_JOIN_sdl_mouse_button_event_data_MAPS_void(&out, to_wrap);
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        sdl_mouse_button_event_data to_wrap;

        to_wrap.x = event.button.x;
        to_wrap.y = event.button.y;
        to_wrap.code = event.button.button;

        wrap_sdl_mouse_button_down_event_FN_PTR_sdl_event_JOIN_sdl_mouse_button_event_data_MAPS_void(&out, to_wrap);
    }

    return out;
}

str sdl_keycode_to_str_FN_i128_MAPS_str(i128 keycode)
{
    // Letters
    if (a <= keycode && keycode <= z || zero <= keycode && keycode <= nine)
    {
        // Static for warning suppression
        static const char out[2] = {(char)(keycode), '\0'};
        return out;
    }

    // f1 - f12
    else if (f1 <= keycode && keycode <= f12)
    {
        if (keycode == f1)
        {
            return "f1";
        }
        else if (keycode == f2)
        {
            return "f2";
        }
        else if (keycode == f3)
        {
            return "f3";
        }
        else if (keycode == f4)
        {
            return "f4";
        }
        else if (keycode == f5)
        {
            return "f5";
        }
        else if (keycode == f6)
        {
            return "f6";
        }
        else if (keycode == f7)
        {
            return "f7";
        }
        else if (keycode == f8)
        {
            return "f8";
        }
        else if (keycode == f9)
        {
            return "f9";
        }
        else if (keycode == f10)
        {
            return "f10";
        }
        else if (keycode == f11)
        {
            return "f11";
        }
        else if (keycode == f12)
        {
            return "f12";
        }
    }

    // Special characters
    else
    {
        switch (keycode)
        {
        case space:
            return "space";

        case esc:
            return "escape";

        case enter:
            return "enter";

        case rightArrow:
            return "right_arrow";

        case leftArrow:
            return "left_arrow";

        case downArrow:
            return "down_arrow";

        case upArrow:
            return "up_arrow";

        case tab:
            return "tab";

        case capslock:
            return "capslock";

        case leftShift:
            return "left_shift";

        case leftControl:
            return "left_control";

        case leftAlt:
            return "left_alt";

        case rightShift:
            return "right_shift";

        case backspace:
            return "backspace";

        case del:
            return "delete";

        case backtick:
            return "`";

        case dash:
            return "-";

        case equals:
            return "=";

        case leftSquareBracket:
            return "[";

        case rightSquareBracket:
            return "]";

        case backslash:
            return "\\";

        case semicolon:
            return ";";

        case quote:
            return "'";

        case comma:
            return ",";

        case period:
            return ".";

        case slash:
            return "/";

        default:
            break;
        }
    }

    return "UNKOWN";
}

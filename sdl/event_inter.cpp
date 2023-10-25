#include "oak/std_oak_header.hpp"
#include "keys.hpp"

// FOR DEBUGGING ONLY
// #include <stdio.h>

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
        wrap_sdl_key_up_event(&out, to_wrap);
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

str sdl_keycode_to_str(i128 keycode)
{
    // Letters
    if (keys::a <= keycode && keycode <= keys::z || keys::zero <= keycode && keycode <= keys::nine)
    {
        // Static for warning suppression
        static const char out[2] = {(char)(keycode), '\0'};
        return out;
    }

    // f1 - f12
    else if (keys::f1 <= keycode && keycode <= keys::f12)
    {
        if (keycode == keys::f1)
        {
            return "f1";
        }
        else if (keycode == keys::f2)
        {
            return "f2";
        }
        else if (keycode == keys::f3)
        {
            return "f3";
        }
        else if (keycode == keys::f4)
        {
            return "f4";
        }
        else if (keycode == keys::f5)
        {
            return "f5";
        }
        else if (keycode == keys::f6)
        {
            return "f6";
        }
        else if (keycode == keys::f7)
        {
            return "f7";
        }
        else if (keycode == keys::f8)
        {
            return "f8";
        }
        else if (keycode == keys::f9)
        {
            return "f9";
        }
        else if (keycode == keys::f10)
        {
            return "f10";
        }
        else if (keycode == keys::f11)
        {
            return "f11";
        }
        else if (keycode == keys::f12)
        {
            return "f12";
        }
    }

    // Special characters
    else
    {
        switch (keycode)
        {
        case keys::space:
            return "space";

        case keys::esc:
            return "escape";

        case keys::enter:
            return "enter";

        case keys::rightArrow:
            return "right_arrow";

        case keys::leftArrow:
            return "left_arrow";

        case keys::downArrow:
            return "down_arrow";

        case keys::upArrow:
            return "up_arrow";

        case keys::tab:
            return "tab";

        case keys::capslock:
            return "capslock";

        case keys::leftShift:
            return "left_shift";

        case keys::leftControl:
            return "left_control";

        case keys::leftAlt:
            return "left_alt";

        case keys::rightShift:
            return "right_shift";

        case keys::backspace:
            return "backspace";

        case keys::del:
            return "delete";

        case keys::backtick:
            return "`";

        case keys::dash:
            return "-";

        case keys::equals:
            return "=";

        case keys::leftSquareBracket:
            return "[";

        case keys::rightSquareBracket:
            return "]";

        case keys::backslash:
            return "\\";

        case keys::semicolon:
            return ";";

        case keys::quote:
            return "'";

        case keys::comma:
            return ",";

        case keys::period:
            return ".";

        case keys::slash:
            return "/";

        default:
            break;
        }
    }

    return "UNKOWN";
}

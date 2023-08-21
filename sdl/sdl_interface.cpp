// Signed integer types
typedef char i8;
typedef short int i16;
typedef int i32;
typedef long int i64;
typedef long long int i128;

// Unsigned integer types
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;
typedef unsigned long long int u128;

// Floating point types
typedef float f32;
typedef double f64;
typedef long double f128;

// Misc
typedef const char *const str;

#include <SDL2/SDL.h>

struct sdl_window
{
    u64 width, height;

    SDL_Window *wind;
    SDL_Renderer *rend;
};

struct sdl_color
{
    u8 r, g, b, a;
};

struct sdl_coord
{
    u64 x, y;
};

struct sdl_rect
{
    u64 x, y, w, h;
};

void sdl_init()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    return;
}

void sdl_quit()
{
    SDL_Quit();
    return;
}

void sdl_delay(i32 ms)
{
    SDL_Delay(ms);
    return;
}

f64 sdl_sin(f64 x)
{
    return SDL_sin(x);
}

f64 sdl_cos(f64 x)
{
    return SDL_cos(x);
}

f64 sdl_tan(f64 x)
{
    return SDL_tan(x);
}

void New(sdl_window *self)
{
    self->width = self->height = 128;
    SDL_CreateWindowAndRenderer(128, 128, 0, &self->wind, &self->rend);

    return;
}

void Copy(sdl_window *self, u64 w, u64 h)
{
    self->width = w;
    self->height = h;
    SDL_CreateWindowAndRenderer(w, h, 0, &self->wind, &self->rend);

    return;
}

void Del(sdl_window *window)
{
    SDL_DestroyWindow(window->wind);
    SDL_DestroyRenderer(window->rend);
    return;
}

void sdl_enable_window_fullscreen(sdl_window *self)
{
    SDL_SetWindowFullscreen(self->wind, SDL_WINDOW_FULLSCREEN_DESKTOP);
    return;
}

void sdl_disable_window_fullscreen(sdl_window *self)
{
    SDL_SetWindowFullscreen(self->wind, 0);
    return;
}

void show(sdl_window *window)
{
    SDL_RenderPresent(window->rend);
    return;
}

void fill(sdl_window *window, sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);
    SDL_RenderClear(window->rend);
    return;
}

void draw_dot(sdl_window *window, sdl_coord point, sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(window->rend, point.x, point.y);
    return;
}

void draw_line(sdl_window *window, sdl_coord point_a, sdl_coord point_b, sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(window->rend, point_a.x, point_a.y, point_b.x, point_b.y);
    return;
}

void draw_rect(sdl_window *window, sdl_rect rect, sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);

    SDL_Rect tempRect;
    tempRect.x = rect.x;
    tempRect.y = rect.y;
    tempRect.w = rect.w;
    tempRect.h = rect.h;

    SDL_RenderFillRect(window->rend, &tempRect);
    return;
}

///////////////////////////////////// Event stuff /////////////////////////////////////

struct sdl_event
{
    u32 type, timestamp;
};

////////////////////////////////////////////////////////////////

/*
Main interfacial file for the SDL Oak library.

Jordan Dehmel, 2023-present
jdehmel@outlook.com, GPLv3
*/

////////////////////////////////////////////////////////////////

#include "/usr/include/oak/std_oak_header.h"
#include <SDL2/SDL.h>

////////////////////////////////////////////////////////////////
// Struct definitions

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

////////////////////////////////////////////////////////////////
// Interfacial functions defined in C

void sdl_init_FN_MAPS_void(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    return;
}

void sdl_quit_FN_MAPS_void(void)
{
    SDL_Quit();
    return;
}

void sdl_delay_FN_i32_MAPS_void(i32 ms)
{
    SDL_Delay(ms);
    return;
}

i32 sdl_get_ticks_FN_MAPS_i32()
{
    return SDL_GetTicks();
}

f64 sdl_sin_FN_f64_MAPS_f64(f64 x)
{
    return SDL_sin(x);
}

f64 sdl_cos_FN_f64_MAPS_f64(f64 x)
{
    return SDL_cos(x);
}

f64 sdl_tan_FN_f64_MAPS_f64(f64 x)
{
    return SDL_tan(x);
}

////////////////////////////////////////////////////////////////
// Interfacial methods

void ExtInit_FN_PTR_sdl_window_MAPS_void(struct sdl_window *self)
{
    self->width = self->height = 128;
    SDL_CreateWindowAndRenderer(128, 128, 0, &self->wind, &self->rend);

    return;
}

struct sdl_window Copy_FN_PTR_sdl_window_JOIN_u64_JOIN_u64_MAPS_sdl_window(struct sdl_window *self, u64 w, u64 h)
{
    self->width = w;
    self->height = h;

    SDL_SetWindowSize(self->wind, w, h);

    return *self;
}

void ExtDel_FN_PTR_sdl_window_MAPS_void(struct sdl_window *window)
{
    SDL_DestroyWindow(window->wind);
    SDL_DestroyRenderer(window->rend);
    return;
}

void sdl_enable_window_fullscreen_FN_PTR_sdl_window_MAPS_void(struct sdl_window *self)
{
    SDL_SetWindowFullscreen(self->wind, SDL_WINDOW_FULLSCREEN_DESKTOP);
    return;
}

void sdl_disable_window_fullscreen_FN_PTR_sdl_window_MAPS_void(struct sdl_window *self)
{
    SDL_SetWindowFullscreen(self->wind, 0);
    return;
}

void show_FN_PTR_sdl_window_MAPS_void(struct sdl_window *window)
{
    SDL_RenderPresent(window->rend);
    return;
}

void fill_FN_PTR_sdl_window_JOIN_sdl_color_MAPS_void(struct sdl_window *window, struct sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);
    SDL_RenderClear(window->rend);
    return;
}

void draw_dot_FN_PTR_sdl_window_JOIN_sdl_coord_JOIN_sdl_color_MAPS_void(struct sdl_window *window,
                                                                        struct sdl_coord point, struct sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(window->rend, point.x, point.y);
    return;
}

void draw_line_FN_PTR_sdl_window_JOIN_sdl_coord_JOIN_sdl_coord_JOIN_sdl_color_MAPS_void(struct sdl_window *window,
                                                                                        struct sdl_coord point_a,
                                                                                        struct sdl_coord point_b,
                                                                                        struct sdl_color color)
{
    SDL_SetRenderDrawColor(window->rend, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(window->rend, point_a.x, point_a.y, point_b.x, point_b.y);
    return;
}

void draw_rect_FN_PTR_sdl_window_JOIN_sdl_rect_JOIN_sdl_color_MAPS_void(struct sdl_window *window, struct sdl_rect rect,
                                                                        struct sdl_color color)
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

void sdl_save_FN_PTR_sdl_window_JOIN_ARR_i8_MAPS_void(struct sdl_window *self, i8 filepath[])
{
    // Get pixels
    SDL_Surface *surf = SDL_CreateRGBSurface(0, self->width, self->height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(self->rend, NULL, 0, surf->pixels, surf->pitch);

    // Save as bmp file
    SDL_SaveBMP(surf, filepath);

    // Free used memory
    SDL_FreeSurface(surf);
}

////////////////////////////////////////////////////////////////

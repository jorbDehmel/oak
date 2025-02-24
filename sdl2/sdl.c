#include <SDL2/SDL.h>

struct sdl_Point {
  float x, y;
};

struct sdl_Rect {
  float h, w, x, y;
};

static_assert(sizeof(sdl_Point) == 8);
static_assert(sizeof(sdl_Rect) == 16);

void sdl_init_FN_MAPS_void() {
  SDL_Init(SDL_INIT_EVERYTHING);
}

void sdl_quit_FN_MAPS_void() {
  SDL_Quit();
}

#include <SDL2/SDL.h>
#include <stdio.h>

int main()
{
    bool running = true;
    SDL_Event event;

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *wind = SDL_CreateWindow("hi", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, 0);

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                printf("%d\n", (int)(event.key.keysym.sym));
            }
            else if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        SDL_Delay(100);
    }

    SDL_DestroyWindow(wind);
    SDL_Quit();

    return 0;
}

#include "util.h"

int main(int argc, char const *argv[])
{
    const int FPS = 60;
    const int TICKS = 1000 / FPS;

    const int W_WIDTH = 640;
    const int W_HEIGHT = 480;

    SDL_Window *window;
    SDL_Renderer *renderer;
    Texture texture;

    SDL_Event e;
    SDL_Rect c_clips[12];

    player player = {
        .w = 16,
        .h = 16,
        .x = 320 - 16,
        .y = 240 - 16,
        .dir = -1,
        .face = DOWN,
        .xvel = 0,
        .yvel = 0,
        .input = {.last = -1, .current = -1},
        .moving = false,
        .acounter = 0,
        .aindex = 1
    };

    game GAME = {
        .window = &window,
        .renderer = &renderer,
        .texture = &texture,
        .p = &player,
        .running = true
    };

    int timer, delta;

    if (initSdl(&window, &renderer, W_WIDTH, W_HEIGHT))
    {
        if (initTextureMap(&renderer, &texture, "assets/tiny16basic/characters.png"))
        {
            initClips(c_clips);

            while (GAME.running)
            {
                timer = SDL_GetTicks();

                // clear renderer
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderClear(renderer);

                playInput(e, &GAME);
                updatePlayer(&player);

                renderPlayer(GAME, c_clips);

                // put it all on screen
                SDL_RenderPresent(renderer);

                // limit framerate to ~60 fps
                delta = SDL_GetTicks() - timer;
                if (delta < TICKS) SDL_Delay(TICKS - delta);
            }
        }
    }

    freeTexture(&texture);
    return 0;
}

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

    SDL_Rect arrow_display[5] = {
        [0].w = 20,
        [0].h = 20,
        [0].x = 320,
        [0].y = 0, 

        [1].w = 20,
        [1].h = 20,
        [1].x = 320 - 20,
        [1].y = 20, 

        [2].w = 20,
        [2].h = 20,
        [2].x = 320,
        [2].y = 20, 

        [3].w = 20,
        [3].h = 20,
        [3].x = 320 + 20,
        [3].y = 20,

        [4].w = 60,
        [4].h = 20,
        [4].x = 320 - 80,
        [4].y = 20,
    };

    player player = {
        .clips = (SDL_Rect *)&c_clips,
        .w = 16,
        .h = 16,
        .x = 320 - 16,
        .y = 240 - 16,
        .dir = -1,
        .face = DOWN,
        .xvel = 0,
        .yvel = 0,
        .input = {0, 0, 0, 0},
        .moving = false,
        .attacking = false,
        .a_hold = false,
        .acounter = 0,
        .aindex = 1,
        .atk_counter = 0,
        .i_queue = {255, 255 ,255 ,255}
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

                playerInput(e, &GAME);
                updatePlayer(&player);
                renderPlayer(GAME);

                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[0]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[1]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[2]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[3]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[4]);

                SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);

                if (player.input[0])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[0]);
                if (player.input[1])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[1]);
                if (player.input[2])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[2]);
                if (player.input[3])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[3]);
                if (player.input[4])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[4]);

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

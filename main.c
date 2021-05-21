#include "util.h"

int main(int argc, char const *argv[])
{
    const int FPS = 60;
    const int TICKS = 1000 / FPS;

    const int W_WIDTH = 640;
    const int W_HEIGHT = 480;

    SDL_Window *window;
    SDL_Renderer *renderer;
    Texture c_texture;
    Texture e_texture;

    SDL_Event e;
    SDL_Rect c_clips[21]; // player/character clips
    SDL_Rect e_clips[5]; // what do here ... - enemy clips

    unsigned char map_blocks[24][32];

    SDL_Rect block = {
        .w = 20,
        .h = 20,
        .x = 0,
        .y = 0
    };

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

    SDL_Rect player_box = {
        .w = 2, .h = 2, 
        .x = 320 - 32, .y = 240 - 32
    };

    player player1 = {
        .clips = (SDL_Rect *)&c_clips,
        .w = 14,
        .h = 30,
        .x = 320 - 32,
        .y = 240 - 32,
        .rx = (320 - 32) / 20,
        .ry = (240 - 32) / 20,
        .dir = -1,
        .face = DOWN,
        .xvel = 0,
        .yvel = 0,
        .input = {0, 0, 0, 0},
        .moving = false,
        .attacking = false,
        .a_hold = false,
        .sprint = false,
        .acounter = 0,
        .aindex = 1,
        .atk_counter = 0,
        .i_queue = {255, 255 ,255 ,255},
        .a_hitBox = {.w = 1, .h = 1, .x = player1.x + 8, .y = player1.y + 40}
    };

    player player2;

    game GAME = {
        .window = &window,
        .renderer = &renderer,
        .c_texture = &c_texture,
        .e_texture = &e_texture,
        .p = &player1,
        .p2 = &player2,
        .running = true,
        .host = false,
        .client = false
    };

    int timer, delta;
    thrd_t nw_thread;

    if (initSdl(&window, &renderer, W_WIDTH, W_HEIGHT))
    {
        if (initTextureMap(&renderer, &c_texture, "assets/doomed_looters/warrior-Sheet.png"))
        //& initTextureMap(&renderer, &e_texture, "assets/Rogue-Like-8x8/Enemies.png"))
        {
            c_initClips(c_clips);
            //e_initClips(e_clips);
            
            memset(map_blocks, 0, (32 * 24));

            while (GAME.running)
            {
                timer = SDL_GetTicks();

                // clear renderer
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderClear(renderer);

                playerInput(e, &GAME, &nw_thread);
                updatePlayer(&player1);

                player_box.x = player1.x;
                player_box.y = player1.y;
                
                for (int y = 0; y < 24; y++)
                {
                    for (int x = 0; x < 32; x++)
                    {
                        block.x = x * 20;
                        block.y = y * 20;

                        if (collision(x * 20, y * 20, player1.x, player1.y)) 
                            map_blocks[y][x] = 1;

                        if (player1.attacking)
                        {
                            // check ranged attack spot
                            if (collision(x * 20, y * 20, player1.rx, player1.ry)) 
                                map_blocks[y][x] = 1;
                        }
                        
                        if (map_blocks[y][x]) 
                        {
                            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
                            SDL_RenderFillRect(*GAME.renderer, &block);
                        }
                    }
                }

                renderPlayer(GAME);

                // p2 test render
                renderTexture(*GAME.renderer, GAME.c_texture, 
                    GAME.p2->x - 16, GAME.p2->y - 28, 
                    &GAME.p->clips[0], false);

                SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
                SDL_RenderFillRect(*GAME.renderer, &player_box);

                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[0]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[1]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[2]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[3]);
                SDL_RenderDrawRect(*GAME.renderer, &arrow_display[4]);

                SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);

                if (player1.input[0])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[0]);
                if (player1.input[1])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[1]);
                if (player1.input[2])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[2]);
                if (player1.input[3])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[3]);
                if (player1.input[4])
                    SDL_RenderFillRect(*GAME.renderer, &arrow_display[4]);

                // put it all on screen
                SDL_RenderPresent(renderer);

                // limit framerate to ~60 fps
                delta = SDL_GetTicks() - timer;
                if (delta < TICKS) SDL_Delay(TICKS - delta);
            }
        }
    }

    shutdown(GAME.nw.sockfd, SHUT_RDWR);

    freeTexture(&c_texture);
    //freeTexture(&e_texture);
    return 0;
}

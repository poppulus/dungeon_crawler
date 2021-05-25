#include "util.h"

int main(int argc, char const *argv[])
{
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

    player g_players[4];

    game GAME = {
        .window = &window,
        .renderer = &renderer,
        .c_texture = &c_texture,
        .e_texture = &e_texture,
        .players = g_players,
        .running = true,
        .host = false,
        .client = false,
        .kill = false,
        .c_clips = c_clips
    };

    GAME.nw.pfds = malloc(sizeof *GAME.nw.pfds * 4);

    int timer, delta;
    thrd_t nw_thread;
    short buffer[4];

    if (argc > 1) GAME.ip = argv[1];
    else GAME.ip = "0.0.0.0";

    if (initSdl(&window, &renderer))
    {
        if (initTextureMap(&renderer, &c_texture, "assets/doomed_looters/warrior-Sheet.png"))
        //& initTextureMap(&renderer, &e_texture, "assets/Rogue-Like-8x8/Enemies.png"))
        {
            c_initClips(c_clips);
            //e_initClips(e_clips);

            memset(map_blocks, 0, (32 * 24));

            setGamePlayers(g_players);

            while (GAME.running)
            {
                timer = SDL_GetTicks();

                // clear renderer
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderClear(renderer);

                playerInput(e, &GAME, &nw_thread);
                updateLocalPlayer(&g_players[PLAYER1]);
                
                for (int p = 1; p < 4; p++) updateOtherPlayer(&g_players[p]);

                for (int y = 0; y < 24; y++)
                {
                    for (int x = 0; x < 32; x++)
                    {
                        block.x = x * 20;
                        block.y = y * 20;

                        if (collision(x * 20, y * 20, g_players[PLAYER1].x, g_players[PLAYER1].y) 
                        && g_players[PLAYER1].spawned) 
                            map_blocks[y][x] = 1;
/*
                        if (player1.attacking)
                        {
                            // check ranged attack spot
                            if (collision(x * 20, y * 20, player1.rx, player1.ry)) 
                                map_blocks[y][x] = 1;
                        }
*/
                        if (map_blocks[y][x]) 
                        {
                            switch (map_blocks[y][x])
                            {
                                case 1: SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff); 
                                break;
                                case 2: SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff); 
                                break;
                                case 3: SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff); 
                                break;
                                case 4: SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x00, 0xff); 
                                break;
                            }
                            SDL_RenderFillRect(*GAME.renderer, &block);
                        }
                    }
                }
                
                renderPlayers(GAME);

                // put it all on screen
                SDL_RenderPresent(renderer);

                // limit framerate to ~60 fps
                delta = SDL_GetTicks() - timer;
                if (delta < TICKS) SDL_Delay(TICKS - delta);
            }
        }
    }

    shutdown(GAME.nw.sockfd, SHUT_RDWR);
    shutdown(GAME.nw.connfd, SHUT_RDWR);
    
    freeTexture(&c_texture);
    //freeTexture(&e_texture);
    return 0;
}

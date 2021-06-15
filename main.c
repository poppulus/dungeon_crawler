#include "util.h"

int main(int argc, char const *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    Texture c_texture;
    Texture e_texture;

    SDL_Event e;
    SDL_Rect c_clips[21]; // player/character clips
    //SDL_Rect e_clips[5]; // what do here ... - enemy clips

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
        .c_clips = c_clips,
        .state = MENU,
        .c_player = NULL,
        .nw.pfds = malloc(sizeof(*GAME.nw.pfds) * 4)
    };

    int timer, delta;
    thrd_t nw_thread;

    if (initSdl(&window, &renderer))
    {
        if (initTextureMap(&renderer, &c_texture, "assets/doomed_looters/warrior-Sheet.png"))
        //& initTextureMap(&renderer, &e_texture, "assets/Rogue-Like-8x8/Enemies.png"))
        {
            if (argc > 1) GAME.ip = argv[1];
            else GAME.ip = "0.0.0.0";

            char_initClips(c_clips);
            //e_initClips(e_clips);

            memset(map_blocks, 0, (32 * 24));

            initPlayers(g_players);

            while (GAME.running)
            {
                timer = SDL_GetTicks();

                // clear renderer
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderClear(renderer);

                switch (GAME.state)
                {
                    case MENU:
                        menuInput(e, &GAME, &nw_thread);
                    break;
                    case HOST:
                    break;
                    case JOIN:
                    break;
                    case PLAY:
                        playInput(e, &GAME);
                        updateLocalPlayer(GAME.c_player);
                        
                        for (int p = 0; p < 4; p++) 
                        {
                            if (&g_players[p] != GAME.c_player 
                            && g_players[p].spawned)
                            {
                                //updateClient(&g_players[p]);
                                updateOtherPlayer(&g_players[p]);
                            }
                        }
                        
                        checkPlayerAtkCol(g_players);
                        checkMapCollision(GAME, &block, map_blocks);

                        if (GAME.c_player->attacking) 
                        {
                            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
                            SDL_RenderFillRect(renderer, &GAME.c_player->a_hitBox);
                        }

                        setRenderOrder(GAME);
                    break;
                }

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

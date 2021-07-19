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

    game GAME = {
        .window = &window,
        .renderer = &renderer,
        .c_texture = &c_texture,
        .e_texture = &e_texture,
        .font = NULL,
        .running = true,
        .host = false,
        .client = false,
        .kill = false,
        .g_cntdwn = false,
        .s_cntdwn = false,
        .g_ready = false,
        .c_clips = c_clips,
        .state = MENU,
        .g_count = {54, 48, '\0'},
        .s_count = 51,
        .c_player = NULL,
        .nw.pfds = malloc(sizeof(*GAME.nw.pfds) * 4),
        .rules.p1_buf = {48, 48, 48, '\0'},
        .rules.p2_buf = {48, 48, 48, '\0'},
        .rules.p3_buf = {48, 48, 48, '\0'},
        .rules.p4_buf = {48, 48, 48, '\0'}
    };

    int timer, delta, r_delta, winner;
    thrd_t nw_thread;

    char g_winner[14] = "Player  wins!", 
        *g_message = NULL, 
        nmb;

    if (initSdl(&window, &renderer))
    {
        if (initTextureMap(&renderer, &c_texture, "assets/doomed_looters/warrior-Sheet.png"))
        //& initTextureMap(&renderer, &e_texture, "assets/Rogue-Like-8x8/Enemies.png"))
        {
            if (argc > 1) GAME.ip = argv[1];
            else GAME.ip = "0.0.0.0";

            GAME.font = FC_CreateFont();
            FC_LoadFont(GAME.font, renderer, "assets/early_gameboy.ttf", 14, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);

            char_initClips(c_clips);
            //e_initClips(e_clips);

            memset(map_blocks, 0, (32 * 24));

            initPlayers(GAME.players);

            while (GAME.running)
            {
                timer = SDL_GetTicks();

                // check game rule timers
                if (GAME.s_cntdwn) 
                {
                    GAME.rules.g_timer--;
                    if (GAME.rules.g_timer > 0) 
                    {
                        if (GAME.rules.g_timer % 60 == 0) GAME.s_count--;
                    }
                    else 
                    {
                        GAME.rules.g_timer = 600;
                        GAME.s_cntdwn = false;
                        GAME.g_cntdwn = true;
                    }
                }
                else if (GAME.g_cntdwn)
                {
                    GAME.rules.g_timer--;
                    if (GAME.rules.g_timer > 0) 
                    {
                        if (GAME.rules.g_timer % 60 == 0) 
                        {
                            if (GAME.g_count[1] > 48) GAME.g_count[1]--;
                            else 
                            {
                                GAME.g_count[0]--;
                                GAME.g_count[1] = 57;
                            }
                        }
                    }
                    else 
                    {
                        GAME.s_count = 51;
                        GAME.rules.g_timer = 180;
                        GAME.g_count[0] = 54;
                        GAME.g_count[1] = 57;
                        GAME.g_cntdwn = false;

                        GAME.g_done = true;

                        // decide the winner, or a draw
                        winner = decideWinner(GAME);

                        if (winner > 0)
                        {
                            nmb = winner + 48;
                            g_winner[6] = nmb;
                            g_message = g_winner;
                        }
                        else g_message = "Draw!";
                    }
                }

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

                        if (GAME.g_cntdwn)
                        {
                            updateLocalPlayer(GAME.c_player);
                            
                            for (int p = 0; p < 4; p++) 
                            {
                                if (&GAME.players[p] != GAME.c_player 
                                && GAME.players[p].spawned)
                                {
                                    //updateClient(&g_players[p]);
                                    updateOtherPlayer(&GAME.players[p]);
                                }
                            }
                            
                            checkPlayerAtkCol(GAME.players);
                            checkMapCollision(&GAME, &block, map_blocks);

                            if (GAME.c_player->attacking) 
                            {
                                SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
                                SDL_RenderFillRect(renderer, &GAME.c_player->a_hitBox);
                            }

                            FC_Draw(GAME.font, renderer, 320, 20, GAME.g_count);
                        }
                        else if (GAME.s_cntdwn) FC_Draw(GAME.font, renderer, 320, 20, &GAME.s_count);
                        else if (GAME.g_done) FC_Draw(GAME.font, renderer, 220, 200, g_message);

                        // draw player cubes
                        for (int i = 0; i < 4; i++)
                        {
                            switch (i)
                            {
                                case 0: 
                                    SDL_SetRenderDrawColor(renderer, 0x00, 0xaa, 0xff, 0xff); 
                                break;
                                case 1: 
                                    SDL_SetRenderDrawColor(renderer, 0xaa, 0xff, 0x00, 0xff); 
                                break;
                                case 2:
                                    SDL_SetRenderDrawColor(renderer, 0xff, 0xaa, 0x00, 0xff); 
                                break;
                                case 3: 
                                    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xaa, 0xff);
                                break;
                            }
                            SDL_RenderFillRect(renderer, &GAME.players[i].cube.rect);
                        }

                        setRenderOrder(GAME);
                    break;
                }

                renderScore(GAME);

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

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    window = NULL;
    renderer = NULL;

    FC_FreeFont(GAME.font);

    IMG_Quit();
    SDL_Quit();
    return 0;
}

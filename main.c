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
        .g_board = &map_blocks[0],
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
        .c_count_flag = false,
        .c_clips = c_clips,
        .state = MENU,
        .g_count = {54, 48, '\0'},
        .s_count = 53,
        .g_c_timer = 60,
        .c_player = NULL,
        .nw.pfds = malloc(sizeof(*GAME.nw.pfds) * 4),
        .rules.g_timer = 300,
        .rules.p1_score = 0,
        .rules.p2_score = 0,
        .rules.p3_score = 0,
        .rules.p4_score = 0,
        .rules.p1_buf = {48, 48, 48, '\0'},
        .rules.p2_buf = {48, 48, 48, '\0'},
        .rules.p3_buf = {48, 48, 48, '\0'},
        .rules.p4_buf = {48, 48, 48, '\0'},
        .g_winner = "Player  wins!"
    };

    int timer, delta, r_delta, winner;
    thrd_t nw_thread;

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
                if (GAME.host) host_countdown(&GAME, &winner);
                else if (GAME.client) client_countdown(&GAME, &winner);

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
                        else 
                        {
                            if (GAME.s_cntdwn) 
                            {
                                FC_Draw(GAME.font, renderer, 320, 20, &GAME.s_count);
                                for (int r = 0; r < 4; r++)
                                {
                                    if (GAME.players[r].ready) drawReadyText(GAME, r);
                                }
                            }
                            else if (GAME.g_done) 
                            {
                                drawMapTiles(GAME, &block, map_blocks);
                                FC_Draw(GAME.font, renderer, 220, 200, GAME.g_message);
                            }
                            else 
                            {
                                for (int r = 0; r < 4; r++)
                                {
                                    if (GAME.players[r].ready) drawReadyText(GAME, r);
                                    
                                    // draw player cubes, do i even want them?
                                    /*
                                    switch (r)
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
                                    SDL_RenderFillRect(renderer, &GAME.players[r].cube.rect);
                                    */
                                }
                            }
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

#include "util.h"

// sdl and rendering stuff
//
bool initSdl(SDL_Window **window, SDL_Renderer **renderer)
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // Create window
        *window = SDL_CreateWindow(
            "Dungeon Crawler", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, 
            W_WIDTH, W_HEIGHT, 
            SDL_WINDOW_SHOWN);

        if( *window == NULL )
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            // create the renderer
            *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
            if (*renderer == NULL) 
            {
                printf("Could not create renderer! %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize PNG loading
                if( !( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) )
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }
    return success;
}

bool initTextureMap(SDL_Renderer **renderer, Texture *sheet, char *str)
{
    bool success = true;
    Uint32 format;
    int access, w, h;
    
    sheet->mTexture = loadTexture(*renderer, str);

    if (sheet->mTexture == NULL) 
    { 
        success = false;
        printf("Image could not initialize! SDL Error: %s\n", SDL_GetError());
    }
    else 
    {
        if (SDL_QueryTexture(sheet->mTexture, &format, &access, &w, &h) != 0) 
        {
            success = false;
            printf("Texture invalid!\n");
        }
        
        sheet->mWidth = w;
        sheet->mHeight = h;
    }

    return success;
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, char *path)
{
    SDL_Texture *newTexture = NULL;
    SDL_Surface *loadedSurface = IMG_Load(path);

    if (loadedSurface == NULL) printf("could not load image! %s\n", IMG_GetError());
    else 
    {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

        if (newTexture == NULL) 
            printf("could not load optimised surface! %s\n", IMG_GetError());

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void renderTexture(SDL_Renderer *r, Texture *text, int x, int y, SDL_Rect *clip, const SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = {x, y, text->mWidth, text->mHeight};

    if (clip != NULL) 
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(r, text->mTexture, clip, &renderQuad, 0, NULL, flip);
}

void freeTexture(Texture *text)
{
    if (text->mTexture != NULL)
    {
        SDL_DestroyTexture(text->mTexture);
        text->mTexture = NULL;

        text->mWidth = 0;
        text->mHeight = 0;
    }
}

int sortfunc(const void *a, const void *b)
{
    return (*(int*)a - *(int*)b);
}

int r_sortfunc(const void *a, const void *b)
{
    /*
    int i = *(int*)a, 
        j = *(int*)b;

    if (i < j) return -1;
    else if (i > j) return 1;
    return 0;
    */
    return (*(int*)a < *(int*)b) - (*(int*)a > *(int*)b);
}

// game stuff
//
void char_initClips(SDL_Rect *clips)
{
    int n = 0;
    for (int i = 0; i < 21; i++)
    {
        clips[i].w = 32;
        clips[i].h = 32;
        clips[i].x = (i % 7) * 32;
        clips[i].y = n * 32;

        if ((i % 7) == 6) n++;
    }
}

void e_initClips(SDL_Rect *clips)
{
    int n = 0;
    for (int i = 0; i < 37; i++)
    {
        clips[i].w = 8;
        clips[i].h = 8;
        clips[i].x = (i % 5) * 8;
        clips[i].y = n * 8;

        if ((i % 5) == 4) n++;
    }
}

bool collision(SDL_Rect a, SDL_Rect b)
{
    int leftA = a.x, 
        leftB = b.x, 
        rightA = a.x + a.w, 
        rightB = b.x + b.w, 
        topA = a.y, 
        topB = b.y, 
        bottomA = a.y + a.h, 
        bottomB = b.y + b.h;

    if(bottomA <= topB) return false;

    if(topA >= bottomB) return false;

    if(rightA <= leftB) return false;

    if(leftA >= rightB) return false;

    return true;
}

void checkMapCollision(game *G, SDL_Rect *block, unsigned char (*map_blocks)[32])
{
    int i, j, x, y;
    bool same = false;

    for (y = 0; y < 24; y++)
    {
        for (x = 0; x < 32; x++)
        {
            block->x = x * 20;
            block->y = y * 20;

            for (i = 0; i < 4; i++)
            {
                if (G->players[i].spawned)
                {
                    SDL_Rect plr = {.x = G->players[i].x, .y = G->players[i].y, .w = 1, .h = 1};

                    if (collision(*block, plr) && (map_blocks[y][x] != (i + 1))) 
                    {
                        for (j = 0; j < 4; j++)
                        {
                            if ((&G->players[i] != &G->players[j]) && G->players[j].spawned)
                            {
                                SDL_Rect test = {.x = G->players[j].x, .y = G->players[j].y, .w = 1, .h = 1};
                                if (collision(*block, test)) 
                                {
                                    same = true; 
                                    break;
                                }
                            }
                        }
                    
                        if (!same)
                        {
                            switch (map_blocks[y][x])
                            {
                                case 1: 
                                    G->rules.p1_score--; 
                                    subPlayerScore(G->rules.p1_buf); 
                                break;
                                case 2: 
                                    G->rules.p2_score--; 
                                    subPlayerScore(G->rules.p2_buf); 
                                break;
                                case 3: 
                                    G->rules.p3_score--; 
                                    subPlayerScore(G->rules.p3_buf); 
                                break;
                                case 4: 
                                    G->rules.p4_score--; 
                                    subPlayerScore(G->rules.p4_buf); 
                                break;
                            }

                            map_blocks[y][x] = (i + 1);

                            switch (i + 1)
                            {
                                case 1: 
                                    G->rules.p1_score++; 
                                    addPlayerScore(G->rules.p1_buf); 
                                break;
                                case 2: 
                                    G->rules.p2_score++; 
                                    addPlayerScore(G->rules.p2_buf); 
                                break;
                                case 3: 
                                    G->rules.p3_score++; 
                                    addPlayerScore(G->rules.p3_buf); 
                                break;
                                case 4: 
                                    G->rules.p4_score++; 
                                    addPlayerScore(G->rules.p4_buf); 
                                break;
                            }
                        }
                    }
                }
            }
            if (map_blocks[y][x]) 
            {
                switch (map_blocks[y][x])
                {
                    case 1: SDL_SetRenderDrawColor(*G->renderer, 0x00, 0x00, 0xff, 0xff); 
                    break;
                    case 2: SDL_SetRenderDrawColor(*G->renderer, 0x00, 0xff, 0x00, 0xff); 
                    break;
                    case 3: SDL_SetRenderDrawColor(*G->renderer, 0xff, 0x00, 0x00, 0xff); 
                    break;
                    case 4: SDL_SetRenderDrawColor(*G->renderer, 0xff, 0xff, 0x00, 0xff); 
                    break;
                }
                SDL_RenderFillRect(*G->renderer, block);
                SDL_SetRenderDrawColor(*G->renderer, 0xee, 0xee, 0xee, 0xff);
                SDL_RenderDrawRect(*G->renderer, block);
            }
        }
    }
}

void checkPlayerAtkCol(player *players)
{
    SDL_Rect plr = {.x = 0, .y = 0, .w = 1, .h = 1};
    for (int i = 0; i < 4; i++)
    {
        if (players[i].attacking && players[i].spawned)
        {
            for (int j = 0; j < 4; j++)
            {
                if ((&players[i] != &players[j]) 
                && players[j].spawned && !players[j].hurt)
                {
                    plr.x = players[j].x; 
                    plr.y = players[j].y - 15;
                    if (collision(players[i].a_hitBox, plr))
                    {
                        players[j].hurt = true;
                        players[j].blocked = true;
                        switch (players[i].face)
                        {
                            case UP:
                                players[j].p_dir = PUSH_UP;
                            break;
                            case DOWN:
                                players[j].p_dir = PUSH_DOWN;
                            break;
                            case LEFT:
                                players[j].p_dir = PUSH_LEFT;
                            break;
                            case RIGHT:
                                players[j].p_dir = PUSH_RIGHT;
                            break;
                        }
                        break;
                    }
                }
            }
        }
    }
}

bool checkPlayerReady(player *p)
{
    short i = 0;
    unsigned char n = 0, m = 0;

    for (i = 0; i < 4; i++) 
        if (p[i].spawned) n++;

    for (i = 0; i < 4; i++) 
        if (p[i].spawned && p[i].ready) m++;
    
    if (m == n && (m > 1 && n > 1)) return true;

    return false;
}

void menuInput(SDL_Event e, game *g, thrd_t *nw_thread)
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: g->running = false; 
            break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                    case SDLK_h: 
                        if (!g->host && !g->client)
                        {
                            g->host = true;
                            thrd_create(nw_thread, setup_server, g);
                        }
                    break;
                    case SDLK_j:
                        if (!g->host && !g->client)
                        {
                            g->client = true;
                            thrd_create(nw_thread, connect_to_server, g);
                        }
                    break;
                    case SDLK_k:
                        g->kill = true;
                    break;
                    case SDLK_ESCAPE: g->running = false; break;
                }
            break;
        }
    }
}

void playInput(SDL_Event e, game *g)
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: g->running = false; break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                    case SDLK_k:
                        g->kill = true;
                    break;
                    case SDLK_ESCAPE: g->running = false; break;
                    case SDLK_RETURN:
                    case SDLK_RETURN2:
                    case SDLK_KP_ENTER:
                        if (!g->g_cntdwn && !g->g_done && (g->host || g->client)) 
                            g->c_player->ready = !g->c_player->ready;
                    break;
                    case SDLK_SPACE:
                        if (!e.key.repeat)
                        {
                            if (!g->c_player->hurt
                            && !g->c_player->attacking 
                            && !g->c_player->a_hold) 
                            {
                                g->c_player->aindex = 3;
                                g->c_player->a_hold = true;
                                enqueue(g->c_player->i_queue, I_ATK);
                                g->c_player->attacking = true;
                                setAttackBox(g->c_player);
                            }
                        }
                    break;
                    case SDLK_LSHIFT: 
                        if (!e.key.repeat) 
                            g->c_player->sprint = true;
                    break;
                    case SDLK_w:
                        if (!e.key.repeat) 
                            enqueue(g->c_player->i_queue, I_UP);
                    break;
                    case SDLK_s:
                        if (!e.key.repeat) 
                            enqueue(g->c_player->i_queue, I_DOWN);
                    break;
                    case SDLK_a:
                        if (!e.key.repeat) 
                            enqueue(g->c_player->i_queue, I_LEFT);
                    break;
                    case SDLK_d:
                        if (!e.key.repeat) 
                            enqueue(g->c_player->i_queue, I_RIGHT);
                    break;
                }
            break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        g->c_player->a_hold = false; 
                    break;
                    case SDLK_LSHIFT: 
                        g->c_player->sprint = false; 
                    break;
                    case SDLK_w:
                        dequeue(g->c_player->i_queue, I_UP);
                    break;
                    case SDLK_a:
                        dequeue(g->c_player->i_queue, I_LEFT);
                    break;
                    case SDLK_s:
                        dequeue(g->c_player->i_queue, I_DOWN);
                    break;
                    case SDLK_d:
                        dequeue(g->c_player->i_queue, I_RIGHT);
                    break;
                }
            break;
        }
    }
}

void initPlayers(player p[4])
{
    for (int i = 0; i < 4; i++)
    {
        setPlayerState(&p[i]);
        p[i].cube.plr = i + 1;
        switch (i)
        {
            case 0:
                p[i].x = 10;
                p[i].y = 10;
                p[i].cube.rect.x = 30;
                p[i].cube.rect.y = 30;
            break;
            case 1: 
                p[i].x = W_WIDTH - 10;
                p[i].y = 10;
                p[i].cube.rect.x = W_WIDTH - 50;
                p[i].cube.rect.y = 30;
            break;
            case 2: 
                p[i].x = 10;
                p[i].y = W_HEIGHT - 10;
                p[i].cube.rect.x = 30;
                p[i].cube.rect.y = W_HEIGHT - 50;
            break;
            case 3:
                p[i].x = W_WIDTH - 10;
                p[i].y = W_HEIGHT - 10;
                p[i].cube.rect.x = W_WIDTH - 50;
                p[i].cube.rect.y = W_HEIGHT - 50;
            break;
        }
    }
}

void setPlayerState(player *p)
{
    memset(p, 0, sizeof(*p));
    p->a_hitBox.w = 20;
    p->a_hitBox.h = 20;
    p->cube.rect.w = 10;
    p->cube.rect.h = 10;
    p->cube.xvel = 0;
    p->cube.yvel = 0;
}

void setAttackBox(player *p)
{
    switch (p->face)
    {
        case UP:
            p->a_hitBox.x = p->x - 10; 
            p->a_hitBox.y = p->y - 40;
        break;
        case DOWN:
            p->a_hitBox.x = p->x - 10; 
            p->a_hitBox.y = p->y - 10;
        break;
        case LEFT:
            p->a_hitBox.x = p->x - 20; 
            p->a_hitBox.y = p->y - 30;
        break;
        case RIGHT:
            p->a_hitBox.x = p->x; 
            p->a_hitBox.y = p->y - 30;
        break;
    }
}

void selectPlayerSlot(player *players, int cfd)
{
    for (int i = 0; i < 4; i++)
    {
        if (players[i].nid == 0)
        {
            players[i].nid = cfd;
            players[i].spawned = true;
            break;
        }
    }
}

void removePlayerSlot(player *players, int cfd)
{
    for (int i = 0; i < 4; i++)
    {
        if (players[i].nid == cfd)
        {
            setPlayerState(&players[i]);
            switch (i)
            {
                case 1:players[i].x = (W_WIDTH) - 1;
                break;
                case 2:players[i].y = (W_HEIGHT) - 1;
                break;
                case 3:
                    players[i].x = (W_WIDTH) - 1;
                    players[i].y = (W_HEIGHT) - 1;
                break;
            }
            break;
        }
    }
}

void updateLocalPlayer(player *p)
{
    p->xvel = 0;
    p->yvel = 0;

    p->dir = p->i_queue[0];

    if (p->hurt)
    {
        p->hurt_counter++;

        if (p->push_counter < 15)
        {
            p->push_counter++;
            switch (p->p_dir)
            {
                case PUSH_UP:
                    p->yvel = -4;
                break;
                case PUSH_DOWN:
                    p->yvel = 4;
                break;
                case PUSH_LEFT:
                    p->xvel = -4;
                break;
                case PUSH_RIGHT:
                    p->xvel = 4;
                break;
            }
        }
        else if ((p->push_counter >= 15) && p->blocked) 
            p->blocked = false;
        
        if (p->hurt_counter >= 60)
        {
            p->push_counter = 0;
            p->hurt_counter = 0;
            p->hurt = false;
        }
    }

    if (!p->attacking && !p->blocked)
    {
        switch (p->dir)
        {
            case I_UP: 
                p->face = UP;
                p->yvel = p->sprint ? -2 : -1;
            break;
            case I_LEFT: 
                p->face = LEFT;
                p->xvel = p->sprint ? -2 : -1;
            break;
            case I_DOWN: 
                p->face = DOWN;
                p->yvel = p->sprint ? 2 : 1;
            break;
            case I_RIGHT: 
                p->face = RIGHT;
                p->xvel = p->sprint ? 2 : 1;
            break;
        }
    }

    animatePlayer(p);

    if (p->x + p->xvel <= 0 || p->x + p->xvel >= 640)
        p->xvel = 0;
    
    if (p->y + p->yvel <= 0 || p->y + p->yvel >= 480)
        p->yvel = 0;

    p->x += p->xvel;
    p->y += p->yvel;
}

void updateOtherPlayer(player *p)
{
    if (p->spawned)
    {
        if (p->hurt)
        {
            p->hurt_counter++;

            if (p->push_counter < 15)
            {
                p->push_counter++;
                switch (p->p_dir)
                {
                    case PUSH_UP:
                        p->yvel = -4;
                    break;
                    case PUSH_DOWN:
                        p->yvel = 4;
                    break;
                    case PUSH_LEFT:
                        p->xvel = -4;
                    break;
                    case PUSH_RIGHT:
                        p->xvel = 4;
                    break;
                }
            }
            else if ((p->push_counter >= 15) && p->blocked) 
                p->blocked = false;
            
            if (p->hurt_counter >= 60)
            {
                p->push_counter = 0;
                p->hurt_counter = 0;
                p->hurt = false;
            }
        }
        // animation
        if (p->nextmove.attacking)
        {
            p->acounter = 0;
            p->aindex = 4;

            switch (p->face)
            {
                case UP:
                    p->a_hitBox.x = p->x - 10; 
                    p->a_hitBox.y = p->y - 40;
                    p->a_hitBox.w = 20;
                    p->a_hitBox.h = 20;
                break;
                case DOWN:
                    p->a_hitBox.x = p->x - 10; 
                    p->a_hitBox.y = p->y - 10;
                    p->a_hitBox.w = 20;
                    p->a_hitBox.h = 20;
                break;
                case LEFT:
                    p->a_hitBox.x = p->x - 20; 
                    p->a_hitBox.y = p->y - 30;
                    p->a_hitBox.w = 20;
                    p->a_hitBox.h = 20;
                break;
                case RIGHT:
                    p->a_hitBox.x = p->x; 
                    p->a_hitBox.y = p->y - 30;
                    p->a_hitBox.w = 20;
                    p->a_hitBox.h = 20;
                break;
            }

            if (p->nextmove.atk_counter < 5) p->aindex = 4;

            else if (p->nextmove.atk_counter >= 5 
            && p->nextmove.atk_counter < 10)
                p->aindex = 5;
            
            else if (p->nextmove.atk_counter >= 10) 
                p->aindex = 6;

            if (p->nextmove.atk_counter + 1 == 15) p->aindex = 0;
        }
        else
        {
            p->acounter++;

            if (p->nextmove.x != p->x && p->nextmove.y == p->y)
            {
                if (p->nextmove.x > p->x) p->face = RIGHT;
                else if (p->nextmove.x < p->x) p->face = LEFT;
                
                if (((p->nextmove.x == p->x + 1) 
                || (p->nextmove.x == p->x - 1)) 
                && (p->acounter % 8 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
                else if (((p->nextmove.x == p->x + 2) 
                || (p->nextmove.x == p->x - 2)) 
                && (p->acounter % 6 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
            }
            else if (p->nextmove.y != p->y && p->nextmove.x == p->x)
            {
                if (p->nextmove.y > p->y) p->face = DOWN;
                else if (p->nextmove.y < p->y) p->face = UP;
                
                if (((p->nextmove.y == p->y + 1) 
                || (p->nextmove.y == p->y - 1)) 
                && (p->acounter % 8 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
                else if (((p->nextmove.y == p->y + 2) 
                || (p->nextmove.y == p->y - 2)) 
                && (p->acounter % 6 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
            }
            else
            {
                if (p->acounter % 32 == 0)
                {
                    p->aindex++;
                    p->aindex %= 2;
                }
            }
        }
        //

        p->attacking = p->nextmove.attacking;
        p->atk_counter = p->nextmove.atk_counter;

        p->x = p->nextmove.x;
        p->y = p->nextmove.y;
    }
}

void updateClient(player *p)
{
    p->xvel = 0;
    p->yvel = 0;

    p->dir = p->i_queue[0];

    switch (p->dir)
    {
        case I_UP: 
            p->yvel = -1;
            p->face = UP;
        break;
        case I_LEFT: 
            p->xvel = -1;
            p->face = LEFT;
        break;
        case I_DOWN: 
            p->yvel = 1;
            p->face = DOWN;
        break;
        case I_RIGHT: 
            p->xvel = 1;
            p->face = RIGHT;
        break;
    }

    //animatePlayer(p);

    if (p->x + p->xvel <= 0 || p->x + p->xvel >= 640)
        p->xvel = 0;
    
    if (p->y + p->yvel <= 0 || p->y + p->yvel >= 480)
        p->yvel = 0;

    //p->x += p->xvel;
    //p->y += p->yvel;

    p->nextmove.x = p->x + p->xvel;
    p->nextmove.y = p->y + p->yvel;
}

void addPlayerScore(char *buf)
{
    if (buf[2] < 57) buf[2]++;
    else
    {
        buf[2] = 48;
        if (buf[1] < 57) buf[1]++;
        else
        {
            buf[1] = 48;
            if (buf[0] < 57) buf[0]++; 
        }
    }
}

void subPlayerScore(char *buf)
{
    if (buf[2] > 48) buf[2]--;
    else
    {
        buf[2] = 57;
        if (buf[1] > 48) buf[1]--;
        else
        {
            buf[1] = 57;
            if (buf[0] > 48) buf[0]--; 
        }
    }
}

void setMapTColor(SDL_Renderer *r, player p)
{
    switch (p.color)
    {
        case BLUE: 
            SDL_SetRenderDrawColor(r, 0x00, 0x00, 0xff, 0xff); 
        break;
        case GREEN:
            SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff); 
        break;
        case RED:
            SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff); 
        break;
        case YELLOW:
            SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff); 
        break;
    }
}

void animatePlayer(player *p)
{
    if (p->attacking)
    {
        if (p->atk_counter % 5 == 0)
        {
            p->aindex++;
            /*
            switch (p->face)
            {
                case DOWN:
                    p->rx = p->x;
                    p->ry = p->y + ((p->aindex - 3) * 20);
                break;
                case UP:
                    p->rx = p->x;
                    p->ry = p->y - ((p->aindex - 3) * 20);
                break;
                case LEFT:
                    p->rx = p->x - ((p->aindex - 3) * 20);
                    p->ry = p->y;
                break;
                case RIGHT:
                    p->rx = p->x + ((p->aindex - 3) * 20);
                    p->ry = p->y;    
                break;
            }
            */
        }

        p->atk_counter++;

        if (p->atk_counter >= 15)
        {
            p->atk_counter = 0;
            p->aindex = 0;
            dequeue(p->i_queue, I_ATK);
            p->attacking = false;
            /*
            p->rx = p->x;
            p->ry = p->y;
            */
        }
    }
    else
    {
        p->acounter++;

        if ((p->xvel != 0) || (p->yvel != 0)) 
        {
            if (p->sprint && (p->acounter % 6 == 0))
            {
                p->aindex++;
                p->aindex = (p->aindex % 2) + 2;
            }
            else if (!p->sprint && (p->acounter % 8 == 0))
            {
                p->aindex++;
                p->aindex = (p->aindex % 2) + 2;
            }
        }
        else 
        {
            if (p->acounter % 32 == 0)
            {
                p->aindex++;
                p->aindex %= 2;
            }
        }
    }
}

void renderPlayer(game G, player *p)
{
    bool flip = (p->face == LEFT ? true : false);
    int c_index;

    if (p->face == LEFT) c_index = (2 * 7) + p->aindex;
    else c_index = (p->face * 7) + p->aindex;

    if (p->hurt && ((p->hurt_counter % 4) == 0))
    {
        renderTexture(*G.renderer, G.c_texture, 
            p->x - 16, p->y - 30, 
            &G.c_clips[c_index], flip);
    }
    else if (!p->hurt)
    {
        renderTexture(*G.renderer, G.c_texture, 
            p->x - 16, p->y - 30, 
            &G.c_clips[c_index], flip);
    }
}

void setRenderOrder(game G)    // kind of slow?
{
    int order[4];

    for (int i = 0; i < 4; i++)
    {
        order[i] = -1;
        if (G.players[i].spawned) 
            order[i] = G.players[i].y;
    }

    qsort(order, 4, sizeof(int), sortfunc);

    for (int j = 0; j < 4; j++)
    {
        if (order[j] != -1)
        {
            for (int m = 0; m < 4; m++)
            {
                if ((G.players[m].y == order[j]) && G.players[m].spawned) 
                    renderPlayer(G, &G.players[m]);
            }
        }
    }
}

int decideWinner(game G)
{
    int i, 
        order[4] = {
            G.rules.p1_score, 
            G.rules.p2_score, 
            G.rules.p3_score, 
            G.rules.p4_score
        };

    qsort(order, 4, sizeof(int), sortfunc);

    if (order[3] == order[2] 
    || order[3] == order[1] 
    || order[3] == order[0]) return 0;
    else if (order[3] == G.rules.p1_score) return 1;
    else if (order[3] == G.rules.p2_score) return 2;
    else if (order[3] == G.rules.p3_score) return 3;
    else if (order[3] == G.rules.p4_score) return 4;

    return -1;
}

void resetTimer(game *G)
{
    G->rules.g_timer = 300;
    G->s_count = 53;
    G->g_count[0] = 54;
    G->g_count[1] = 48;
}

void resetScore(g_rules *r) // make this complete !!!
{
    for (int i = 0; i < 4; i++)
    {
        switch (i)
        {
            case 0: resetScoreBuffer(r->p1_buf); break;
            case 1: resetScoreBuffer(r->p2_buf); break;
            case 2: resetScoreBuffer(r->p3_buf); break;
            case 3: resetScoreBuffer(r->p4_buf); break;
        }
    }
    
    r->p1_score = 0;
    r->p2_score = 0;
    r->p3_score = 0;
    r->p4_score = 0;
}

void resetScoreBuffer(char b[])
{
    b[0] = 48;
    b[1] = 48;
    b[2] = 48;
    b[3] = '\0';
}

void resetPlayers(player p[])
{
    for (int i = 0; i < 4; i++)
    {
        bzero(&p[i].nextmove, sizeof(p_next));
        p[i].face = 0;
        p[i].dir = 0; 
        p[i].p_dir = 0;
        p[i].xvel = 0; 
        p[i].yvel = 0;
        switch (i)
        {
            case 0: 
                p[i].x = 10; 
                p[i].y = 10;
            break;
            case 1: 
                p[i].x = W_WIDTH - 10; 
                p[i].y = 10;
            break;
            case 2: 
                p[i].x = 10;
                p[i].y = W_HEIGHT - 10; 
            break;
            case 3: 
                p[i].x = W_WIDTH - 10; 
                p[i].y = W_HEIGHT - 10;
            break;
        }
        resetPlayerTimers(&p[i]);
        resetPlayerState(&p[i]);
    }
}

void resetPlayerTimers(player *p)
{
    for (int i = 0; i < Q_SIZE; i++) 
        p->i_queue[i] = 0;
    
    p->acounter = 0; 
    p->aindex = 0;
    p->atk_counter = 0;
    p->hurt_counter = 0;
    p->push_counter = 0;
}

void resetPlayerState(player *p)
{
    p->blocked = false; 
    p->attacking = false; 
    p->a_hold = false; 
    p->sprint = false; 
    p->ready = false; 
    p->hurt = false;
}

void renderScore(game G)
{
    //p1
    FC_Draw(G.font, *G.renderer, 20, 20, G.rules.p1_buf);
    //p2
    FC_Draw(G.font, *G.renderer, W_WIDTH - 60, 20, G.rules.p2_buf);
    //p3
    FC_Draw(G.font, *G.renderer, 20, W_HEIGHT - 40, G.rules.p3_buf);
    //p4
    FC_Draw(G.font, *G.renderer, W_WIDTH - 60, W_HEIGHT - 40, G.rules.p4_buf);
}

void drawMapTiles(game G, SDL_Rect *block, unsigned char (*map_blocks)[32])
{
    int x, y;
    for (y = 0; y < 24; y++)
    {
        for (x = 0; x < 32; x++)
        {
            block->x = x * 20;
            block->y = y * 20;

            if (map_blocks[y][x])
            {
                switch (map_blocks[y][x])
                {
                    case 1: SDL_SetRenderDrawColor(*G.renderer, 0x00, 0x00, 0xff, 0xff); 
                    break;
                    case 2: SDL_SetRenderDrawColor(*G.renderer, 0x00, 0xff, 0x00, 0xff); 
                    break;
                    case 3: SDL_SetRenderDrawColor(*G.renderer, 0xff, 0x00, 0x00, 0xff); 
                    break;
                    case 4: SDL_SetRenderDrawColor(*G.renderer, 0xff, 0xff, 0x00, 0xff); 
                    break;
                }
                SDL_RenderFillRect(*G.renderer, block);
                SDL_SetRenderDrawColor(*G.renderer, 0xee, 0xee, 0xee, 0xff);
                SDL_RenderDrawRect(*G.renderer, block);
            }
        }
    }
}

void drawReadyText(game G, int r)
{
    switch (r)
    {
        case 0:
            FC_Draw(G.font, *G.renderer, 20, 40, "Ready!");
        break;
        case 1:
            FC_Draw(G.font, *G.renderer, W_WIDTH - 100, 40, "Ready!");
        break;
        case 2:
            FC_Draw(G.font, *G.renderer, 20, W_HEIGHT - 60, "Ready!");
        break;
        case 3:
            FC_Draw(G.font, *G.renderer, W_WIDTH - 100, W_HEIGHT - 60, "Ready!");
        break;
    }
}

void enqueue(unsigned char *q, unsigned char val)
{
    for (int i = Q_SIZE - 1; i > 0; i--)
        q[i] = q[i - 1];
    
    if (q[0] == I_ATK) q[1] = val;
    else q[0] = val;
}

void dequeue(unsigned char *q, unsigned char val)
{
    bool found = false;

    for (int i = 0; i < Q_SIZE; i++)
    {
        if (q[i] == val) 
            found = true;

        if (found && ((i + 1) < Q_SIZE)) 
            q[i] = q[i + 1];
    }

    q[Q_SIZE - 1] = 0;
}

int setup_server(void *ptr)
{
    game *G = ptr;
    network *nw = &G->nw;

    // socket create and verification
    nw->sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (nw->sockfd == -1) {
		printf("socket creation failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully created..\n");

    if (setsockopt(nw->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        thrd_exit(1);
    }

    bzero(&nw->servaddr, sizeof(nw->servaddr));

	// assign IP, PORT
	nw->servaddr.sin_family = AF_INET;
	nw->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	nw->servaddr.sin_port = htons(8008);

	// Binding newly created socket to given IP and verification
	if ((bind(nw->sockfd, (struct sockaddr*)&nw->servaddr, sizeof(nw->servaddr))) != 0) {
		printf("socket bind failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully bound..\n");

	nw->addrlen = sizeof(nw->cli);

    // Now server is ready to listen and verification
	if ((listen(nw->sockfd, 10)) == -1) {
		perror("listen");
        thrd_exit(1);
	}
	
	printf("Server listening..\n");

    // just for testing, it makes sense with player1 as the host though ...
    G->c_player = &G->players[PLAYER1];
    G->c_player->spawned = true;
    G->c_player->nid = nw->sockfd;
    G->state = PLAY;

    host_loop(G);

    G->state = MENU;
    G->host = false;
    G->kill = false;

	// Accept the data packet from client and verification
    /*
	nw->connfd = accept(nw->sockfd, (struct sockaddr*)&nw->cli, &nw->addrlen);
	if (nw->connfd < 0) {
		printf("server acccept failed...\n");
	}
	else
    {
        printf("server acccept the client...\n");

        // just for testing !!!
        G->players[PLAYER2].nid = nw->connfd;
        G->players[PLAYER1].spawned = true;
        G->players[PLAYER2].spawned = true;

        host_loop(G);
        G->host = false;
        G->kill = false;
    }
    */

    close(G->nw.sockfd);

    printf("host thread terminated\n");
    thrd_exit(0);
    return 0;
}

int connect_to_server(void *ptr)
{
    game *G = ptr;
    network *nw = &G->nw;

    // socket create and varification
	nw->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (nw->sockfd == -1) {
		printf("socket creation failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully created..\n");

	bzero(&nw->servaddr, sizeof(nw->servaddr));

	// assign IP, PORT
	nw->servaddr.sin_family = AF_INET;
	nw->servaddr.sin_addr.s_addr = inet_addr(G->ip);
	nw->servaddr.sin_port = htons(8008);

    if (connect(nw->sockfd, (struct sockaddr*)&nw->servaddr, sizeof(nw->servaddr)) != 0) {
		printf("connection with the server failed...\n");
        thrd_exit(1);
	}
	else 
    {
        printf("connected to the server..\n");

        short buffer[6];

        // testing !!!
        G->c_player = NULL;

        if (recv(G->nw.sockfd, buffer, sizeof(buffer), 0) == -1) 
            perror("recv");
        else 
        {
            G->nw.connfd = buffer[0];
            client_loop(G);
        }
    }

    G->state = MENU;
    G->client = false;
    G->kill = false;

    close(G->nw.sockfd);

    printf("client thread terminated\n");
    thrd_exit(0);
    return 0;
}

void host_loop(game *G)
{
    Uint32 timer, delta;
    int nbytes, 
        p, i, j, 
        fd_counter = 1, poll_count;
    short buffer[6];
    short send_buf[27];
    bool q = false;

    G->nw.pfds[0].fd = G->nw.sockfd;
    G->nw.pfds[0].events = POLLIN;

    printf("host loop\n");

    while (!q && G->running && !G->kill)
    {
        timer = SDL_GetTicks();

        poll_count = poll(G->nw.pfds, fd_counter, 100);

        if (poll_count == -1)
        {
            perror("poll");
            thrd_exit(1);
        }

        for (p = 0; p < fd_counter; p++)
        {
            if (G->nw.pfds[p].revents & POLLIN)
            {
                if (G->nw.pfds[p].fd == G->nw.sockfd)
                {
                    G->nw.addrlen = sizeof(G->nw.cli);
                    G->nw.connfd = accept(G->nw.sockfd, (struct sockaddr*)&G->nw.cli, &G->nw.addrlen);

                    if (G->nw.connfd == -1) perror("accept");
                    else
                    {
                        if (setsockopt(G->nw.connfd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int)) < 0)
                            perror("setsocket");
                        else
                        {
                            printf("server acccepted the client...\n");
                            G->nw.pfds[fd_counter].fd = G->nw.connfd;
                            G->nw.pfds[fd_counter].events = POLLIN;

                            selectPlayerSlot(G->players, G->nw.connfd);

                            buffer[0] = G->nw.connfd;

                            if (send(G->nw.connfd, buffer, sizeof(buffer), 0) == -1) 
                                perror("send");

                            fd_counter++;
                        }
                    }
                }
                else 
                {
                    nbytes = recv(G->nw.pfds[p].fd, buffer, sizeof(buffer), 0);

                    if (nbytes <= 0)
                    {
                        if (nbytes == 0) printf("socket %d hung up\n", G->nw.pfds[p].fd);
                        else perror("recv");

                        close(G->nw.pfds[p].fd);
                        removePlayerSlot(G->players, G->nw.pfds[p].fd);
                        G->nw.pfds[p] = G->nw.pfds[fd_counter - 1];
                        fd_counter--;
                    }
                    else
                    {
                        for (i = 1; i < 4; i++)
                        {
                            if (G->nw.pfds[p].fd == G->players[i].nid)
                            {
                                G->players[i].nextmove.x = buffer[1];
                                G->players[i].nextmove.y = buffer[2];
                                G->players[i].nextmove.attacking = buffer[3];
                                G->players[i].nextmove.atk_counter = buffer[4];
                                G->players[i].ready = buffer[5];
                            }
                        }
                        bzero(buffer, sizeof(buffer));
                    }
                }
            }
        }

        //  check if all connected players are ready
        if (!G->g_ready && checkPlayerReady(G->players))
        {
            printf("all players ready\n");
            G->g_ready = true;
            G->s_cntdwn = true;
        }
        else if (G->g_ready && !checkPlayerReady(G->players))
        {
            printf("player(s) not ready\n");
            G->g_ready = false;
            G->s_cntdwn = false;
            resetTimer(G);
        }

        h_player_update(send_buf, G->players);

        // testing countdown
        if (G->s_cntdwn) send_buf[NW_S_COUNT] = G->s_count;
        else if (G->g_cntdwn) send_buf[NW_G_COUNT] = G->g_c_timer;
        else if (G->g_done) send_buf[NW_G_END] = 1;
        else 
        {
            send_buf[NW_S_COUNT] = 0;
            send_buf[NW_G_COUNT] = 0;
            send_buf[NW_G_END] = 0;
        }
        //

        for (j = 1; j < 4; j++)
        {
            if (G->players[j].spawned)
            {
                if (send(G->players[j].nid, send_buf, sizeof(send_buf), 0) == -1) 
                    perror("send");
            }
        }

        /* udp
        recvfrom(G->nw.sockfd, buffer, sizeof(buffer), MSG_WAITALL, 
            (struct sockaddr*)&G->nw.cli, 
            &G->nw.addrlen);

        sendto(G->nw.sockfd, buffer, sizeof(buffer), MSG_CONFIRM, 
                (struct sockaddr*)&G->nw.cli, 
                G->nw.addrlen);
        */

        bzero(send_buf, sizeof(send_buf));

        // set tickrate to ~60
        delta = SDL_GetTicks() - timer;
        if (delta < TICKS) SDL_Delay(TICKS  - delta);
    }
}

void client_loop(game *G)
{
    int i, nbytes, timer, delta;
    short buffer[6];
    short down_buf[27];
    bool q = false;

    printf("client loop\n");

    while (!q && G->running && !G->kill)
    {
        //timer = SDL_GetTicks();

        /* udp
            recvfrom(G->nw.sockfd, buffer, sizeof(buffer), MSG_WAITALL, 
                (struct sockaddr*)&G->nw.servaddr, 
                &G->nw.addrlen); 

            sendto(G->nw.sockfd, buffer, sizeof(buffer), MSG_CONFIRM, 
                (struct sockaddr*)&G->nw.servaddr, 
                sizeof(G->nw.servaddr));
        */

        buffer[0] = G->nw.connfd;

        if (G->c_player != NULL)
        { 
            buffer[1] = G->c_player->x;
            buffer[2] = G->c_player->y;
            buffer[3] = G->c_player->attacking;
            buffer[4] = G->c_player->atk_counter; 
            buffer[5] = G->c_player->ready;
        }

        if (send(G->nw.sockfd, buffer, sizeof(buffer), 0) == -1)
            perror("send");

        bzero(buffer, sizeof(buffer));

        nbytes = recv(G->nw.sockfd, down_buf, sizeof(down_buf), 0);

        if (nbytes <= 0)
        {
            if (nbytes == 0) printf("socket %d hung up\n", G->nw.sockfd);
            else perror("recv");
            close(G->nw.sockfd);

            initPlayers(G->players);
            G->c_player = NULL;
            G->state = MENU;
            q = true;
        }
        else
        {
            client_sync(G, down_buf);

            for (i = 0; i < 4; i++)
            {
                if (G->players[i].nid != 0) 
                    c_player_update(G, i, down_buf);
                else
                {
                    if (down_buf[(i * NW_P_SIZE)] != 0)
                    {
                        if (down_buf[(i * NW_P_SIZE)] == G->nw.connfd 
                        && G->c_player == NULL)
                        {
                            G->c_player = &G->players[i];
                            G->state = PLAY;
                        }
                        G->players[i].nid = down_buf[(i * NW_P_SIZE)];
                        G->players[i].spawned = true;
                    }
                }
            }
            bzero(down_buf, sizeof(down_buf));
        }

        // set tickrate to ~60
        //delta = SDL_GetTicks() - timer;
        //if (delta < TICKS) SDL_Delay(TICKS  - delta);
    }
}

void c_player_update(game *G, int i, short *buffer)
{
    if (buffer[(i * NW_P_SIZE)] == 0)
    {
        setPlayerState(&G->players[i]);
        switch (i)
        {
            case 1:G->players[i].x = (W_WIDTH) - 1;
            break;
            case 2:G->players[i].y = (W_HEIGHT) - 1;
            break;
            case 3:
                G->players[i].x = (W_WIDTH) - 1;
                G->players[i].y = (W_HEIGHT) - 1;
            break;
        }
        return;
    }
    G->players[i].nextmove.x = buffer[(i * NW_P_SIZE) + 1];
    G->players[i].nextmove.y = buffer[(i * NW_P_SIZE) + 2];
    G->players[i].nextmove.attacking = buffer[(i * NW_P_SIZE) + 3];
    G->players[i].nextmove.atk_counter = buffer[(i * NW_P_SIZE) + 4];
    
    if (G->c_player != &G->players[i]) 
        G->players[i].ready = buffer[(i * NW_P_SIZE) + 5];
}

void h_player_update(short *buffer, player *players)
{
    buffer[N_P1] = players[PLAYER1].nid;
    buffer[N_P1+1] = players[PLAYER1].x;
    buffer[N_P1+2] = players[PLAYER1].y;
    buffer[N_P1+3] = players[PLAYER1].attacking;
    buffer[N_P1+4] = players[PLAYER1].atk_counter;
    buffer[N_P1+5] = players[PLAYER1].ready;
    
    for (int i = 1; i < 4; i++)
    {
        buffer[(i * NW_P_SIZE)] = players[i].nid;
        if (players[i].nid != 0)
        {
            buffer[(i * NW_P_SIZE) + 1] = players[i].nextmove.x;
            buffer[(i * NW_P_SIZE) + 2] = players[i].nextmove.y;
            buffer[(i * NW_P_SIZE) + 3] = players[i].nextmove.attacking;
            buffer[(i * NW_P_SIZE) + 4] = players[i].nextmove.atk_counter;
            buffer[(i * NW_P_SIZE) + 5] = players[i].ready;
        }
    }
}

void host_sync()
{

}

void client_sync(game *G, short *buffer)
{
    if (buffer[NW_G_COUNT] > 0)
    {
        G->g_cntdwn = true;
        if (buffer[NW_G_COUNT] < G->g_c_timer) 
        {
            G->g_c_timer--;
            G->c_count_flag = true;
        }
    }
    else if (buffer[NW_S_COUNT] > 0)
    {
        G->s_cntdwn = true;
        if (buffer[NW_S_COUNT] < G->s_count) G->s_count--;
    } 
    else if (buffer[NW_G_END] > 0 && !G->g_done) G->g_done = true;
    else if (buffer[NW_G_END] == 0 && G->g_done) 
    {
        G->g_done = false;
        bzero(G->g_board, 32*24);
        resetPlayers(G->players);
        resetScore(&G->rules);
    }
    else 
    {
        G->s_cntdwn = false;
        G->g_cntdwn = false;
        G->s_count = 53;
    }
}

void host_countdown(game *G, int *winner)
{
    if (G->s_cntdwn) 
    {
        G->rules.g_timer--;
        if (G->rules.g_timer > 0) 
        {
            if (G->rules.g_timer % 60 == 0) G->s_count--;
        }
        else 
        {
            G->rules.g_timer = 600;
            G->s_cntdwn = false;
            G->g_cntdwn = true;
        }
    }
    else if (G->g_cntdwn)
    {
        G->rules.g_timer--;
        if (G->rules.g_timer > 0) 
        {
            if (G->rules.g_timer % 60 == 0) 
            {
                G->g_c_timer--;
                if (G->g_count[1] > 48) G->g_count[1]--;
                else 
                {
                    G->g_count[0]--;
                    G->g_count[1] = 57;
                }
            }
        }
        else 
        {
            resetTimer(G);
            G->g_c_timer = 60;
            G->g_cntdwn = false;
            G->g_done = true;

            // decide the winner, or a draw
            *winner = decideWinner(*G);

            if (*winner > 0)
            {
                G->g_winner[6] = (*winner) + 48;
                G->g_message = G->g_winner;
            }
            else G->g_message = "    Draw!    ";
        }
    }
    else if (G->g_done)
    {
        G->rules.g_timer--;
        if (G->rules.g_timer > 0) 
        {
            if (G->rules.g_timer % 60 == 0) G->s_count--;
        }
        else 
        {
            G->rules.g_timer = 300;
            G->s_count = 53;
            G->g_done = false;
            bzero(G->g_board, 32*24);
            resetPlayers(G->players);
            resetScore(&G->rules);
        }
    }
}

void client_countdown(game *G, int *winner)
{
    if (G->g_cntdwn)
    {
        if (G->c_count_flag)
        {
            if (G->g_count[1] > 48) G->g_count[1]--;
            else 
            {
                G->g_count[0]--;
                G->g_count[1] = 57;
            }
            G->c_count_flag = false;
        }
        else 
        {
            if (G->g_done)
            {
                resetTimer(G);
                G->g_c_timer = 60;
                G->g_cntdwn = false;
                G->s_cntdwn = false;

                // decide the winner, or a draw
                *winner = decideWinner(*G);

                if (*winner > 0)
                {
                    G->g_winner[6] = (*winner) + 48;
                    G->g_message = G->g_winner;
                }
                else G->g_message = "    Draw!    ";
            }
        }
    }
}

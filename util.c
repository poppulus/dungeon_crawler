#include "util.h"

// sdl and rendering stuff
//
bool initSdl(SDL_Window **window, SDL_Renderer **renderer, const int W_WIDTH, const int W_HEIGHT)
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
        
        if (newTexture == NULL) printf("could not load optimised surface! %s\n", IMG_GetError());

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

// game stuff
//
void c_initClips(SDL_Rect *clips)
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

bool collision(SDL_Rect rect, int x, int y)
{

    return false;
}

void playerInput(SDL_Event e, game *g)
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: g->running = false; break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                    case SDLK_ESCAPE: g->running = false; break;
                    case SDLK_SPACE:
                        g->p->input[4] = 1;
                        if (!e.key.repeat && !g->p->attacking && !g->p->a_hold) 
                        {
                            g->p->aindex = 0;

                            g->p->a_hold = true;
                            enqueue(g->p->i_queue, ATTACK);
                            g->p->attacking = true;
                        }
                    break;
                    case SDLK_LSHIFT: 
                        if (!e.key.repeat) g->p->sprint = true; 
                    break;
                    case SDLK_UP:
                        if (!e.key.repeat) 
                        {enqueue(g->p->i_queue, UP); g->p->input[0] = 1;}
                    break;
                    case SDLK_DOWN:
                        if (!e.key.repeat) 
                        {enqueue(g->p->i_queue, DOWN); g->p->input[2] = 1;}
                    break;
                    case SDLK_LEFT:
                        if (!e.key.repeat) 
                        {enqueue(g->p->i_queue, LEFT); g->p->input[1] = 1;}
                    break;
                    case SDLK_RIGHT:
                        if (!e.key.repeat) 
                        {enqueue(g->p->i_queue, RIGHT); g->p->input[3] = 1;}
                    break;
                }
            break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
                    case SDLK_SPACE: 
                        g->p->input[4] = 0; 
                        g->p->a_hold = false; 
                    break;
                    case SDLK_LSHIFT: g->p->sprint = false; break;
                    case SDLK_UP:
                        dequeue(g->p->i_queue, UP); 
                        g->p->input[0] = 0;
                    break;
                    case SDLK_LEFT:
                        dequeue(g->p->i_queue, LEFT); 
                        g->p->input[1] = 0;
                    break;
                    case SDLK_DOWN:
                        dequeue(g->p->i_queue, DOWN); 
                        g->p->input[2] = 0;
                    break;
                    case SDLK_RIGHT:
                        dequeue(g->p->i_queue, RIGHT); 
                        g->p->input[3] = 0;
                    break;
                }
            break;
        }
    }
}

void updatePlayer(player *p)
{
    p->xvel = 0;
    p->yvel = 0;

    p->dir = p->i_queue[0];

    if ((p->i_queue[0] != 255) 
    && (p->i_queue[0] != ATTACK) && !p->attacking) 
        p->face = p->i_queue[0];

    if (!p->attacking)
    {
        switch (p->dir)
        {
            case UP: 
                p->yvel = p->sprint ? -2 : -1;
                p->a_hitBox.x = p->x + 7; 
                p->a_hitBox.y = p->y;
            break;
            case LEFT: 
                p->xvel = p->sprint ? -2 : -1;
                p->a_hitBox.x = p->x; 
                p->a_hitBox.y = p->y + 15;
            break;
            case DOWN: 
                p->yvel = p->sprint ? 2 : 1; 
                p->a_hitBox.x = p->x + 7; 
                p->a_hitBox.y = p->y + p->h;
            break;
            case RIGHT: 
                p->xvel = p->sprint ? 2 : 1; 
                p->a_hitBox.x = p->x + p->w; 
                p->a_hitBox.y = p->y + 15;
            break;
        }
    }

    animatePlayer(p);

    if (p->x + p->xvel <= 0 || p->x + p->xvel + p->w >= 640)
    {
        p->xvel = 0;
    }
    if (p->y + p->yvel <= 0 || p->y + p->yvel + p->h >= 480)
    {
        p->yvel = 0;
    }
    
    p->x += p->xvel;
    p->y += p->yvel;
}

void animatePlayer(player *p)
{
    if (p->attacking)
    {
        if (p->atk_counter % 6 == 0)
        {
            p->aindex++;
            p->aindex = (p->aindex % 3) + 4;
        }

        p->atk_counter++;

        if (p->atk_counter > 15)
        {
            p->atk_counter = 0;
            p->aindex = 0;
            dequeue(p->i_queue, ATTACK);
            p->attacking = false;
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
            if (p->acounter % 16 == 0)
            {
                p->aindex++;
                p->aindex %= 2;
            }
        }
    }
}

void renderPlayer(game G)
{
    bool flip = (G.p->face == 3 ? true : false);
    int c_index;

    if (G.p->face == 3) c_index = (2 * 7) + G.p->aindex;
    else c_index = (G.p->face * 7) + G.p->aindex;

    renderTexture(*G.renderer, G.c_texture, G.p->x - 9, G.p->y - 1, &G.p->clips[c_index], flip);

    if (G.p->attacking) 
    {
        SDL_SetRenderDrawColor(*G.renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderFillRect(*G.renderer, &G.p->a_hitBox);
    }
}

void enqueue(unsigned char *q, unsigned char val)
{
    for (int i = Q_SIZE - 1; i > 0; i--)
        q[i] = q[i - 1];
    
    q[0] = val;
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

    q[Q_SIZE - 1] = 255;
}

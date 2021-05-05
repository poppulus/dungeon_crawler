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
void initClips(SDL_Rect *clips)
{
    int n = 0;
    for (int i = 0; i < 12; i++)
    {
        clips[i].w = 16;
        clips[i].h = 16;
        clips[i].x = 48 + ((i % 3) * 16);
        clips[i].y = n * 16;

        if ((i % 3) == 2) n++;
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
                    case SDLK_ESCAPE: g->running = false; break;
                    case SDLK_UP:
                        if (!e.key.repeat) {enqueue(g->p->i_queue, UP); g->p->input[0] = 1;}
                        break;
                    case SDLK_DOWN:
                        if (!e.key.repeat) {enqueue(g->p->i_queue, DOWN); g->p->input[2] = 1;}
                        break;
                    case SDLK_LEFT:
                        if (!e.key.repeat) {enqueue(g->p->i_queue, LEFT); g->p->input[1] = 1;}
                        break;
                    case SDLK_RIGHT:
                        if (!e.key.repeat) {enqueue(g->p->i_queue, RIGHT); g->p->input[3] = 1;}
                        break;
                }
            break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
                    case SDLK_UP:
                        if (!e.key.repeat) {dequeue(g->p->i_queue, UP); g->p->input[0] = 0;}
                    break;
                    case SDLK_LEFT:
                        if (!e.key.repeat) {dequeue(g->p->i_queue, LEFT); g->p->input[1] = 0;}
                    break;
                    case SDLK_DOWN:
                        if (!e.key.repeat) {dequeue(g->p->i_queue, DOWN); g->p->input[2] = 0;}
                    break;
                    case SDLK_RIGHT:
                        if (!e.key.repeat) {dequeue(g->p->i_queue, RIGHT); g->p->input[3] = 0;}
                    break;
                }
            break;
        }
    }
}

void updatePlayer(player *p)
{
    /*
    printf("_ _ _ _\n");

    for (int i = 0; i < 4; i++) printf(" %d ", p->i_queue[i]);

    printf("\n_ _ _ _\n");
    */

    p->xvel = 0;
    p->yvel = 0;

    p->dir = p->i_queue[0];

    if (p->i_queue[0] != 255) p->face = p->i_queue[0];

    switch (p->dir)
    {
        case UP: p->yvel = -1; break;
        case LEFT: p->xvel = -1; break;
        case DOWN: p->yvel = 1; break;
        case RIGHT: p->xvel = 1; break;
    }

    if ((p->xvel != 0) || (p->yvel != 0)) 
    {
        p->moving = true;
        p->acounter++;

        if (p->acounter % 8 == 0)
        {
            p->aindex++;
            p->aindex %= 3;
        }
    }
    else 
    {
        p->moving = false;
        p->aindex = 1;
    }

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

void renderPlayer(game G, SDL_Rect *clips)
{
    renderTexture(*G.renderer, G.texture, G.p->x, G.p->y, &clips[c_anim_idx(G.p->face, G.p->aindex)], false);
}

void enqueue(unsigned char *q, unsigned char val)
{
    for (int i = 3; i > 0; i--)
        q[i] = q[i - 1];
    
    q[0] = val;
}

void dequeue(unsigned char *q, unsigned char val)
{
    bool found = false;

    for (int i = 0; i < 4; i++)
    {
        if (q[i] == val) 
            found = true;

        if (found && ((i + 1) < 4)) 
            q[i] = q[i + 1];
    }

    q[3] = 255;
}

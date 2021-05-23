#include "SDL_FontCache.h"
#include <stdbool.h>

typedef struct texture
{
    SDL_Texture *mTexture;
    short mWidth, mHeight;
} Texture; 

bool initSdl(SDL_Window **, SDL_Renderer **);
bool initTextureMap(SDL_Renderer **, Texture *, char *);
void renderTexture(SDL_Renderer *, Texture *, int x, int y, SDL_Rect *, const SDL_RendererFlip);
void freeTexture(Texture *);
SDL_Texture *loadTexture(SDL_Renderer *, char *path);

#include <string.h>
#include <stdlib.h>
#include "entity.h"
#include "graphics.h"

#define MaxSprites    255
#define MaxWindows    64

SDL_Surface *screen; /*pointer to the draw buffer*/
SDL_Surface *buffer; /*pointer to the background image buffer*/
TTF_Font *font;
SDL_Rect Camera; /*x & y are the coordinates for the background map, w and h are of the screen*/
SDL_Rect Abs_Camera; /* this just keeps track of how far in the current level we have traveled*/
SPRITE SpriteList[MaxSprites];
SPRITE WindowList[MaxWindows];
Entity *Mouse;
Uint32 NOW;
int NumSprites;
int NumWindows;
extern int NumLives;
extern Entity EntityList[MAXENTITIES];

/*some data on the video settings that can be useful for a lot of functions*/
Uint32 rmask,gmask,bmask,amask;
ScreenData  S_Data;


void Init_Graphics()
{
    Uint32 Vflags = SDL_FULLSCREEN | SDL_ANYFORMAT;
    Uint32 HWflag = 0;
    SDL_Surface *temp;
    S_Data.xres = 1024;
    S_Data.yres = 600;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
    #else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    #endif
    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_DOUBLEBUF) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
        if(SDL_VideoModeOK(1024, 600, 32, SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_HWSURFACE))
    {
        S_Data.xres = 1024;
        S_Data.yres = 600;
        S_Data.depth = 32;
        Vflags = SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_HWSURFACE;
        HWflag = SDL_HWSURFACE;
    }
    else if(SDL_VideoModeOK(1024, 600, 16, SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_HWSURFACE))
    {
        S_Data.xres = 1024;
        S_Data.yres = 600;
        S_Data.depth = 16;
        Vflags = SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_HWSURFACE;
        HWflag = SDL_HWSURFACE;
    }
    else if(SDL_VideoModeOK(1024, 600, 16, SDL_FULLSCREEN | SDL_ANYFORMAT))
    {
        S_Data.xres = 1024;
        S_Data.yres = 600;
        S_Data.depth = 16;
        Vflags = SDL_FULLSCREEN | SDL_ANYFORMAT;
        HWflag = SDL_SWSURFACE;
    }
    screen = SDL_SetVideoMode(S_Data.xres, S_Data.yres,S_Data.depth, Vflags);
    if ( screen == NULL )
    {
        fprintf(stderr, "Unable to set 1024x600 video: %s\n", SDL_GetError());
        exit(1);
    }
    temp = SDL_CreateRGBSurface(Vflags, 2048, 600, S_Data.depth,rmask, gmask,bmask,amask);
    if(temp == NULL)
	  {
        fprintf(stderr,"Couldn't initialize Video buffer: %s\n", SDL_GetError());
        exit(1);
	  }
    buffer = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    Camera.x = 0;
    Camera.y = 0;
    Camera.w = screen->w;
    Camera.h = screen->h;
    Abs_Camera.x = 0;
    Abs_Camera.y = 0;
    Abs_Camera.w = Camera.w;
    Abs_Camera.h = Camera.h;
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
    if(TTF_Init() == 0)
      atexit(TTF_Quit);
    else
    {
        fprintf(stderr,"Couldn't initialize True Type Font: %s\n", SDL_GetError());
        exit(1);
    }
    font = TTF_OpenFont("fonts/Trebuchet_MS_Italic.ttf",14);
    if(font == NULL)
    {
        fprintf(stderr,"Couldn't load font Trebuchet_MS_Italic: %s\n", SDL_GetError());
        exit(1);
    }
}


void ResetBuffer()
{
    SDL_BlitSurface(buffer,&Camera,screen,NULL);
}

/*
  makes sure a minimum number of ticks is waited between frames
  this is to ensure that on faster machines the game won't move so fast that
  it will look terrible.
  This is a very handy function in game programming.
*/

void FrameDelay(Uint32 delay)
{
    static Uint32 pass = 100;
    Uint32 dif;
    dif = SDL_GetTicks() - pass;
    if(dif < delay)SDL_Delay( delay - dif);
    pass = SDL_GetTicks();
}

void NextFrame()
{
  Uint32 Then;
  SDL_Flip(screen);							/*and then update the screen*/
  Then = NOW;									/*these next few lines  are used to show how long each frame takes to update.  */
  NOW = SDL_GetTicks();
  FrameDelay(15); /*this will make your frame rate about 30 frames per second.  If you want 60 fps then set it to about 15 or 16*/
}

/*
  InitSpriteList is called when the program is first started.
  It just sets everything to zero and sets all pointers to NULL.
  It should never be called again.
*/

void InitSpriteList()
{
  int x;
  NumSprites = 0;
  NumWindows = 0;
  memset(SpriteList,0,sizeof(SPRITE) * MaxSprites);
  for(x = 0;x < MaxSprites;x++)SpriteList[x].image = NULL;
  memset(WindowList,0,sizeof(SPRITE) * MaxWindows);
  for(x = 0;x < MaxWindows;x++)WindowList[x].image = NULL;
}

SPRITE *NewWindow(int sx, int sy,int sw,int sh,int bgcolor)
{
  int i;
  SDL_Surface *temp;
  /*first check to see if we have room for a new window*/
  if(NumWindows + 1 >= MaxWindows)
  {
        fprintf(stderr, "Maximum Windows Reached.\n");
        exit(1);
  } 
  NumWindows++;
  for(i = 0;i <= NumWindows;i++)
  {
    if(!WindowList[i].used)break;
  }
  temp = SDL_CreateRGBSurface(0, sw, sh, S_Data.depth,rmask, gmask,bmask,amask);
  WindowList[i].image = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  WindowList[i].w = sw;
  WindowList[i].h = sh;
  WindowList[i].color2 = sx;
  WindowList[i].color3 = sy;
   /*I know its odd to use the color fields for position, but normal sprites
    don't have the x and y and this way I save memory*/
  WindowList[i].color1 = bgcolor;
  WindowList[i].used = 1;
  PaintWindow(WindowList[i].image,0,0,sw - 1, sh - 1,bgcolor);
  return &WindowList[i];
}

void CloseWindow()
{
  int i;
  for(i = 0;i < MaxWindows;i++)
  {
     FreeSprite(&WindowList[i]);
     /*since I am treating windows as just another sprite,
      it makes sense to get rid of them like any other sprite
      */
  }
}

void DrawWindow(SPRITE *sprite,SDL_Surface *surface)
{
    SDL_Rect src,dest;
    src.x = 0;
    src.y = 0;
    src.w = sprite->w;
    src.h = sprite->h;
    dest.x = sprite->color2;
    dest.y = sprite->color3;
    dest.w = sprite->w;
    dest.h = sprite->h;
   /*textbook (literally) use of blitsurface*/
    SDL_BlitSurface(sprite->image, &src, surface, &dest);
}

SPRITE *DisplayBar()
{
  SPRITE *win;
  SDL_Surface *txt;
  SDL_Surface *tmp;
  SDL_Color c1,c2;
  SDL_Rect dest;
  win = NewWindow(0, screen->h - 32,screen->w,32,SDL_MapRGB(screen->format, 0,0,0));
  c1.r = 240;
  c1.g = 240;
  c1.b = 250;
  c2.r = 1;
  c2.g = 1;
  c2.b = 1;
  dest.x = (screen->w/2) - 46;
  dest.y = 8;
  tmp = TTF_RenderText_Shaded(font,"Attacker",c1,c2);
  txt = SDL_DisplayFormat(tmp);
  SDL_SetColorKey(txt, SDL_SRCCOLORKEY , SDL_MapRGB(txt->format, 0,0,0));
  SDL_FreeSurface(tmp);
  SDL_BlitSurface(txt, NULL, win->image, &dest);
  return win;
}

void UpdateDisplayBar(Entity *player,SPRITE *window)
{
  int i;
  SDL_Surface *temp;
  SDL_Surface *draw;
  SDL_Rect healthbar;
  healthbar.x = 130;
  healthbar.y = 8;
  healthbar.w = 100;
  healthbar.h = 16;
  temp = SDL_CreateRGBSurface(SDL_ANYFORMAT, window->w, window->h, S_Data.depth,rmask, gmask,bmask,amask);
  if(temp == NULL)
  {
        fprintf(stderr,"Couldn't create SDL Surface: %s\n", SDL_GetError());
        exit(1);
  }
  draw = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_BlitSurface(window->image, NULL, draw,NULL);
  for(i = 0;i < NumLives;i++)
    DrawSprite(player->sprite,draw,5 + (i * player->sprite->w) ,1,0);
  for(i = 0;i < player->ammo;i++)
    DrawSprite(player->sprite,draw,screen->w - (i * 16) - 24 ,1,15);
  SDL_FillRect(draw,&healthbar,SDL_MapRGB(screen->format,255,0,0));
  healthbar.w = (player->health * 100) / player->healthmax;
  SDL_FillRect(draw,&healthbar,SDL_MapRGB(screen->format,0,255,0));
  healthbar.w = player->heat;
  healthbar.y += 12;
  healthbar.h = 4;
  SDL_FillRect(draw,&healthbar,SDL_MapRGB(screen->format,155,155,0));
  temp = window->image;
  window->image = draw;
  DrawWindow(window,screen);
  window->image = temp;
  SDL_FreeSurface(draw);
}

/*Create a sprite from a file, the most common use for it.*/

SPRITE *LoadSprite(char *filename,int sizex, int sizey, int c1, int c2, int c3)
{
  int i;
  SDL_Surface *temp;
  /*first search to see if the requested sprite image is alreday loaded*/
  for(i = 0; i < NumSprites; i++)
  {
    if((strncmp(filename,SpriteList[i].filename,20)==0)&&(SpriteList[i].used >= 1)&&(c1 == SpriteList[i].color1)&&(c2 == SpriteList[i].color2)&&(c3 == SpriteList[i].color3))
    {
      SpriteList[i].used++;
      return &SpriteList[i];
    }
  }
  /*makesure we have the room for a new sprite*/
  if(NumSprites + 1 >= MaxSprites)
  {
        fprintf(stderr, "Maximum Sprites Reached.\n");
        exit(1);
  }
  /*if its not already in memory, then load it.*/
  NumSprites++;
  for(i = 0;i <= NumSprites;i++)
  {
    if(!SpriteList[i].used)break;
  }
  temp = IMG_Load(filename);
  if(temp == NULL)
  {
        fprintf(stderr, "FAILED TO LOAD A VITAL SPRITE.\n");
        exit(1);
  }
  SpriteList[i].image = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  /*sets a transparent color for blitting.*/
  SDL_SetColorKey(SpriteList[i].image, SDL_SRCCOLORKEY , SDL_MapRGB(SpriteList[i].image->format, 255,255,255));
  SwapSprite(SpriteList[i].image,c1,c2,c3);
   /*then copy the given information to the sprite*/
  strcpy(SpriteList[i].filename,filename);
      /*now sprites don't have to be 16 frames per line, but most will be.*/
  SpriteList[i].framesperline = 16;
  SpriteList[i].w = sizex;
  SpriteList[i].h = sizey;
  SpriteList[i].color1 = c1;
  SpriteList[i].color2 = c2;
  SpriteList[i].color3 = c3;
  SpriteList[i].used = 1;
  return &SpriteList[i];
}

/*
 * When we are done with a sprite, lets give the resources back to the system...
 * so we can get them again later.
 */

void FreeSprite(SPRITE *sprite)
{
  int i;
  /*first lets check to see if the sprite is still being used.*/
  if(!sprite->used)return;  //already deleted
  for(i = 0;i < MAXENTITIES;i++)
  {
    if((EntityList[i].used)&&(sprite == EntityList[i].sprite))
    return; /*yup, we found another entity is using this sprite.*/
  }
  sprite->used--;//still in use
  if(sprite->used > 0)return;
  strcpy(sprite->filename,"\0");
     /*just to be anal retentive, check to see if the image is already freed*/
  if(sprite->image != NULL)SDL_FreeSurface(sprite->image);
  sprite->image = NULL;
  NumSprites--;
   /*and then lets make sure we don't leave any potential seg faults 
  lying around*/
}

void CloseSprites()
{
  int i;
   for(i = 0;i < MaxSprites;i++)
   {
     /*it shouldn't matter if the sprite is already freed, 
     FreeSprite checks for that*/
      FreeSprite(&SpriteList[i]);
   }
   for(i = 0;i < MaxWindows;i++)
   {
     FreeSprite(&WindowList[i]);
     /*and since no enitity will use a window, this will work just fine for windows too*/
   }
}

void DrawSprite(SPRITE *sprite,SDL_Surface *surface,int sx,int sy, int frame)
{
    SDL_Rect src,dest;
    src.x = frame%sprite->framesperline * sprite->w;
    src.y = frame/sprite->framesperline * sprite->h;
    src.w = sprite->w;
    src.h = sprite->h;
    dest.x = sx;
    dest.y = sy;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite->image, &src, surface, &dest);
  
}
/*
 * For some lazy blending, I use part of one sprite to blend over another.
 * It looks.... ok.  But nothing fancy.
 */
void DrawSpritePart(SPRITE *sprite,SDL_Surface *surface,int sx,int sy, int sw, int sh, int frame)
{
    SDL_Rect src,dest;
    src.x = frame%16 * sprite->w;
    src.y = frame/16 * sprite->h;
    src.w = sw;
    src.h = sh;
    dest.x = sx;
    dest.y = sy;
    dest.w = sw;
    dest.h = sh;
    SDL_BlitSurface(sprite->image, &src, surface, &dest);

}

/*
  Copied from SDL's website.  I use it for palette swapping
  Its not plagerism if you document it!
*/

void DrawPixel(SDL_Surface *screen, Uint8 R, Uint8 G, Uint8 B, int x, int y)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) )
    {
        if ( SDL_LockSurface(screen) < 0 )
        {
            return;
        }
    }
    switch (screen->format->BytesPerPixel)
    {
        case 1:
        { /* Assuming 8-bpp */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2:
        { /* Probably 15-bpp or 16-bpp */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3:
        { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *(bufp+screen->format->Rshift/8) = R;
            *(bufp+screen->format->Gshift/8) = G;
            *(bufp+screen->format->Bshift/8) = B;
        }
        break;

        case 4:
        { /* Probably 32-bpp */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
    if ( SDL_MUSTLOCK(screen) )
    {
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, x, y, 1, 1);
}

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    /* Here p is the address to the pixel we want to retrieve*/
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

    switch(surface->format->BytesPerPixel)
    {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /*shouldn't happen, but avoids warnings*/
    }
}



/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

    switch(surface->format->BytesPerPixel)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


/*
  copied and pasted and then significantly modified from the sdl website.  
  I kept ShowBMP to test my program as I wrote it.
*/

void ShowBMP(SDL_Surface *image, SDL_Surface *screen, int x, int y)
{
    SDL_Rect dest;

    /* Blit onto the screen surface.
       The surfaces should not be locked at this point.
     */
    dest.x = x;
    dest.y = y;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);

    /* Update the changed portion of the screen */
    SDL_UpdateRects(screen, 1, &dest);
}

/*
  At the time I wrote these draw functions waay back in '01 I had forgotten about
  y=mx+b.  Now its quite embarrising, but hey...  For special cases, they can
  be more efficient
*/

void VertLine(SDL_Surface *buffer,int sx,int sy,int h,Uint32 color)
{
    int y;
    SDL_LockSurface(buffer);
    for(y = 0;y < h;y++)
    {
        putpixel(buffer,sx,sy + y,color);
    }
    SDL_UnlockSurface(buffer);
}

void HorzLine(SDL_Surface *buffer,int sx,int sy,int w,Uint32 color)
{
    int x;

    SDL_LockSurface(buffer);
    for(x = 0;x < w;x++)
    {
        putpixel(buffer,sx + x,sy,color);
    }
    SDL_UnlockSurface(buffer);
}

void DrawLine(SDL_Surface *buffer,int x1,int y1,int x2,int y2,Uint32 color)
{
    if(x1 == x2)
    {
        if(y2 > y1)
        {
            VertLine(buffer,x1,y1,y2-y1,color);
            return;
        }
        VertLine(buffer,x1,y2,y1-y2,color);
        return;
    }
    if(y1 == y2)
    {
        if(x2 > x1)
        {
            HorzLine(buffer,x1,y1,x2-x1,color);
            return;
        }
        HorzLine(buffer,x2,y2,x1-x2,color);
        return;
    }
}
/*
 * When I first started programming graphics, back in high school, the library
 * I was using then had a button class.  So to get around reusing their
 * identifiers, I started to use booton.  It kinda grew on me.
 */
void booton(SDL_Surface *buffer,int x, int y, int width, int height)
{
    HorzLine(buffer,x,y,width,0xFFFFFF);
    VertLine(buffer,x+width,y,height,0xFFFFFF);
    VertLine(buffer,x,y,height,0x000000);
    HorzLine(buffer,x,y+height,width,0x000000);
}
/*
 * A quick reversal of the above.  By switching between them buttons can
 * appear to be pressed.
 */
void ubooton(SDL_Surface *buffer,int x, int y, int width, int height)
{
    HorzLine(buffer,x,y,width,0x000000);
    VertLine(buffer,x+width,y,height,0x000000);
    VertLine(buffer,x,y,height,0xFFFFFF);
    HorzLine(buffer,x,y+height,width,0xFFFFFF);
}

/*
  This will create a simple single color window on the surface 'buffer'.
  it will have a very simple old school look to it.
*/

void PaintWindow(SDL_Surface *buffer, int x,int y,int w, int h,Uint32 color)
{
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    SDL_FillRect(buffer, &r, color);
    booton(buffer,x,y,w,h);
}

/*sets an sdl surface to all color.*/

void BlankScreen(SDL_Surface *buf,Uint32 color)
{
    SDL_LockSurface(buf);
    memset(buf->pixels, (Uint8)color,buf->format->BytesPerPixel * buf->w *buf->h);
    SDL_UnlockSurface(buf);
}
/*
 * This is the beginning of my Palette swapping scheme.  It checks the value
 * of the color it is given to see if the given color is PURE red, PURE green,
 * or PURE blue.  If it is, it takes the value as a percentage to apply to
 * the new color.  It returns either the old color untouched (if it wasn't a 
 * special case) or the new color.
 */

Uint32 SetColor(Uint32 color, int newcolor1,int newcolor2, int newcolor3)
{
    Uint8 r,g,b;
    Uint8 intensity;
    int newcolor;
    SDL_GetRGB(color, screen->format, &r, &g, &b);
    if((r == 0) && (g == 0)&&(b !=0))
    {
        intensity = b;
        newcolor = newcolor3;
    }
    else if((r ==0)&&(b == 0)&&(g != 0))
    {
        intensity = g;
        newcolor = newcolor2;
    }
    else if((g == 0)&&(b == 0)&&(r != 0))
    {
        intensity = r;
        newcolor = newcolor1;
    }
    else return color;
    switch(newcolor)
    {
        case Red:
            r = intensity;
            g = 0;
            b = 0;
            break;
        case Green:
            r = 0;
            g = intensity;
            b = 0;
            break;
        case Blue:
            r = 0;
            g = 0;
            b = intensity;
            break;
        case Yellow:
            r = (Uint8)intensity * 0.7;
            g = (Uint8)intensity * 0.7;
            b = 0;
            break;
        case Orange:
            r = (Uint8)intensity * 0.9;
            g = (Uint8)intensity * 0.4;
            b = (Uint8)intensity * 0.1;
            break;
        case Violet:
            r = (Uint8)intensity * 0.7;
            g = 0;
            b = (Uint8)intensity * 0.7;
            break;
        case Brown:
            r = (Uint8)intensity * 0.6;
            g = (Uint8)intensity * 0.3;
            b = (Uint8)intensity * 0.15;
            break;
        case Grey:
            r = (Uint8)intensity * 0.5;
            g = (Uint8)intensity * 0.5;
            b = (Uint8)intensity * 0.5;
            break;
        case DarkRed:
            r = (Uint8)intensity * 0.5;
            g = 0;
            b = 0;
            break;
        case DarkGreen:
            r = 0;
            g = (Uint8)intensity * 0.5;
            b = 0;
            break;
        case DarkBlue:
            r = 0;
            g = 0;
            b = (Uint8)intensity * 0.5;
            break;
        case DarkYellow:
            r = (Uint8)intensity * 0.4;
            g = (Uint8)intensity * 0.4;
            b = 0;
            break;
        case DarkOrange:
            r = (Uint8)intensity * 0.6;
            g = (Uint8)intensity * 0.2;
            b = (Uint8)intensity * 0.1;
            break;
        case DarkViolet:
            r = (Uint8)intensity * 0.4;
            g = 0;
            b = (Uint8)intensity * 0.4;
            break;
        case DarkBrown:
            r = (Uint8)intensity * 0.2;
            g = (Uint8)intensity * 0.1;
            b = (Uint8)intensity * 0.05;
            break;
        case DarkGrey:
            r = (Uint8)intensity * 0.3;
            g = (Uint8)intensity * 0.3;
            b = (Uint8)intensity * 0.3;
            break;
        case LightRed:
            r = intensity;
            g = (Uint8)intensity * 0.45;
            b = (Uint8)intensity * 0.45;
            break;
        case LightGreen:
            r = (Uint8)intensity * 0.45;
            g = intensity;
            b = (Uint8)intensity * 0.45;
            break;
        case LightBlue:
            r = (Uint8)intensity * 0.45;
            b = intensity;
            g = (Uint8)intensity * 0.45;
            break;
        case LightYellow:
            r = intensity;
            g = intensity;
            b = (Uint8)intensity * 0.45;
            break;
        case LightOrange:
            r = intensity;
            g = (Uint8)intensity * 0.75;
            b = (Uint8)intensity * 0.35;
            break;
        case LightViolet:
            r = intensity;
            g = (Uint8)intensity * 0.45;
            b = intensity;
            break;
        case LightBrown:
            r = intensity;
            g = (Uint8)intensity * 0.85;
            b = (Uint8)intensity * 0.45;
            break;
        case LightGrey:
            r = (Uint8)intensity * 0.85;
            g = (Uint8)intensity * 0.85;
            b = (Uint8)intensity * 0.85;
            break;
        case Black:
            r = (Uint8)intensity * 0.15;
            g = (Uint8)intensity * 0.15;
            b = (Uint8)intensity * 0.15;
            break;
        case White:
            r = intensity;
            g = intensity;
            b = intensity;
            break;
        case Tan:
            r = intensity;
            g = (Uint8)intensity * 0.9;
            b = (Uint8)intensity * 0.6;
            break;
        case Gold:
            r = (Uint8)intensity * 0.8;
            g = (Uint8)intensity * 0.7;
            b = (Uint8)intensity * 0.2;
            break;
        case Silver:
            r = (Uint8)intensity * 0.95;
            g = (Uint8)intensity * 0.95;
            b = intensity;
            break;
        case YellowGreen:
            r = (Uint8)intensity * 0.45;
            g = (Uint8)intensity * 0.75;
            b = (Uint8)intensity * 0.2;
            break;
        case Cyan:
            r = 0;
            g = (Uint8)intensity * 0.85;
            b = (Uint8)intensity * 0.85;
            break;
        case Magenta:
            r = (Uint8)intensity * 0.7;
            g = 0;
            b = (Uint8)intensity * 0.7;
            break;
    }
    return SDL_MapRGB(screen->format,r,g,b);
}

/* This will probably never have to be called, returns the hex code for the
 * enumerated color
 */

Uint32 IndexColor(int color)
{
    switch(color)
    {
    case Red:
        return Red_;
    case Green:
        return Green_;
    case Blue:
        return Blue_;
    case Yellow:
        return Yellow_;
    case Orange:
        return Orange_;
    case Violet:
        return Violet_;
    case Brown:
        return Brown_;
    case Grey:
        return Grey_;
    case DarkRed:
        return DarkRed_;
    case DarkGreen:
        return DarkGreen_;
    case DarkBlue:
        return DarkBlue_;
    case DarkYellow:
        return DarkYellow_;
    case DarkOrange:
        return DarkOrange_;
    case DarkViolet:
        return DarkViolet_;
    case DarkBrown:
        return DarkBrown_;
    case DarkGrey:
        return DarkGrey_;
    case LightRed:
        return LightRed_;
    case LightGreen:
        return LightGreen_;
    case LightBlue:
        return LightBlue_;
    case LightYellow:
        return LightYellow_;
    case LightOrange:
        return LightOrange_;
    case LightViolet:
        return LightViolet_;
    case LightBrown:
        return LightBrown_;
    case LightGrey:
        return LightGrey_;
    case Black:
        return Black_;
    case White:
        return White_;
    case Tan:
        return Tan_;
    case Gold:
        return Gold_;
    case Silver:
        return Silver_;
    case YellowGreen:
        return YellowGreen_;
    case Cyan:
        return Cyan_;
    case Magenta:
        return Magenta_;
    }
    return Black_;
}
/*
 * and now bringing it all together, we swap the pure colors in the sprite out
 * and put the new colors in.  This maintains any of the artist's shading and
 * detail, but still lets us have that old school palette swapping.  
 */
void SwapSprite(SDL_Surface *sprite,int color1,int color2,int color3)
{
    int x, y;
    Uint32 pixel;
   /*First the precautions, that are tedious, but necessary*/
    if(color1 == -1)return;
    if(sprite == NULL)return;
    if ( SDL_LockSurface(sprite) < 0 )
    {
        fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
        exit(1);
    }
   /*now step through our sprite, pixel by pixel*/
    for(y = 0;y < sprite->h ;y++)
    {
        for(x = 0;x < sprite->w ;x++)
        {                           
             pixel = getpixel(sprite,x,y);/*and swap it*/
             putpixel(sprite,x,y,SetColor(pixel,color1,color2,color3));
        }
    }
    SDL_UnlockSurface(sprite);
}

/*mouse handling functions*/

void InitMouse()
{
  Mouse = NewEntity();
  Mouse->sprite = LoadSprite("images/mouse.png",16, 16, White, Brown, Cyan);
  Mouse->framestates[0] = 0;
  Mouse->framestates[1] = 16;
  Mouse->framestates[2] = 32;
  Mouse->framestates[3] = 48;
  Mouse->state = 0;
  Mouse->shown = 0;
  Mouse->frame = 0;
}

    /*draws to the screen immediately before the blit, after all
     it wouldn't be a very good mouse if it got covered up by the
     game content*/
void DrawMouse()
{
  int mx,my;
  SDL_GetMouseState(&mx,&my);
  DrawSprite(Mouse->sprite,screen,mx,my,Mouse->frame);
  Mouse->count = GetNextCount(Mouse);
  Mouse->frame = Mouse->count + Mouse->framestates[Mouse->state];
}


#include <stdlib.h>
#include "SDL.h"
#include "graphics.h"
#include "entity.h"
#include "space.h"
#include "audio.h"

extern SDL_Surface *screen;
extern SDL_Surface *buffer; /*pointer to the draw buffer*/
extern SDL_Rect Camera;
extern Entity *Mouse;

void Init_All();

int main(int argc, char *argv[])
{
  SPRITE *window;
  int done;
  int keyn;
  Uint8 *keys;
  Entity *player;
  Level *testlevel;
  Init_All();
  testlevel = GetCurrentLevel(0);
  RenderLevel(testlevel);
  LoadSprite("images/effects.png",16,16,-1,-1,-1);
  window = DisplayBar();
  player = MakeShip();
  done = 0;
  do
  {
    if(!MoveCamera(2))
    {
      player->vx = 2;
    }
    UpdateEntities();
    ResetBuffer();//    DrawMouse();
    DrawEntities();
    UpdateDisplayBar(player,window);
    NextFrame();
    SDL_PumpEvents();
    keys = SDL_GetKeyState(&keyn);
    if(keys[SDLK_ESCAPE])done = 1;
  }while(!done);
    
  exit(0);
  return 0;
}

void CleanUpAll()
{
  CloseSprites();
  ClearEntities();
  /*any other cleanup functions can be added here
  fafafasff
  sfdafaf
  asfasf
  sf*/ 
}

void Init_All()
{
  Init_Graphics();
  Init_Audio();
  InitSoundList();
  InitSpriteList();
  InitEntityList();
  InitMouse();
  LoadLevelSprites();
  atexit(CleanUpAll);
}


#ifndef _ENTITY_
#define _ENTITY_

#include "graphics.h"
#include "audio.h"

/*
  This file contains the data structures and function prototypes for handling entities.
  Entities all contain information about the associate sprite that may be unique to itself
  as well as the animation information, if it has any.
*/

#define MAXENTITIES   511
#define MAXSTATES     20
#define SOUNDSPERENT  4

enum STATES {ST_IDLE,ST_FIRE1,ST_FIRE2,ST_FIRE3,ST_DYING,ST_DEAD};

typedef struct ENTITY_T
{
  SPRITE *sprite;
  Sound *sound[SOUNDSPERENT];
  struct ENTITY_T *owner;
  void (*think) (struct ENTITY_T *self);
  /*the think function is the function called each frame to update the state of the entity*/
  int shown; /*if 1 then it will be rendered when it is on screen*/
  int used; /*used is for keeping track of what entities are free and should NEVER be touched.*/
  int frame;
  int count;  /*used for animation*/
  int state;  /*making each entity a finite state machine.*/
  int sx,sy; /*sprite's coordinates*/
  int animframe;  /*specifies how many global frames go by before the sprite frame is animated.*/
  int busy;   /*used for some think functions when issuing a command, if its busy with an animation it won't bother the sprite*/
  int delay;
  int vx,vy;  /*vector values*/
  /*an entity can have up to MAXSTATES states.  Each state shows the fram it Ends on, the previous state marks
    the begining.*/
  int framestates[MAXSTATES];
  /*the absolute screen coordinates and the map coordinates*/
  SDL_Rect bbox;  //The bounding box for hit detection, x and y are offsets from the position.
  int weaplevel;
  int health;
  int healthmax;
  int ammo;
  int heat;
  int enemy;
}Entity;


void InitEntityList();
Entity *NewEntity();
void FreeEntity(Entity *ent);
void ClearEntities();
void DrawEntities();
void DrawEntity(Entity *ent);
void UpdateEntities();
void UpdateDisplayBar(Entity *player,SPRITE *window);

int GetNextCount(Entity *self);

Entity *MakeShip();
void PlayerThink(Entity *self);
Entity *FireMissile(Entity *Owner,int sx,int sy,int vx,int vy,int damage,int enemy);
void MissileThink(Entity *self);
Entity *SpawnBlast(Entity *Owner,int sx,int sy,int vx,int vy,int damage,int type,int enemy);
void BlueBlastThink(Entity *self);
void RedBlastThink(Entity *self);
void PurpleBlastThink(Entity *self);
Entity *SpawnBug(int sx,int sy,int type);
void BugThink(Entity *self);
void AttackBugThink(Entity *self);
Entity *DropMissile(int sx,int sy,int vx,int vy);
void MissilePickup(Entity *self);
Entity *DropPowerUp(int sx,int sy,int vx,int vy,int level);
void PowerPickup(Entity *self);
Entity *SpawnMantid(int sx,int sy,int type);
void MantThink(Entity *self);
void AttackMantThink(Entity *self);

#endif

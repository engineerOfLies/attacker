/*
 *    Donald Kehoe
 *    Sometime in Fenruary
 *    Last Modified: 3/3/05
 *
 *    Description: definitions for Entity handling functions (methods).
 *      Definitions for specific instances of Entities for this simple game.
*/
#include <stdlib.h>
#include <math.h>
#include "entity.h"
#include "space.h"
/*
  Entity function definitions
*/

enum FACTION {E_NONE,E_Player,E_Bugs};
extern SDL_Surface *screen;
extern SDL_Event Event; //This is just so the think functions in this file can read what has been input.
extern SDL_Rect Camera;
extern int NumLevels;
extern int CurrentLevel;
Entity EntityList[MAXENTITIES];
Entity *Player;
int NumLives = 3;
int NumEnts;
int MOUSEMOVE = 1;
int lastx,lasty; /*last mouse coordinates*/

enum BlasterTypes {BL_Red,BL_Blue,BL_Purple,BL_Blue2,BL_Blue3,BL_Teal};

void DrawEntities()
{
  int i;
  for(i = 0; i < MAXENTITIES;i++)
  {
    if(EntityList[i].used)
    {
      if(EntityList[i].shown)
        DrawEntity(&EntityList[i]);
    }
  }
}

void UpdateEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(EntityList[i].used)
    {
      if(EntityList[i].think != NULL)
      {
        EntityList[i].think(&EntityList[i]);
      }
    }
  }
}

void DrawEntity(Entity *ent)
{
  DrawSprite(ent->sprite,screen,ent->sx,ent->sy,ent->frame);
}

void InitEntityList()
{
  int i,j;
  NumEnts = 0;
  for(i = 0;i < MAXENTITIES; i++)
  {
    EntityList[i].sprite = NULL;
    EntityList[i].owner = NULL;
    EntityList[i].think = NULL;
    for(j = 0;j < SOUNDSPERENT;j++)
    {
      EntityList[i].sound[j] = NULL;
    }
    EntityList[i].shown = 0;
    EntityList[i].used = 0;
  }
}

/*
  returns NULL if all filled up, or a pointer to a newly designated Entity.
  Its up to the other function to define the data.
*/
Entity *NewEntity()
{
  int i;
  if(NumEnts + 1 >= MAXENTITIES)
  {
    return NULL;
  }
  NumEnts++;
  for(i = 0;i <= NumEnts;i++)
  {
    if(!EntityList[i].used)break;
  }
  EntityList[i].used = 1;
  return &EntityList[i];
}

/*done with an entity, now give back its water..I mean resources*/
void FreeEntity(Entity *ent)
{
  int j;
  ent->used = 0;
  NumEnts--;
  if(ent->sprite != NULL)FreeSprite(ent->sprite);
  for(j = 0;j < SOUNDSPERENT;j++)
  {
    if(ent->sound[j] != NULL)FreeSound(ent->sound[j]);
    ent->sound[j] = NULL;
  }
  ent->sprite = NULL;
  ent->owner = NULL;
}

/*kill them all*/
void ClearEntities()
{
  int i = 0;
  for(i = 0;i < MAXENTITIES;i++)
  {
    FreeEntity(&EntityList[i]);
  }
}

/*
 * This handly little function will figure pace out a looping animation for an entity.
 * It could probably have been a macro...oh well.
 */
int GetNextCount(Entity *self)
{
  return (self->count+1)%(self->framestates[self->state + 1] - self->framestates[self->state]);
}

void ScaleVectors(float *vx, float *vy)
{
  float hyp;
  hyp = sqrt((*vx * *vx) + (*vy * *vy));
  hyp = 1 / hyp;
  *vx = (*vx * hyp);
  *vy = (*vy * hyp);
}

int Collide(SDL_Rect box1,SDL_Rect box2)
{
  /*check to see if box 1 and box 2 clip, then check to see if box1 is in box or vice versa*/
  if((box1.x + box1.w >= box2.x) && (box1.x <= box2.x+box2.w) && (box1.y + box1.h >= box2.y) && (box1.y <= box2.y+box2.h))
    return 1;
  return 0;
}

Entity *HitNextEnt(Entity *self,Entity *target)
{
  int i;
  SDL_Rect b1,b2;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  for(i = 0;i < MAXENTITIES; i++)
  {
    if(target == &EntityList[i])break;
  }
  for(i = i;i < MAXENTITIES; i++)
  {
    if(EntityList[i].used)
    {
      if((EntityList[i].health > 0)&&(EntityList[i].enemy != E_NONE)&&(EntityList[i].enemy != self->enemy))
      {
        b2.x = EntityList[i].sx + EntityList[i].bbox.x;
        b2.y = EntityList[i].sy + EntityList[i].bbox.y;
        b2.w = EntityList[i].bbox.w;
        b2.h = EntityList[i].bbox.h;
        if(Collide(b1,b2))
         return &EntityList[i];
      }
    }
  }
  return NULL;
}

Entity *GetClosestTarget(Entity *self)
{
  int i;
  int range = 2000;
  int Trange = 2000;
  Entity *target = NULL;
  for(i = 0;i < MAXENTITIES; i++)
  {
    if(EntityList[i].used)
    {
      if((EntityList[i].health > 0)&&(EntityList[i].enemy != E_NONE)&&(EntityList[i].enemy != self->enemy))
      {
        Trange = sqrt((EntityList[i].sx - self->sx)*(EntityList[i].sx - self->sx) + (EntityList[i].sy - self->sy)*(EntityList[i].sy - self->sy));
        if(Trange < range)
        {
          target = &EntityList[i];
          range = Trange;
        }
      }
    }
  }
  return target;
}


Entity *HitEnt(Entity *self)
{
  int i;
  SDL_Rect b1,b2;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  for(i = 0;i < MAXENTITIES; i++)
  {
    if(EntityList[i].used)
    {
      if((EntityList[i].health > 0)&&(EntityList[i].enemy != E_NONE)&&(EntityList[i].enemy != self->enemy))
      {
        b2.x = EntityList[i].sx + EntityList[i].bbox.x;
        b2.y = EntityList[i].sy + EntityList[i].bbox.y;
        b2.w = EntityList[i].bbox.w;
        b2.h = EntityList[i].bbox.h;
        if(Collide(b1,b2))
         return &EntityList[i];
      }
    }
  }
  return NULL;
}


/*
  This is the list of specific instances of entities.
*/
/*
 *  Spawns a mantid attack bug
 */

Entity *SpawnMantid(int sx,int sy,int type)
{
  Entity *bug;
  bug = NewEntity();
  if(bug == NULL)return bug;
  bug->sprite = LoadSprite("images/mantid.png",48,32,-1,-1,-1);
  bug->sound[0] = LoadSound("sounds/bugdeath.wav",MIX_MAX_VOLUME/2);
  bug->sound[1] = LoadSound("sounds/burst.wav",MIX_MAX_VOLUME/2);
  bug->sound[2] = LoadSound("sounds/medatck1.wav",MIX_MAX_VOLUME/2);
  bug->shown = 1;
  bug->sx = sx;
  bug->sy = sy;
  bug->vx = -1;
  bug->vy = 0;
  if(!type)bug->think = MantThink;
  else bug->think = AttackMantThink;
  bug->bbox.x = 4;
  bug->bbox.y = 4;
  bug->bbox.w = 40;
  bug->bbox.h = 20;
  bug->frame = 0;
  bug->state = ST_IDLE;
  bug->ammo = 2;
  bug->health = 20;
  bug->owner = NULL;
  bug->enemy = E_Player;
  return bug;
}

void AttackMantThink(Entity *self)
{
  SDL_Rect b1,b2;
  float tx,ty;  /*target vectors for the player*/
  self->sx += self->vx;
  self->sy += self->vy;
  if(self->vx > -2)self->vx--;
  if(self->sy < 0)
  {
    self->sy = 0;
    self->vy += 15;
    self->delay += rand() % 20;
  }
  if(self->sy > screen->h - 64)
  {
    self->sy = screen->h - 64;
    self->vy -= 15;
    self->delay += rand() % 10;
  }
  /*check for colision with player*/
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  if((self->health <= 0)&&(self->state != ST_DYING))
  {
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[0]->sound,0);
  }
  else if((Player->health > 0)&&(self->state != ST_DYING)&&(Collide(b1,b2)))
  {
    Player->health -= 8;
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[1]->sound,0);
  }
  /*Check to see if I should die*/
  if(self->sx < - 32)
  {
    self->shown = 0;
    FreeEntity(self);
  }
  switch(self->state)
  {
    case ST_DYING:
      if(self->frame >= 10)
      {
        if((rand() % 8) == 0)DropMissile(self->sx,self->sy,-4,0);
        FreeEntity(self);
      }
      self->frame++;
    break;
    case ST_FIRE1:
      if(self->frame >= 6)
      {
        /*spawn purple bullet*/
        tx = self->sx - Player->sx;
        ty = self->sy - Player->sy;
        if(ty != 0)ty = 10 * ty/tx;
        if(Player->sy >= self->sy)
        {
          ty = fabs(ty);
        }
        else ty = fabs(ty) * -1;
        tx = -6;
        SpawnBlast(self,self->sx,self->sy + 15,(int)tx,(int)ty,10,BL_Teal,E_NONE);
        self->state = ST_IDLE;
        self->frame = 0;
      }
      else self->frame++;
    break;
    case ST_IDLE:
      self->frame++;
      if(self->frame >= 3)
        self->frame = 0;
      if(self->delay == 0)
      {
        if((self->vy >= 5)||(self->vy <= -5))
        {
          self->ammo *= -1;   /*ammo is not ammo, its the accelleration in the y direction*/
        }
        self->vy += self->ammo;
        self->delay += rand() % 26;
      }
      if(self->delay > 0)self->delay--;
      if(self->heat == 0)
      {
        self->state = ST_FIRE1;
        self->frame = 4;
        self->heat = 10 + rand()%10;
      }
      if(self->heat > 0)self->heat--;
    break;
  }
}

void MantThink(Entity *self)
{
  SDL_Rect b1,b2;
  self->sx += self->vx;
  self->sy += self->vy;
  if(self->vx > -2)self->vx--;
  if(self->sy < 0)
  {
    self->sy = 0;
    self->vy += 15;
    self->delay += rand() % 20;
  }
  if(self->sy > screen->h - 64)
  {
    self->sy = screen->h - 64;
    self->vy -= 15;
    self->delay += rand() % 20;
  }
  /*check for colision with player*/
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  if((self->health <= 0)&&(self->state != ST_DYING))
  {
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[0]->sound,0);
  }
  else if((Player->health > 0)&&(self->state != ST_DYING)&&(Collide(b1,b2)))
  {
    Player->health -= 7;
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[1]->sound,0);
  }
  /*Check to see if I should die*/
  if(self->sx < - 32)
  {
    self->shown = 0;
    FreeEntity(self);
  }
  switch(self->state)
  {
    case ST_DYING:
      if(self->frame >= 10)
      {
        FreeEntity(self);
      }
      self->frame++;
    break;
    case ST_IDLE:
      self->frame++;
      if(self->frame >= 4)
        self->frame = 0;
      if(self->delay == 0)
      {
        if((self->vy >= 5)||(self->vy <= -5))
        {
          self->ammo *= -1;
        }
        self->vy += self->ammo;
        self->delay += rand() % 20;
      }
      if(self->delay > 0)self->delay--;
    break;
  }
}


/*
  Spawn a generic Bug enemy
*/
Entity *SpawnBug(int sx,int sy,int type)
{
  Entity *bug;
  bug = NewEntity();
  if(bug == NULL)return bug;
  bug->sprite = LoadSprite("images/bug1.png",32,32,-1,-1,-1);
  bug->sound[0] = LoadSound("sounds/bugdeath.wav",MIX_MAX_VOLUME/2);
  bug->sound[1] = LoadSound("sounds/burst.wav",MIX_MAX_VOLUME/2);
  bug->sound[2] = LoadSound("sounds/bugattack.wav",MIX_MAX_VOLUME/2);
  bug->shown = 1;
  bug->sx = sx;
  bug->sy = sy;
  bug->vx = -2;
  bug->vy = 0;
  if(!type)bug->think = BugThink;
  else bug->think = AttackBugThink;
  bug->bbox.x = 4;
  bug->bbox.y = 4;
  bug->bbox.w = 20;
  bug->bbox.h = 20;
  bug->frame = 0;
  bug->state = ST_IDLE;
  bug->health = 10;
  bug->owner = NULL;
  bug->enemy = E_Player;
  return bug;
}
/*
  Returns 1 if Box1 Collides with Box2.  A second check may be necessary with the boxes reversed
*/


void AttackBugThink(Entity *self)
{
  SDL_Rect b1,b2;
  self->sx += self->vx;
  self->sy += self->vy;
  if(self->vx > -2)self->vx--;
  if(self->sy < 0)
  {
    self->sy = 0;
    self->vy += 15;
    self->delay += rand() % 20;
  }
  if(self->sy > screen->h - 64)
  {
    self->sy = screen->h - 64;
    self->vy -= 15;
    self->delay += rand() % 10;
  }
  /*check for colision with player*/
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  if((self->health <= 0)&&(self->state != ST_DYING))
  {
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[0]->sound,0);
  }
  else if((Player->health > 0)&&(self->state != ST_DYING)&&(Collide(b1,b2)))
  {
    Player->health -= 5;
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[1]->sound,0);
  }
  /*Check to see if I should die*/
  if(self->sx < - 32)
  {
    self->shown = 0;
    FreeEntity(self);
  }
  switch(self->state)
  {
    case ST_DYING:
      if(self->frame >= 10)
      {
        if((rand() % 8) == 0)DropMissile(self->sx,self->sy,-4,0);
        FreeEntity(self);
      }
      self->frame++;
    break;
    case ST_FIRE1:
      if(self->frame >= 5)
      {
        /*spawn purple bullet*/
        SpawnBlast(self,self->sx,self->sy + 15,-4,0,5,BL_Purple,E_NONE);
        self->state = ST_IDLE;
        self->frame = 0;
        self->vx = -2;
      }
      else self->frame++;
    break;
    case ST_IDLE:
      self->frame++;
      if(self->frame >= 3)
        self->frame = 0;
      if(self->delay == 0)
      {
        if(abs(Player->sy - self->sy) <= 3)
        {
          self->state = ST_FIRE1;
          self->vx = 0;
          self->vy = 0;
          Mix_PlayChannel(-1,self->sound[2]->sound,0);
        }
        if(Player->sy < self->sy)
        {
          if(self->vy > -8)
          {
            self->vy -= 5;
            if(self->vy < -8)self->vy = -8;
          }
        }
        if(Player->sy > self->sy)
        {
          if(self->vy < 8)
          {
            self->vy += 5;
            if(self->vy > 8)self->vy = 8;
          }
        }
        self->delay += rand() % 20;
      }
      if(self->delay > 0)self->delay--;
    break;
  }
}

/*this is the generic dumb bug that will happily sacrafice itself for his people.*/
void BugThink(Entity *self)
{
  SDL_Rect b1,b2;
  self->sx += self->vx;
  self->sy += self->vy;
  if(self->vx > -2)self->vx--;
  if(self->sy < 0)
  {
    self->sy = 0;
    self->vy += 15;
    self->delay += rand() % 20;
  }
  if(self->sy > screen->h - 64)
  {
    self->sy = screen->h - 64;
    self->vy -= 15;
    self->delay += rand() % 20;
  }
  /*check for colision with player*/
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  if((self->health <= 0)&&(self->state != ST_DYING))
  {
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[0]->sound,0);
  }
  else if((Player->health > 0)&&(self->state != ST_DYING)&&(Collide(b1,b2)))
  {
    Player->health -= 5;
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[1]->sound,0);
  }
  /*Check to see if I should die*/
  if(self->sx < - 32)
  {
    self->shown = 0;
    FreeEntity(self);
  }
  switch(self->state)
  {
    case ST_DYING:
      if(self->frame >= 10)
      {
        FreeEntity(self);
      }
      self->frame++;
    break;
    case ST_IDLE:
      self->frame++;
      if(self->frame >= 3)
        self->frame = 0;
      if(self->delay == 0)
      {        
        if(Player->sy < self->sy)
        {
          if(self->vy > -8)
          {
            self->vy -= 5;
            if(self->vy < -8)self->vy = -8;
          }
        }
        if(Player->sy > self->sy)
        {
          if(self->vy < 8)
          {
            self->vy += 5;
            if(self->vy > 8)self->vy = 8;
          }
        }
        self->delay += rand() % 20;
      }
      if(self->delay > 0)self->delay--;
    break;
  }
}
/*
 *  Powerups to win the game
 */
Entity *DropPowerUp(int sx,int sy,int vx,int vy,int level)
{
  Entity *missile;
  missile = NewEntity();
  if(missile == NULL)return missile;
  missile->sprite = LoadSprite("images/effects.png",16,16,-1,-1,-1);
  missile->bbox.x = 1;
  missile->bbox.y = 2;
  missile->bbox.w = 10;
  missile->bbox.h = 10;
  missile->frame = 22 + level;
  missile->sx = sx;
  missile->sy = sy;
  missile->vx = vx;
  missile->vy = vy;
  missile->weaplevel = level;
  missile->shown = 1;
  missile->state = ST_IDLE;
  missile->think = PowerPickup;
  missile->enemy = E_NONE;
  return missile;
}

void PowerPickup(Entity *self)
{
  SDL_Rect b1,b2;
  if((self->sx < - 16)||(self->sx < - 16)||(self->sy > screen->w - 32))
  {
    self->shown = 0;
    FreeEntity(self);
  }
  self->sx += self->vx;
  self->sy += self->vy;
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  if(Collide(b1,b2))
  {
    /*play a sound when I make one*/
    if(Player->weaplevel < self->weaplevel + 1)Player->weaplevel++;
    FreeEntity(self);
  }
}

/*
  DropMissile will create a missile item pickup for the player.
*/

Entity *DropMissile(int sx,int sy,int vx,int vy)
{
  Entity *missile;
  missile = NewEntity();
  if(missile == NULL)return missile;
  missile->sprite = LoadSprite("images/effects.png",16,16,-1,-1,-1);
  missile->bbox.x = 1;
  missile->bbox.y = 4;
  missile->bbox.w = 7;
  missile->bbox.h = 6;
  missile->frame = 22;
  missile->sx = sx;
  missile->sy = sy;
  missile->vx = vx;
  missile->vy = vy;
  missile->shown = 1;
  missile->state = ST_IDLE;
  missile->think = MissilePickup;
  missile->enemy = E_NONE;
  return missile;
}

void MissilePickup(Entity *self)
{
  SDL_Rect b1,b2;
  if((self->sx < - 16)||(self->sx < - 16)||(self->sy > screen->w - 32))
  {
    self->shown = 0;
    FreeEntity(self);
  }
  self->sx += self->vx;
  self->sy += self->vy;
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  if(Collide(b1,b2))
  {
    /*play a sound when I make one*/
    if(Player->ammo < 10)Player->ammo++;
    FreeEntity(self);
  }
}

/*
  FireMissile wlll spawn a rocket out of the player's ship
  It will Gravitate toward the nearest enemy.
*/

Entity *FireMissile(Entity *Owner,int sx,int sy,int vx,int vy,int damage,int enemy)
{
  Entity *missile;
  missile = NewEntity();
  if(missile == NULL)return missile;
  missile->sprite = LoadSprite("images/effects.png",16,16,-1,-1,-1);
  missile->sound[0] = LoadSound("sounds/rocketfly.wav",MIX_MAX_VOLUME/5);
  missile->sound[1] = LoadSound("sounds/xplode.wav",MIX_MAX_VOLUME);
  missile->bbox.x = 2;
  missile->bbox.y = 6;
  missile->bbox.w = 6;
  missile->bbox.h = 2;
  missile->frame = 10;
  missile->sx = sx;
  missile->sy = sy;
  missile->vx = vx;
  missile->vy = vy;
  missile->shown = 1;
  missile->state = ST_FIRE1;
  missile->think = MissileThink;
  missile->health = 50;
  missile->owner = Owner;
  missile->enemy = enemy;
  return missile;
}

void MissileThink(Entity *self)
{
  Entity *target;
    /*Check for colision if not already dying*/
  if(self->sx > screen->w)
  { //We have gone off the map, there be dragons here, so lets get rid of us.
    self->shown = 0;
    FreeEntity(self);
    return;
  }
  if(self->state != ST_DYING)
  {
    target = HitEnt(self);
    if(target != NULL)
    {
      self->state = ST_DYING;
      self->frame = 14;
      target->health -= self->health;
      if(target->health < 0)self->health = target->health * -1;
      target->sx += self->vx * self->health / 2;
      self->vx = 0;
      self->vy = 0;
      Mix_PlayChannel(-1,self->sound[1]->sound,2);
    }
  }
  switch(self->state)
  {
    case ST_FIRE1:
      self->frame++;
      if(self->frame >= 13)
      {
        self->state = ST_IDLE;
      }
    break;
    case ST_DYING:
      self->frame++;
      if(self->frame >= 19)
      {
        self->bbox.x -= 100;
        self->bbox.y -= 100;
        self->bbox.w += 200;
        self->bbox.h += 200;
        target = HitEnt(self);
        while(target != NULL)
        {
          target->health -= self->health;
          Mix_PlayChannel(-1,self->sound[1]->sound,2);
          target = HitNextEnt(self,target);
        }
        FreeEntity(self);
      }
    break;
    case ST_IDLE:
      self->sx += self->vx;
      self->sy += self->vy;
      self->frame++;
      target = GetClosestTarget(self);
      if(target != NULL)
      {
        if(self->sy < target->sy)self->vy = 2;
        else if(self->sy > target->sy)self->vy = -2;
        else self->vy = 0;
      }
      if(self->frame >= 14)self->frame = 13;
      if(self->delay <= 0)
      {
        Mix_PlayChannel(-1,self->sound[0]->sound,2);
        self->delay = 3;
      }
      else self->delay--;
    break;
  }
}

/*
    SpawnBlast will create the basic blaster bolts used in the game.
*/

Entity *SpawnBlast(Entity *Owner,int sx,int sy,int vx,int vy,int damage,int type,int enemy)
{
  Entity *blast;
  blast = NewEntity();
  if(blast == NULL)return blast;
  blast->owner = Owner;
  blast->sprite = LoadSprite("images/effects.png",16,16,-1,-1,-1);
  blast->shown = 1;
  blast->health = damage;
  blast->sx = sx;
  blast->sy = sy;
  blast->vy = vy;
  blast->vx = vx;
  blast->enemy = enemy;
  switch(type)
  {
    case BL_Blue:
      blast->frame = 6;
      blast->state = ST_IDLE;
      blast->think = BlueBlastThink;
      blast->bbox.x = 6;
      blast->bbox.y = 6;
      blast->bbox.w = 5;
      blast->bbox.h = 2;
    break;
    case BL_Blue2:
      blast->frame = 7;
      blast->state = ST_IDLE;
      blast->think = BlueBlastThink;
      blast->bbox.x = 6;
      blast->bbox.y = 4;
      blast->bbox.w = 5;
      blast->bbox.h = 3;
    break;
    case BL_Blue3:
      blast->frame = 8;
      blast->state = ST_IDLE;
      blast->think = BlueBlastThink;
      blast->bbox.x = 6;
      blast->bbox.y = 3;
      blast->bbox.w = 5;
      blast->bbox.h = 5;
    break;
    case BL_Red:
      blast->frame = 0;
      blast->state = ST_FIRE1;
      blast->think = RedBlastThink;
      blast->bbox.x = 1;
      blast->bbox.y = 5;
      blast->bbox.w = 13;
      blast->bbox.h = 5;
    break;
    case BL_Purple:
      blast->frame = 20;
      blast->state = ST_IDLE;
      blast->think = PurpleBlastThink;
      blast->bbox.x = 6;
      blast->bbox.y = 6;
      blast->bbox.w = 4;
      blast->bbox.h = 4;
    break;
    case BL_Teal:
      blast->frame = 21;
      blast->state = ST_IDLE;
      blast->think = PurpleBlastThink;
      blast->bbox.x = 5;
      blast->bbox.y = 5;
      blast->bbox.w = 6;
      blast->bbox.h = 6;
    break;
    default:
      blast->frame = 6;
      blast->state = ST_IDLE;
      blast->think = BlueBlastThink;
      blast->bbox.x = 6;
      blast->bbox.y = 6;
      blast->bbox.w = 5;
      blast->bbox.h = 2;
  }
  
  return blast;
}

void PurpleBlastThink(Entity *self)
{
  SDL_Rect b1,b2;
    /*Update position*/
  self->sx += self->vx;
  self->sy += self->vy;
    /*Check for colision if not already dying*/
  if((self->sx < -16)||(self->health <= 0))
  { //We have gone off the map, there be dragons here, so lets get rid of us.
    self->shown = 0;
    FreeEntity(self);
    return;
  }
  b2.x = Player->sx + Player->bbox.x;
  b2.y = Player->sy + Player->bbox.y;
  b2.w = Player->bbox.w;
  b2.h = Player->bbox.h;
  b1.x = self->sx + self->bbox.x;
  b1.y = self->sy + self->bbox.y;
  b1.w = self->bbox.w;
  b1.h = self->bbox.h;
  switch(self->state)
  {
    case ST_DYING:
      self->shown = 0;
      FreeEntity(self);
    break;
    case ST_IDLE:
      if(Collide(b1,b2))
      {
        self->state = ST_DYING;
        self->vx = 0;
        self->vy = 0;
        Player->health -= self->health;
      }
    break;
  }
}

void BlueBlastThink(Entity *self)
{
  Entity *target;
    /*Update position*/
  self->sx += self->vx;
  self->sy += self->vy;
    /*Check for colision if not already dying*/
  if(self->sx > screen->w)
  { //We have gone off the map, there be dragons here, so lets get rid of us.
    self->shown = 0;
    FreeEntity(self);
    return;
  }
  if(self->state != ST_DYING)
  {
    target = HitEnt(self);
    if(target != NULL)
    {
      self->state = ST_DYING;
      self->vx = 0;
      self->vy = 0;
      target->health -= self->health;
      target->vx += self->vx * self->health / 2;
    }
  }
  switch(self->state)
  {
    case ST_DYING:
      self->frame++;
      if(self->frame >= 10)FreeEntity(self);
    break;
  }
}

void RedBlastThink(Entity *self)
{
  Entity *target;
    /*Update position*/
  self->sx += self->vx;
  self->sy += self->vy;
    /*Check for colision if not already dying*/
  if(self->sx > screen->w)
  { //We have gone off the map, there be dragons here, so lets get rid of us.
    self->shown = 0;
    FreeEntity(self);
    return;
  }
  if(self->state != ST_DYING)
  {
    target = HitEnt(self);
    if(target != NULL)
    {
      target->health -= self->health;
      target->vx += self->vx * self->health / 4;
      if(target->health < 0)self->health = target->health * -1;
      else
      {
        self->state = ST_DYING;
        self->vx = 0;
        self->vy = 0;
      }
    }
  }
  switch(self->state)
  {
    case ST_DYING:
      self->frame++;
      if(self->frame >= 5)FreeEntity(self);
    break;
    case ST_IDLE:
      self->frame = 2;
    break;
    case ST_FIRE1:
      self->frame++;
      if(self->frame == 2)self->state = ST_IDLE;
    break;
  }
}

/*
  MakeShip is a simple function that will handle all of the specifics of setting up the player's ship entity.

*/

Entity *MakeShip()
{
  Entity *ship;
  ship = NewEntity();
  if(ship == NULL)return ship;
  ship->sprite = LoadSprite("images/ship.png",32,32,-1,-1,-1); //Load the ship file, no palette swapping
  ship->sound[0] = LoadSound("sounds/blaster.wav",MIX_MAX_VOLUME/5);
  ship->sound[1] = LoadSound("sounds/redblast.wav",MIX_MAX_VOLUME/5);
  ship->sound[2] = LoadSound("sounds/xplode.wav",MIX_MAX_VOLUME/5);
  ship->sound[3] = LoadSound("sounds/explode.wav",MIX_MAX_VOLUME/5);
  ship->bbox.x = 3;
  ship->bbox.y = 7;
  ship->bbox.w = 21;
  ship->bbox.h = 12;
  ship->weaplevel = 0;
  ship->frame = 0;
  ship->sx = 100;
  ship->sy = screen->h/2;
  ship->shown = 1;
  ship->state = ST_IDLE;
  ship->think = PlayerThink;
  ship->health = 30;
  ship->healthmax = 30;
  ship->ammo = 5;
  ship->owner = ship;
  ship->enemy = E_Bugs;
  Player = ship;
  return ship;
}

void PlayerThink(Entity *self)
{
  int my,i;
  int numkeys;
  Uint8 *keys = SDL_GetKeyState(&numkeys);
  if(self->heat > 0)self->heat--;
  if(self->busy > 0)self->busy--;
  if(self->health > 0)
  {
    if(self->vy != 0)
    {
      if(self->vy > 0)self->vy -= 2;
      else self->vy += 2;
    }
    SDL_GetMouseState(NULL,&my);
    if(my != lasty)
    {
      lasty = my;
      self->vy = 0;
      if((my > self->sy +5 )||(my < self->sy -5 ))self->vy = my - self->sy;
    }
    self->sx += self->vx;
    self->sy += self->vy;
    if(self->sy < 0)self->sy = 0;
    if(self->sy > (screen->h - 64))self->sy = (screen->h - 64);
    if(self->sx > Camera.w)
    {
      /*We have completed a level, load a new one*/
      if(CurrentLevel + 1 < NumLevels)CurrentLevel++;
      else exit(1);
      RenderLevel(GetCurrentLevel(CurrentLevel));
      self->sx = 100;
      self->vx = 0;
    }
  }
  if((self->health <= 0)&&(self->state != ST_DYING)&&(self->state != ST_DEAD))
  {
    self->health = 0;
    self->state = ST_DYING;
    self->frame = 7;
    Mix_PlayChannel(-1,self->sound[3]->sound,0);
  }
  if(keys[SDLK_UP])
  {
          MOUSEMOVE = 0;
          if(self->vy > -25)self->vy -= 5;
  }
  if(keys[SDLK_DOWN])
  {
          MOUSEMOVE = 0;
          if(self->vy < 25)self->vy += 5;
  }
  if(keys[SDLK_s])
  {
        if((self->state == ST_IDLE)&&(self->heat < 100))
          {
            self->state = ST_FIRE1;
            self->heat += 30 - (self->weaplevel*2);
            Mix_PlayChannel(-1,self->sound[1]->sound,0);
          }
  }
  if(keys[SDLK_d])
  {
          if(self->state == ST_IDLE)
          {
            self->state = ST_FIRE2;
            self->frame = 5;
            Mix_PlayChannel(-1,self->sound[0]->sound,0);
          }
  }
  if(keys[SDLK_a])
  {
          if((self->state == ST_IDLE)&&(self->ammo > 0)&&(self->busy <= 0))
          {
            self->state = ST_FIRE3;
            self->delay = 5;
            self->ammo--;
            self->busy = 20;
            Mix_PlayChannel(-1,self->sound[2]->sound,0);
            //spawn big green missile
          }
  }
  switch(self->state)
  {
    case ST_DEAD:
      self->frame = 13;
      self->shown = 0;
      if(self->delay <= 0)
      {
       if(NumLives > 0)
       {
        self->health = self->healthmax;
        self->state = ST_IDLE;
        self->shown = 1;
        NumLives--;
       }
       else exit(1);
      }
      else self->delay--;
    break;
    case ST_DYING:
      if(self->frame >= 13)
      {
        self->state = ST_DEAD;
        self->delay = 5;
      }
      self->frame++;
    break;
    case ST_IDLE:
      self->frame = 0;
    break;
    case ST_FIRE1:
      if(self->delay > 0)
      {
        self->delay--;
        if(self->delay <= 0)
        {
          self->delay = 0;
          self->state = ST_IDLE;
        }
      }
      else if(self->frame >= 4)
      {
        SpawnBlast(self,self->sx + 24,self->sy + 11,5,0,25,BL_Red,E_Bugs);
        /*spawn the big red bullet;*/
        self->delay = 7 - self->weaplevel;
      }
      else self->frame++;
    break;
    case ST_FIRE2:
      if(self->frame >= 6)
      {
        switch(self->weaplevel)
        {
          case 0:
            SpawnBlast(self,self->sx + 10,self->sy + 15,8,0,2,BL_Blue,E_Bugs);
          break;
          case 1:
            SpawnBlast(self,self->sx + 10,self->sy + 15,9,0,3,BL_Blue2,E_Bugs);
          break;
          case 2:
            SpawnBlast(self,self->sx + 10,self->sy + 15,10,0,4,BL_Blue3,E_Bugs);
          break;
        }
        /*spawn the little blue bullet*/
        self->state = ST_IDLE;
      }
      else self->frame++;
    break;
    case ST_FIRE3:
      if(self->delay <= 0)
      {
        self->delay = 0;
        self->state = ST_IDLE;
        /*spawn a mini Nuke*/
        for(i = 0;i <= self->weaplevel;i++)
        FireMissile(self,self->sx + 14,self->sy + 10 + (i *15),10,0,50,E_Bugs);
      }
      else self->delay--;
    break;
  }
}








/**/








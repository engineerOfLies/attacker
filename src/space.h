#ifndef __space__
#define __space__

/*
  *   Donald Kehoe
  *   Way back in February
  *   Last modified: 3/3/05
  *   Header file to define the function prototype and datastructures for Entities
  *   Last change : Added comments.
*/

#include "SDL.h"
/*
 *  This file will define the data structures and function prototypes needed for rendering and animating
 *  the scrolling backgrounds for this game.
 *  The functions will create teh entities needed for an interactive level.
 */

#define TAGS          4
#define TILERES       12
#define LEVELLENGTH   128

enum Tags {T_Empty,T_POW1,T_POW2, T_Bug, T_BugA,T_Mant,T_MantA};

typedef struct MAP_NODE
{
  char tags[TAGS];   //This little array is used to fill the level with things such as debris, bugs and hazards
  char Tiles[TILERES];    //references the frame to render from the texture list.  so far I have 4
}MapNode_T;

typedef struct LEVEL_STRUCT
{
  MapNode_T map[LEVELLENGTH];
  Uint16  length;     //The actual length of the level, it need not be the max.
  char bgmusic[20];
}Level;

void LoadLevelSprites();  //this just loads the appropriate textures used to render the map.
void RenderLevel(Level *level);  //This will use the camera position to determine what to render.
void SpawnLevelEnts(Level *level);
Level *GetCurrentLevel(int level); //right now this doesn't do much
int MoveCamera(int Length);  // only moves in that -> direction
void UpdateLevel(Level *level);

#endif


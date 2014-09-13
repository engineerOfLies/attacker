#include <stdlib.h>
#include <string.h>
#include "SDL_getenv.h"
#include "audio.h"

#define MAXSOUNDCLIPS 128
int NumSounds = 0;

Sound sounds[MAXSOUNDCLIPS];

void Init_Audio()
{
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,AUDIO_S16SYS,2,4096) == -1)
  {
    fprintf(stderr, "Unable to init SDL Mixer: %s\n", Mix_GetError());
    exit(1);
  }
  atexit(Mix_CloseAudio);
  if(Mix_AllocateChannels(10) != 10)
  {
    fprintf(stderr, "Unable to allocate enough channels: %s\n", Mix_GetError());
    exit(1);
  }
  Mix_VolumeMusic(MIX_MAX_VOLUME);
}

void InitSoundList()
{
  int i;
  for(i = 0;i < MAXSOUNDCLIPS;i++)
  {
    sounds[i].sound = NULL;
    strcpy(sounds[i].filename,"\0");
    sounds[i].used = 0;
    sounds[i].volume = 0;
  }
  NumSounds = 0;
}

Sound *LoadSound(char filename[20],int volume)
{
  int i;
  /*first search to see if the requested sound is alreday loaded*/
  for(i = 0; i < MAXSOUNDCLIPS; i++)
  {
    if((strncmp(filename,sounds[i].filename,19)==0)&&(sounds[i].used >= 1))
    {
      sounds[i].used++;
      return &sounds[i];
    }
  }
  /*makesure we have the room for a new sound*/
  if(NumSounds + 1 >= MAXSOUNDCLIPS)
  {
        fprintf(stderr, "Maximum Sound clips Reached.\n");
        exit(1);
  }
  /*if its not already in memory, then load it.*/
  NumSounds++;
  for(i = 0;i < MAXSOUNDCLIPS;i++)
  {
    if(!sounds[i].used)break;
  }
  sounds[i].sound = Mix_LoadWAV(filename);
  if(sounds[i].sound == NULL)
  {
        fprintf(stderr, "FAILED TO LOAD A VITAL SOUND.\n");
        exit(1);
  }
  strcpy(sounds[i].filename,filename);
  sounds[i].volume = volume;
  sounds[i].used = 1;
  Mix_VolumeChunk(sounds[i].sound,volume);
  return &sounds[i];
}

void FreeSound(Sound *sound)
{
  if(sound->used > 1)
  {
    sound->used--;
    return;
  }
  NumSounds--;
  sound->used--;
  Mix_FreeChunk(sound->sound);
  sound->sound = NULL;
}

void ClearSoundList()
{
  int i;
  for(i = 0;i < MAXSOUNDCLIPS;i++)
  {
    if(sounds[i].sound != NULL)
    {
      Mix_FreeChunk(sounds[i].sound);
      sounds[i].sound = NULL;
    }
    strcpy(sounds[i].filename,"\0");
    sounds[i].used = 0;
    sounds[i].volume = 0;
  }
  NumSounds = 0;
}

/*end of file*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include "callbacks.h"
#include "graphics.h"
#include "filer.h"

#include "psp.h"
#include "CHIP8.h"

#define printf pspDebugScreenPrintf
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y)) 

PSP_MODULE_INFO("Chip-8", 0, 1, 1);


int main(int argc, char **argv)
{
	pspDebugScreenInit();
  SetupCallbacks();
  
  pspAudioInit();
  initGraphics();
  
  srand(time(NULL));
  
  char Ebootpath[256];
  strcpy(Ebootpath, argv[0]);
  char *p = strrchr(Ebootpath, '/');
  *p = 0;
  
  FileBrowser(Ebootpath);
  
  sceKernelSleepThread();

 	exit_callback();
 	return 0;
} 

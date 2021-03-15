/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                                MSDOS.c                                 **/
/**                                                                        **/
/** This file contains the MS-DOS implementation                           **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#include <pspctrl.h>
#include "CHIP8.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <pspdisplay.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include "graphics.h"
#include "psp.h"

static volatile byte timer_count;               /* 55Hz counter             */
static volatile byte keyb_status[256];          /* If 1, key is pressed     */
static int  sync=1;                             /* If 0, do not sync        */
                                                /* emulation                */
static byte uperiod=1;                          /* number of interrupts per */
                                                /* screen update            */
                  

static long ReadTimer (void)
{
#ifdef HAVE_CLOCK
 return (long)((float)clock()*1000000.0/(float)CLOCKS_PER_SEC);
#else
 /* If clock() is unavailable, just return a large number */
 static long a=0;
 a+=1000000;
 return a;
#endif
}


const float PI = 3.1415926535897932f;
const int sampleRate = 44100;
float frequency = 440.0f;
float curtime = 0;

typedef struct {
        short l, r;
} sample_t;

float currentFunction(const float time) {
  double x;
  if(modf(time / (2 * PI), &x)) 
	  return -0.2f;
	else
	  return 0.2f;
}


/* This function gets called by pspaudiolib every time the
   audio buffer needs to be filled. The sample format is
   16-bit, stereo. */
void audioCallback(void* buf, unsigned int length, void *userdata) {
        const float sampleLength = 1.0f / sampleRate;
	const float scaleFactor = SHRT_MAX - 1.0f;
        static float freq0 = 440.0f;
       	sample_t* ubuf = (sample_t*) buf;
	int i;
	
	if (frequency != freq0) {
	        curtime *= (freq0 / frequency);
	}
	for (i = 0; i < length; i++) {
	        short s = (short) (scaleFactor * currentFunction(2.0f * PI * frequency * curtime));
		ubuf[i].l = s;
		ubuf[i].r = s;
		curtime += sampleLength;
	}
	if (curtime * frequency > 1.0f) {
	        double d;
		curtime = modf(curtime * frequency, &d) / frequency;
	}
	freq0 = frequency;
}

static int sChannel = 0;

/****************************************************************************/
/* Turn sound on                                                            */
/****************************************************************************/
void chip8_sound_on (void)
{
	pspAudioSetChannelCallback(sChannel, audioCallback, NULL);
}

/****************************************************************************/
/* Turn sound off                                                           */
/****************************************************************************/
void chip8_sound_off (void)
{
  pspAudioSetChannelCallback(sChannel, 0, NULL);
}

/****************************************************************************/
/* Update the display                                                       */
/****************************************************************************/
static void update_display (void)
{
	clearScreen(0x00);
	
	#ifdef CHIP8_SUPER
	const int mag = 2;
  #else
  const int mag = 4;
  #endif

	Image *dis = (Image*)malloc(sizeof(Image));
	dis = createImage(CHIP8_WIDTH*mag, CHIP8_HEIGHT*mag);

	u32 *od = dis->data;
	byte *d = chip8_display;
	
	int i = 0;
	for(int y=0;y<CHIP8_HEIGHT;y++)
	{
		for(int my=0;my<mag;my++)
		{
			byte *dd = d;
			for(int x=0;x<CHIP8_WIDTH;x++)
			{  
				for(int mx = 0;mx<mag;mx++)
				{
					if(*dd==0xff)
			  		*od = 0xffffff;
					else
						*od = 0x000000;
					od++;
			  }
			  dd++;
			  i++;
			}
		}
		d+=CHIP8_WIDTH;
	}
	blitImageToScreen(0, 0, dis->imageWidth, dis->imageHeight, dis, (480/2)-((CHIP8_WIDTH*mag)/2), (272/2)-((CHIP8_HEIGHT*mag)/2));
	
	flipScreen();
  
  freeImage(dis);
}

/****************************************************************************/
/* Update CHIP8 keyboard status                                             */
/****************************************************************************/
static void check_keys (void)
{
	memset (chip8_keys,0,sizeof(chip8_keys));
	
  SceCtrlData pad;
  sceCtrlReadBufferPositive(&pad, 1);
  if(pad.Buttons & PSP_CTRL_LEFT) chip8_keys[0x04] = 1;
  if(pad.Buttons & PSP_CTRL_RIGHT) chip8_keys[0x06] = 1;
  if(pad.Buttons & PSP_CTRL_UP) chip8_keys[0x08] = 1;
  if(pad.Buttons & PSP_CTRL_DOWN) chip8_keys[0x02] = 1;
  	
  if(pad.Buttons & PSP_CTRL_SQUARE) chip8_keys[0x03] = 1;
  if(pad.Buttons & PSP_CTRL_CIRCLE) chip8_keys[0x05] = 1;
  if(pad.Buttons & PSP_CTRL_TRIANGLE) chip8_keys[0x07] = 1;
  if(pad.Buttons & PSP_CTRL_CROSS) chip8_keys[0x01] = 1;
  
  if(pad.Buttons & PSP_CTRL_START) chip8_running = 0;
}

/****************************************************************************/
/* Return time passed in 55th seconds                                       */
/****************************************************************************/
static byte get_timer_count (void)
{
 return timer_count;
}

/****************************************************************************/
/* Update keyboard and display, sync emulation with hardware timer          */
/****************************************************************************/
void chip8_interrupt (void)
{
 clock_t newtimer;
 static int ucount=1;
 if (!--ucount)
 {
  ucount=uperiod;
  update_display ();
 }
 check_keys ();
 if (sync)
 {
  newtimer=ReadTimer ();
  timer_count+=20000;
  if ((newtimer-timer_count)<0)
  {
   do
    newtimer=ReadTimer ();
   while ((newtimer-timer_count)<0);
  }
  else timer_count=newtimer;
 }
}

/****************************************************************************/
/* New timer interrupt routine                                              */
/****************************************************************************/
static void new_int8 (void)
{
 
}

/****************************************************************************/
/* New keyboard interrupt routine                                           */
/****************************************************************************/
static void new_int9 (void)
{
 
}


int Emulate(char *szFileName)
{
	FILE *file;
	chip8_iperiod=15;
	 
	file = fopen(szFileName,"rb");
	if(!file) return 0;
		
	int r = fread(chip8_mem+0x200,1,4096-0x200,file);
	fclose(file);
	
	if(r==0) return 0;

	chip8();

  return 1;
}

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <psptypes.h>

#define	PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define SIZE_LINE    176		//in pixel
#define GUARD_LINE   8			//in pixel

#define RGB(r,g,b) (((b)<<16)|((g)<<8)|(r))
#define RGBA(r,g,b,a) (((a)<<24)|((b)<<16)|((g)<<8)|(r))
#define RGB_R(c) ((c) & 0xFF)
#define RGB_G(c) (((c) >> 8) & 0xFF)
#define RGB_B(c) (((c) >> 16) & 0xFF)
#define RGB_16to32(c) RGBA((((c)&0x1F)*255/31),((((c)>>5)&0x1F)*255/31),((((c)>>10)&0x1F)*255/31),((c&0x8000)?0xFF:0))

#define IS_ALPHA(color) (((color)&0xff000000)==0xff000000?0:1)
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*4)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))


typedef struct
{
	unsigned short u, v;
	short x, y, z;
} Vertex;

typedef struct
{
	unsigned int color;
	short x, y, z;
} CSVertex;

typedef struct
{
	int textureWidth;  // the real width of data, 2^n with n>=0
	int textureHeight;  // the real height of data, 2^n with n>=0
	int imageWidth;  // the image width
	int imageHeight;
	Color* data;
} Image;

int getNextPower2(int width);

extern void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);
extern void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);

extern Image* createImage(int width, int height);

int freeImage(Image* image);

extern void clearScreen(Color color);

extern void printText(int x, int y, const char* text, u32 color);

extern void printTextf(int x, int y, u32 color, const char* text, ...);

extern void flipScreen();

/**
 * Initialize the graphics.
 */
extern void initGraphics();

/**
 * Disable graphics, used for debug text output.
 */
extern void disableGraphics();

extern Color* getVramDrawBuffer();

/**
 * Get the current display buffer for fast unchecked access.
 *
 * @return the start address of the current display buffer
 */
extern Color* getVramDisplayBuffer();

extern void guStart();

#endif

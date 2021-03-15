#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>

#include "graphics.h"
#include "framebuffer.h"

extern u8 msx[];

unsigned int __attribute__((aligned(16))) list[262144];
static int dispBufferNumber;
static int initialized = 0;

int getNextPower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

Color* getVramDrawBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if(dispBufferNumber>0) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

Color* getVramDisplayBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if(dispBufferNumber==0) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;
	Color* vram = getVramDrawBuffer();
	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuCopyImage(GU_PSM_8888, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_LINE_SIZE, vram);
	sceGuFinish();
	sceGuSync(0,0);
}

void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, (void*) source->data);
	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	
	int j = 0;
	while (j < width) {
		Vertex* vertices = (Vertex*) sceGuGetMemory(2 * sizeof(Vertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}


Image* createImage(int width, int height)
{
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if (!image->data) return NULL;
	memset(image->data, 0, image->textureWidth * image->textureHeight * sizeof(Color));
	return image;
}

int freeImage(Image* image)
{
	if(image==NULL || image->data==NULL) return 0;
	free(image->data);
	image->data = NULL;
	free(image);
	image = NULL;
	return 1;
}

void clearScreen(Color color)
{
	if (!initialized) return;
	guStart();
	sceGuClearColor(color);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}

void printText(int x, int y, const char *text, u32 color)
{
	int c, i, j, l, ox=x;
	u8 *font;
	Color *vram_ptr;
	Color *vram;
	
	if (!initialized) return;

	for (c = 0; c < (int)strlen(text); c++) {
		if (x < 0 || y < 0 || y + 8 > SCREEN_HEIGHT) break;
		char ch = text[c];
		if(x > SCREEN_WIDTH){
			x=ox;
			y+=8;
		}
		vram = getVramDrawBuffer() + x + y * PSP_LINE_SIZE;
		if(ch=='\n'){
			x=ox;
			y+=8;
			if(y > SCREEN_HEIGHT)
				return;
			continue;
		}
		if(ch=='\0')
			return;
		font = &msx[ (int)ch * 8];
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			vram_ptr  = vram;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *vram_ptr = color;
				vram_ptr++;
			}
			vram += PSP_LINE_SIZE;
		}
		x += 8;
	}
}

void printTextf(int x, int y, u32 color, const char *text, ...)
{
	va_list ap;
	char szBuf[(480*272)/8];

	va_start(ap, text);
	vsprintf(szBuf, text, ap);
	va_end(ap);
	printText(x, y, szBuf, color);
}

void flipScreen()
{
	if (!initialized) return;
		
	sceGuSwapBuffers();
	
	dispBufferNumber^=1;
}

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

void initGraphics()
{
	dispBufferNumber = 1;

	sceGuInit();

	guStart();
	sceGuDrawBuffer(GU_PSM_8888, (void*)FRAMEBUFFER_SIZE, PSP_LINE_SIZE);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthBuffer((void*) (FRAMEBUFFER_SIZE*2), PSP_LINE_SIZE);
	sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuAmbientColor(0xffffffff);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	initialized = 1;
}

void disableGraphics()
{
	initialized = 0;
}

void guStart()
{
	sceGuStart(GU_DIRECT, list);
}

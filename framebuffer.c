#include "graphics.h"
#include "framebuffer.h"

Color* g_vram_base = (Color*) (0x40000000 | 0x04000000);


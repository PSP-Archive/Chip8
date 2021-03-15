#include <pspctrl.h>
#include <psptypes.h>

extern u32 new_pad;
extern u32 old_pad;
extern u32 now_pad;
extern SceCtrlData paddata;


void readpad(void);
void readanalog(unsigned int x, unsigned int y);

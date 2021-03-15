#include "controller.h"

u32 new_pad;
u32 old_pad;
u32 now_pad;
SceCtrlData paddata;

//Read controller information
void readpad()
{
	static int n=0;

	sceCtrlReadBufferPositive(&paddata, 1);

	now_pad = paddata.Buttons;
	new_pad = now_pad & ~old_pad;
	if(old_pad==now_pad){
		n++;
		if(n>=15){
			new_pad=now_pad;
			n = 10;
		}
	}else{
		n=0;
		old_pad = now_pad;
	}
}


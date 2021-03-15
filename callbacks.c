#include <pspkernel.h>
#include "callbacks.h"

/* Exit callback */
int exit_callback() {
 	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp) {
  int cbid;
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, 0);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
	int thid = 0;
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0) {
  	sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}
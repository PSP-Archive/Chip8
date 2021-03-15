#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include "graphics.h"
#include "filer.h"
#include "controller.h"

typedef struct
{
	int Type; //0=regular file, 1=dir, 2=..
	char *szPath;
} DIRENTRY;

int findDirEntries(char *szDirname, DIRENTRY * dirEnt) 
{  
	SceIoDirent g_dir;
	int fd = sceIoDopen(szDirname); 
	if (fd < 0)
		return 0;  

	memset(&g_dir, 0, sizeof(SceIoDirent));  

	int i = 0;
	if(strcmp(szDirname, "ms0:"))
	{
	  dirEnt[0].Type = 2;
	  dirEnt[0].szPath = (char*)malloc(strlen(szDirname)+(strlen("..")+1)*sizeof(char)); 
    sprintf(dirEnt[0].szPath, "%s/..", szDirname);
    i++;
  }
	

	while (sceIoDread(fd, &g_dir)) 
	{  
		if(g_dir.d_name[0]=='.') continue;
		dirEnt[i].Type = FIO_S_ISDIR(g_dir.d_stat.st_mode);
		  
		dirEnt[i].szPath = (char*)malloc(strlen(szDirname)+(strlen(g_dir.d_name)+1)*sizeof(char)); 
		sprintf(dirEnt[i].szPath, "%s/%s", szDirname, g_dir.d_name);
		i++; 
	} 
	sceIoDclose(fd); 
	return i;
}

void bubble_sort(int numItems)
{
	
}

void FileBrowser(char *szStartDir)
{
	const int row = 30;
	int sel = 0, srow = 0;
	
	char curPath[256];
	strcpy(curPath, szStartDir);

	DIRENTRY curDir[256];
	int numEnt = findDirEntries(szStartDir, curDir);
	while(1)
	{
		readpad();
		if(new_pad & PSP_CTRL_CROSS)
		{
			if(curDir[sel].Type==0)
			{  
				Emulate(curDir[sel].szPath);
			}
			else if(curDir[sel].Type==2)
			{ 
				char *p = strrchr(curDir[sel].szPath, '/');
				if(p)
				{
				  *p = 0;
				  p = strrchr(curDir[sel].szPath, '/');
				  *p = 0;
				  strcpy(curPath, curDir[sel].szPath);
				  numEnt = findDirEntries(curDir[sel].szPath, curDir);
				  sel = 0;
				}
			}else{
				strcpy(curPath, curDir[sel].szPath);
				numEnt = findDirEntries(curDir[sel].szPath, curDir); 
				sel = 0;
			}
		}
		if(new_pad & PSP_CTRL_DOWN) sel++;
		if(new_pad & PSP_CTRL_UP) sel--;


		if(sel >= numEnt)		sel=0;
		if(sel < 0)				sel=numEnt-1;
		if(srow > numEnt-row)	srow=numEnt-row;
		if(srow < 0)				srow=0;
		if(sel >= srow+row)		srow=sel-row+1;
		if(sel < srow)			srow=sel;
		
		clearScreen(0x00);
		
		printTextf(0, 0, RGB(255,255,255), "%s", curPath);
		
		for(int i=0;i<row;i++)
		{  
			if(i>=numEnt) break;
			char *p = strrchr(curDir[i+srow].szPath, '/');
			printTextf(4, (i*8)+16, sel==i+srow?RGB(255,255,255):RGB(200,200,200), "%s%s", (p)?(p+1):(curDir[i+i].szPath), curDir[i+srow].Type==1?"/":"");
		}
		sceDisplayWaitVblankStart();
		flipScreen();
	}
}


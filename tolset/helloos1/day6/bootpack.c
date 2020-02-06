/*告诉C编译器，有个函数在别的文件里*/

#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里
#include <stdio.h>      //<>表示该文件位于编译器所提供的文件夹里

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40],mcursor[256];
	int mx, my;
	
	init_gdtidt();
	init_pic();
	io_sti();
	
	init_palette();	
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	
	mx = (binfo->scrnx - 16)/2;
	my = (binfo->scrny - 28-16) /2;
	init_mouse_cursor8(mcursor, COL8_840084);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor,16);
	
	//putfonts8_asc(binfo->vram, binfo->scrnx, 8,8, COL8_000000, "hey guys!");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 7,7, COL8_FFFFFF, "hey guys!"); //神奇的阴影效果！
	
	sprintf(s,"mouse:( %d ,%d )",mx,my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 8,31,COL8_FFFFFF, s);
	
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);
	for(;;)
	{
		io_hlt();		/*执行naskfunc里的_io_hlt*/
	}
}


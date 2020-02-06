/*告诉C编译器，有个函数在别的文件里*/

#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里
#include <stdio.h>      //<>表示该文件位于编译器所提供的文件夹里

extern struct KEYBUF keybuf;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40],mcursor[256];
	int mx, my;
	int i,j;
	
	init_gdtidt();
	init_pic();
	io_sti();
	
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);
	
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
	
	for(;;)
	{
		io_cli();  				 //先屏蔽中断，因为此时如果有中断进来会乱，so先去看flag
		/*if(keybuf.flag == 0)	 //此时没有键被按下，等待的同时不如hlt
		{
			io_stihlt();		 //但hlt就接不到中断了，所以stihlt一起被执行，等待被唤醒
		}
		else{						//有按键数据，先存到变量i，清空缓存，开放中断，显示字符
			i = keybuf.data;
			keybuf.flag = 0;
			io_sti();
			sprintf(s,"%02x",i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0,16,15,31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
		if(keybuf.next == 0)
		{
			io_stihlt();
		}
		else{
			i = keybuf.data[0];
			keybuf.next --;
			for( j = 0;j < keybuf.next;j++)
			{
				keybuf.data[j] = keybuf.data[j+1];
			}
			io_sti();
			sprintf(s,"%02x",i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0,16,15,31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}*/
		if(keybuf.len == 0)  //类似环状链表，读写指针到32结尾时跳到1重新读写，大大提高效率，缓冲区可以记录大量数据
		{
			io_stihlt();
		}
		else{
			i = keybuf.data[keybuf.next_r];
			keybuf.len--;
			keybuf.next_r++;
			if(keybuf.next_r ==32)
			{
				keybuf.next_r = 0;
			}
			io_sti();
			sprintf(s,"%02x",i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0,16,15,31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
		//io_hlt();		/*执行naskfunc里的_io_hlt*/替换成上边的io_stihlt()
	}
}


#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
{
	io_out8(PIC0_IMR,	0xff); //禁止所有中断
	io_out8(PIC1_IMR,	0xff);
	
	io_out8(PIC0_ICW1,	0x11); //边沿触发模弁E
	io_out8(PIC0_ICW2,	0x20); //IRQ0-7由INT20-27接收
	io_out8(PIC0_ICW3,	1<<2); //PIC1由IRQ2链接
	io_out8(PIC0_ICW4,	0x01); //无缓傅区模弁E
	
	io_out8(PIC1_ICW1,	0x11);
	io_out8(PIC1_ICW2,	0x28);//IRQ8-15由INT28-2f接收
	io_out8(PIC1_ICW3,	2	);
	io_out8(PIC1_ICW4,	0x01);
	
	io_out8(PIC0_IMR,	0xfb);//11111011 PIC1以外傅部禁止
	io_out8(PIC1_IMR,	0xff); //禁止所有中断
	
	return;
}

#define PORT_KEYDAT 0x0060

struct KEYBUF keybuf;

void inthandler21(int *esp)
{
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	
	unsigned char data;			
	io_out8(PIC0_OCW2, 0x61);	//用来通知PIC已经知道发生了IRQ1中断了，IRQ3-0x63 (0x60+IRQ -> OCW) PIC就会时刻监视中断是否发生了
	data = io_in8(PORT_KEYDAT);
	
	/*if(keybuf.flag == 0)         //flag=0表示这个缓冲区为空/ 1=buf有数据
	{
		keybuf.data = data;
		keybuf.flag = 1;
	}
	if(keybuf.next < 32)
	{
		keybuf.data[keybuf.next] = data;
		keybuf.next++;
	}*/
	if(keybuf.len < 32)
	{
		keybuf.data[keybuf.next_w] = data;
		keybuf.len++;
		keybuf.next_w++;
		if(keybuf.len == 32)
		{
			keybuf.next_w = 0;
		}
	}
	return;
	
}

void inthandler2c(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32*8-1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2c (IRQ12): PS/2 mouse");
	for(;;)
	{
		io_hlt();
	}
}

void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67);
	return;
}


/*告诉C编译器，有个函数在别的文件里*/

#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里
#include <stdio.h>      //<>表示该文件位于编译器所提供的文件夹里

extern struct FIFO8 keyfifo,mousefifo;
void init_keyboard(void);
void enable_mouse(void)	;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40],mcursor[256],keybuf[32],mousebuf[128];
	int mx, my;
	int i;
	
	init_gdtidt();
	init_pic();
	io_sti();
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);
	
	init_keyboard();
	
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
	
	enable_mouse();

	for(;;)
	{
		io_cli();  							 //先屏蔽中断，因为此时如果有中断进来会乱，so先去看flag
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
		{
			io_stihlt(); 					//如果键盘和鼠标缓冲区都空，就静静等待
		}
		else{
			if(fifo8_status(&keyfifo) != 0)
			{
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_840084,0,16,15,31);
				putfonts8_asc(binfo->vram,binfo->scrnx, 0, 16, COL8_FFFFFF,s);
			}
			else if(fifo8_status(&mousefifo) != 0)
			{
				i = fifo8_get(&mousefifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_840084,32,16,47,31);
				putfonts8_asc(binfo->vram,binfo->scrnx, 32, 16, COL8_FFFFFF,s);
			}
		}
	}
}

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define	PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void) //让键盘控制电路做好准备，等待控制指令到来（CPU快，要照顾键盘接收数据的能力）
{
	for(;;)
	{
		if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
		{
			break;	//等待键盘控制电路准备完毕 CPU从0x0064读数据倒数第二位，如果不是0就一直循环
		}
	}
	return;
}

void init_keyboard(void)
{
	wait_KBC_sendready(); //确认可否往键盘控制电路传送信息
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); //发送模式设定指令 模式设定指定=0x60，鼠标模式模式号=0x47
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void)		//激活鼠标
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);  //发送指令0xd4,下一个数据就会自动发给鼠标了
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);		//鼠标收到激活指令，准备好了答复CPU 0xfa
	return;
}

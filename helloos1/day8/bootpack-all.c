/*告诉C编译器，有个函数在别的文件里*/

#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里
#include <stdio.h>      //<>表示该文件位于编译器所提供的文件夹里

extern struct FIFO8 keyfifo,mousefifo;
struct MOUSE_DEC {
	unsigned char buf[3],phase;
	int x,y,btn;
};
void init_keyboard(void);
void enable_mouse(struct MOUSE_DEC *mdec)	;
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat); //判断三字节是否齐全

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40],mcursor[256],keybuf[32],mousebuf[128];
	int mx, my;
	int i;
	struct MOUSE_DEC mdec;
	
	init_gdtidt();					//最初状态GDT在asmhead，而不在270000，IDT也没设定，所以要趁硬件上累计过多错误数据之前尽快开放中断接收数据，在调色板和画面初始化之前创建GDT IDT 
	init_pic();						//初始化PIC 开中断
	io_sti();
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);		//开放鼠标中断
	
	init_keyboard();
	
	init_palette();	
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16)/2;
	my = (binfo->scrny - 28-16) /2;
	init_mouse_cursor8(mcursor, COL8_840084);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor,16);
		
	//putfonts8_asc(binfo->vram, binfo->scrnx, 8,8, COL8_000000, "hey guys!");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 7,7, COL8_FFFFFF, "hey guys!"); //神奇的阴影效果！
	//sprintf(s,"mouse:( %3d, %3d )",mx,my);  //初始鼠标在画面中间的坐标
	sprintf(s,"(%3d, %3d)",mx,my);  //初始鼠标在画面中间的坐标
	putfonts8_asc(binfo->vram, binfo->scrnx, 8,31,COL8_FFFFFF, s); //8,0才和设定的重合////////////////////mouse字符串显示的位置[8,31]
	
	enable_mouse(&mdec);
		
	for(;;)
	{
		io_cli();  							 //先屏蔽中断，因为此时如果有中断进来会乱，so先去看flag
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
		{
			io_stihlt(); 					//如果键盘和鼠标缓冲区都空，就静静等待
		}
		else{
			if(fifo8_status(&keyfifo) != 0)		//否则先检查键盘缓冲
			{
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"K:%02X",i);		//key
				boxfill8(binfo->vram,binfo->scrnx,COL8_840084,0,16,23,31); //x0,y0 x1 y1填充背景色8,16,23,31
				putfonts8_asc(binfo->vram,binfo->scrnx, 0, 16, COL8_FFFFFF,s); //8, 16
			}
			else if(fifo8_status(&mousefifo) != 0)   //再检查鼠标缓冲 累计三个字节显示一次，phase=接收鼠标数据进展到什么阶段了
			{
				i = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,i) != 0)
				{
					sprintf(s,"[lcr: %4d %4d]",mdec.x,mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L'; //s的第二个字符换成大写L s[0] = '['
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram,binfo->scrnx,COL8_840084,32,16,32+15*8-1,31);
					putfonts8_asc(binfo->vram,binfo->scrnx, 32, 16, COL8_FFFFFF,s);
					//鼠标的移动
					boxfill8(binfo->vram, binfo->scrnx, COL8_840084, mx,my, mx+15, my+15); //隐藏鼠标
					mx += mdec.x;
					my += mdec.y;
					if(mx < 0)
					{
						mx = 0;
					}
					if(my < 0)
					{
						my = 0;
					}
					if(mx > binfo->scrnx - 16)
					{
						mx = binfo->scrnx - 16;
					}
					if(my > binfo->scrny - 16)
					{
						my = binfo->scrny - 16;
					}
					sprintf(s,"(%3d, %3d)",mx,my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_840084, 8,0,8+10*8-1,15);//隐藏坐标
					putfonts8_asc(binfo->vram, binfo->scrnx, 8, 0, COL8_FFFFFF,s);//显示坐标
					putblock8_8(binfo->vram, binfo->scrnx, 16,16,mx,my,mcursor,16); //描画鼠标
				}
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

void enable_mouse(struct MOUSE_DEC *mdec)		//激活鼠标
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);  //发送指令0xd4,下一个数据就会自动发给鼠标了
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);		//鼠标收到激活指令，准备好了答复CPU 0xfa
	mdec->phase = 0;							//等待0xfa阶段
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if(mdec->phase == 0)
	{
		if(dat == 0xfa)
		{
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1)
	{
		if((dat & 0xc8) == 0x08)		//判断1->移动反应范围  {0，3} 2{8-F} 错误数据会被舍去
		{
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if(mdec->phase == 2)
	{
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if(mdec->phase == 3) //解读处理的核心 鼠标键状态放在buf0低三位
	{
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07; //取出低三位
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if((mdec->buf[0] & 0x10) != 0)
		{
			mdec->x |= 0xffffff00;		//x,y要使用第1字节对鼠标移动有反应的几位，所以第8位及以后置1or0
		}
		if((mdec->buf[0] & 0x20) != 0)
		{
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; 			//取反，因为鼠标与屏幕y方向相反
		return 1;
	}
	return -1;
}

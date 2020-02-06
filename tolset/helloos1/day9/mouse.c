#include "bootpack.h"

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
{
	//struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32*8-1, 15);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2c (IRQ12): PS/2 mouse");
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64); //从PIC先通知IRQ12 OK，再通知主PIC
	io_out8(PIC0_OCW2, 0x62); //通知PIC0 IRQ -02已经受理了
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
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

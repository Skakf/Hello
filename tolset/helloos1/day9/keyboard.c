#include "bootpack.h"

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;			
	io_out8(PIC0_OCW2, 0x61);	//用来通知PIC已经知道发生了IRQ1中断了，IRQ3-0x63 (0x60+IRQ -> OCW) PIC就会时刻监视中断是否发生了
	data = io_in8(PORT_KEYDAT);
	
	fifo8_put(&keyfifo, data);
	return;
}

//#define PORT_KEYDAT				0x0060
//#define	PORT_KEYCMD				0x0064

#define PORT_KEYSTA				0x0064
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



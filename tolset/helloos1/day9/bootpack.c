/*告诉C编译器，有个函数在别的文件里*/

#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里
#include <stdio.h>      //<>表示该文件位于编译器所提供的文件夹里


unsigned int memtest(unsigned int start, unsigned int end);
//unsigned int memtest_sub(unsigned int start, unsigned int end);
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
	enable_mouse(&mdec);
	
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
	
	i = memtest(0x00400000,0xbfffffff)/(1024*1024);
	sprintf(s,"memory: %dMB",i);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 45,COL8_FFFFFF,s);
	
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


#define EFLAGS_AC_BIT		0x0004000
#define CR0_CACHE_DISABLE	0x6000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg,cr0,i;
	
	//确认CPU是386 Or >486
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;		//AC-bit=1 486 EFLAGS reg的第18位=AC标志=1，386会变回0，所以有意识地写入1，再读出看是不是还是1，最后AC置0
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if((eflg & EFLAGS_AC_BIT) != 0)	//如果是386，即使设定AC=1，AC值还会自动回到0
	{
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;		//AC-bit = 0 取反=0xfffbffff
	io_store_eflags(eflg);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |=CR0_CACHE_DISABLE; //禁止缓存
		store_cr0(cr0);
	}
	
	i = memtest_sub(start,end);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; //允许缓存
		store_cr0(cr0);				//load_cr0,store_cr0用汇编写，在nasfunc.asm
	}
	return i;	
}

/*unsigned int memtest_sub(unsigned int start, unsigned int end)//调查从start到end地址范围内能够使用的内存的末尾地址
{
	unsigned int i,*p,old;
	unsigned int pat0 = 0xaa55aa55;
	unsigned int pat1 = 0x55aa55aa;
	for(i = start; i <= end; i+=0x1000)  //i+4因为每次检查4字节 i+0x1000每次4kb
	{
		p = (unsigned int*)(i+0x0ffc);  //检查末尾的4个字节？1111 1111 1100
		old = *p;				//先记住修改前的值
		*p = pat0;				//试写
		*p ^= 0xffffffff;		//反转
		if(*p != pat1)			//检查反转结果   有些机型如果不做反转检查会直接读出写入的数据？？
		{
		not_memory:
				*p = old; 
				break;
		}
		*p ^= 0xffffffff;		//再次反转
		if(*p != pat0)			//检查值是否恢复
		{
			goto not_memory;
		}
		*p = old;				//用old恢复为修改前的值
	}
	return i;
}*/

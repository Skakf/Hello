/*告诉C编译器，有个函数在别的文件里*/

#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里
#include <stdio.h>      //<>表示该文件位于编译器所提供的文件夹里

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40],mcursor[256],keybuf[32],mousebuf[128];
	int mx, my;
	int i;
	struct MOUSE_DEC mdec;
	
	unsigned int memtotal;        //内存管理
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	
	struct SHTCTL *shtctl;
	struct SHEET *sht_back,*sht_mouse;
	unsigned char *buf_back,buf_mouse[256];
	
	init_gdtidt();					//最初状态GDT在asmhead，而不在270000，IDT也没设定，所以要趁硬件上累计过多错误数据之前尽快开放中断接收数据，在调色板和画面初始化之前创建GDT IDT 
	init_pic();						//初始化PIC 开中断
	io_sti();
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);		//开放鼠标中断
	
	init_keyboard();
	enable_mouse(&mdec);
	memtotal= memtest(0x00400000,0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000,0x0009e000);
	memman_free(memman,0x00400000,memtotal - 0x00400000);
	
	init_palette();	
	
	shtctl = shtctl_init(memman,binfo->vram, binfo->scrnx, binfo->scrny);//分配内存空间
	sht_back = sheet_alloc(shtctl);  //找到一个图层分给bg
	sht_mouse = sheet_alloc(shtctl); //找到一个图层分给鼠标
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);//分配一块内存 放图层内容
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //-1 没有透明色 
	sheet_setbuf(sht_mouse, buf_mouse, 16,16,99);//背景色号99
	
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	sheet_slide(shtctl, sht_back,0,0);
	
	//init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16)/2;
	my = (binfo->scrny - 28-16) /2;
	//init_mouse_cursor8(mcursor, COL8_840084);
	sheet_slide(shtctl,sht_mouse,mx,my);
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse,1);
	
    //putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor,16);
	
	
	//putfonts8_asc(binfo->vram, binfo->scrnx, 8,8, COL8_000000, "hey guys!");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 7,7, COL8_FFFFFF, "hey guys!"); //神奇的阴影效果！
	//sprintf(s,"mouse:( %3d, %3d )",mx,my);  //初始鼠标在画面中间的坐标
	sprintf(s,"Mouse:(%3d, %3d)",mx,my);  //初始鼠标在画面中间的坐标
	putfonts8_asc(buf_back, binfo->scrnx, 0,31,COL8_FFFFFF, s); //8,0才和设定的重合////////////////////mouse字符串显示的位置[8,31]

	sprintf(s,"memory: %dMB free : %d KB", memtotal / (1024*1024), memman_total(memman)/1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0,45,COL8_FFFFFF, s);
	sheet_refresh(shtctl,sht_back,0,0,binfo->scrnx,64);  //向下修改鼠标，否则会盖住memory一行

	
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
				sprintf(s,"k:%02X",i);		//key
				boxfill8(buf_back,binfo->scrnx,COL8_840084,0,16,31,31); //x0,y0左上角起始坐标 x1 y1右下坐标 填充背景色 //牛逼欸，扩大了果然就没再重叠显示惹！8,16,23,31  0,16,23,31
				putfonts8_asc(buf_back,binfo->scrnx, 0, 16, COL8_FFFFFF,s); //8, 16 左上角起始坐标
				sheet_refresh(shtctl,sht_back,0,16,32,32);
			}
			else if(fifo8_status(&mousefifo) != 0)   //再检查鼠标缓冲 累计三个字节显示一次，phase=接收鼠标数据进展到什么阶段了
			{
				i = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,i) != 0)
				{
					sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L'; //s的第二个字符换成大写L s[0] = '['
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, COL8_840084,32,16,32+15*8-1,31);
					putfonts8_asc(buf_back, binfo->scrnx, 40, 16, COL8_FFFFFF,s); //32
					sheet_refresh(shtctl,sht_back,32,16,32+15*8,32);
					//鼠标的移动
					//boxfill8(binfo->vram, binfo->scrnx, COL8_840084, mx,my, mx+15, my+15); //隐藏鼠标
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
					if(mx > binfo->scrnx - 1)
					{
						mx = binfo->scrnx - 1;
					}
					if(my > binfo->scrny - 1)
					{
						my = binfo->scrny - 1;
					}
					sprintf(s,"(%3d, %3d)",mx,my);
					boxfill8(buf_back, binfo->scrnx, COL8_840084, 8,0,8+10*8-1,15);//隐藏坐标
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF,s);//显示坐标
					//putblock8_8(buf_back, binfo->scrnx, 16,16,mx,my,mcursor,16); //描画鼠标
					sheet_refresh(shtctl,sht_back,8,0,88,16);
					sheet_slide(shtctl, sht_mouse, mx,my);
				}
			}
		}
	}
}



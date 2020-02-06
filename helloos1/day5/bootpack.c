/*告诉C编译器，有个函数在别的文件里*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);

void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);

void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

#include <stdio.h>

struct BOOTINFO{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

struct SEGMENT_DESCRIPTOR {
	short limit_low,base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};//GDT_8byte seg_size,startaddr,attribute,low3_can't 8192  gdtr:s_addr,num

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};//IDT

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset,int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
	char s[40],mcursor[256];
	int mx, my;
	
	init_palette();	
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	mx = (binfo->scrnx - 16)/2;
	my = (binfo->scrny - 28-16) /2;
	init_mouse_cursor8(mcursor, COL8_840084);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor,16);
	
	putfonts8_asc(binfo->vram, binfo->scrnx, 8,8, COL8_000000, "hey JiLin!");
	putfonts8_asc(binfo->vram, binfo->scrnx, 7,7, COL8_FFFFFF, "hey JiLin!"); //神奇的阴影效果！
	
	//sprintf(s,"scrnx = %d",binfo->scrnx);
	sprintf(s,"mouse:( %d ,%d )",mx,my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 8,31,COL8_FFFFFF, s);
	
	for(;;)
	{
		io_hlt();		/*执行naskfunc里的_io_hlt*/
	}
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3]=              //static char声明一个常数表 == DB
	{
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};
	set_palette(0,15,table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)	//设置调色板
{
	int i,eflags;
	
	eflags = io_load_eflags();				//记录中断许可标志的值
	io_cli();								//标志位置0，禁止中断
	
	io_out8(0x03c8,start);
	for(i = start; i <= end; i++)
	{
		io_out8(0x03c9, rgb[0] / 4);			//io_out cpu 与设备交换信息，读调色板，03c8 3c9 3c7都是固定有意义的
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	
	io_store_eflags(eflags);				//恢复标志位
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x,y;
	for(y = y0; y <= y1; y++)
	{
		for(x = x0; x <= x1; x++)
			vram[y*xsize + x] = c;
	}
	return;
}

void init_screen( char *vram, int x, int y )
{
	boxfill8(vram, x, COL8_840084,  0,     0,      x -  1, y - 27); //840084主界面颜色
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y - 26); //底下小灰条有被分成了好多小部分，比如高亮区，阴影区，所以有黑有白这么多块
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 25, x -  1, y - 25);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 24, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 22, 58,     y - 22);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 22,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 58,     y -  4);
	boxfill8(vram, x, COL8_848484, 58,     y - 21, 58,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 58,     y -  3);
	boxfill8(vram, x, COL8_000000, 59,     y - 22, 59,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 22, x -  4, y - 22);
	boxfill8(vram, x, COL8_848484, x - 47, y - 21, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 22, x -  3, y -  3);
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p,d;
	for(i = 0; i < 16; i++)
	{
		p = vram + (y+i)*xsize +x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for(; *s!= 0x00; s++)
	{
		putfont8(vram, xsize, x,y,c,hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc)		//初始化显示鼠标
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x,y;
	
	for(y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			if(cursor[y][x] == '*')
			{
				mouse[y*16+x] = COL8_000000;
			}
			if (cursor[y][x] == 'O')
			{
				mouse[y*16+x] = COL8_FFFFFF;
			}
			if(cursor[y][x] == '.')
			{
				mouse[y*16+x] = bc;
			}
		}
	}
	return ;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x,y;
	for(y = 0; y < pysize;y++)
	{
		for(x = 0; x < pxsize; x++)
		{
			vram[(py0 + y)*vxsize + (px0 + x)] = buf[y*bxsize + x];
		}
	}
	return;
}

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)0x0026f800;
	int i;
	//GDT init
	for (i = 0; i < 8192; i++)            //8192*8//270000-27ffff
	{
		set_segmdesc(gdt + i,0,0,0);	////上限limit：段的字节数减1，base 基址 access:访问权限 ==0
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);   //对段号1，2进行设定 1：上限值为ffffffff=4GB,地址为0，表示CPU所能管理的全部内存。4092：段属性
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);  //2：上限值512kB，地址为280000，bootpack.hrb
	load_gdtr(0xffff, 0x00270000);    //gdtr赋值
	
	//IDT init
	for(i = 0; i < 256; i++)
	{
		set_gatedesc(idt + i,0,0,0 );
	}
	load_idtr(0x7ff, 0x0026f800);
	
	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff){
		ar |= 0x8000;			//ar = ar|0x8000   或               >>右移
		limit /= 0x1000;		//limit = limit/0x1000 除法
	}
	sd->limit_low = limit & 0xffff;
	sd->base_low = base & 0xffff;
	sd->base_mid = (base>>16)&0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high = ((limit >>16) & 0x0f) | ((ar >> 8)&0xf0);
	sd->base_high = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset,int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar>>8)&0xff;
	gd->access_right = ar & 0xff;
	gd->offset_low = (offset >> 16)&0xfff;
	
	return;
}




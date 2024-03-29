//asmhead.nas
struct BOOTINFO{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};
#define ADR_BOOTINFO 	0x00000ff0    //用到的地址 

//naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

//fifo.c
struct FIFO8
{
	unsigned char *buf;
	int p,q,size,free,flags;
};
//size 缓冲区大小 可变  free 保存缓冲区里没有数据的字节数 buf 缓冲区地址 p = next_w q = next_r

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data); //往缓存区里存储1字节信息 并确认是否溢出
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);		//报告积攒了多少数据

//graphic.c
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);

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

//dsctbl.c
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

#define ADR_IDT		0x0026f800
#define LIMIT_IDT	0x000007ff
#define ADR_GDT		0x00270000
#define LIMIT_GDT	0x0000ffff
#define ADR_BOTPAK  0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e

//int.c  PIC-可编程中断控制器 IMR-中断屏蔽寄存器 8位对8路IRQ信号 某位值=1表示该位被屏蔽
/*struct KEYBUF {
	unsigned char data[32];
	int next_r,next_w,len;
};*/

void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020   //ICW-初始化控制数据 4个 ICW1，4主板配线方式，中断信号电气特性 固定 ICW3有关主从连接设定 00000100 
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021  //决定IRQ以哪一号中断通知CPU
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

//keyboard.c
void init_keyboard(void);
void wait_KBC_sendready(void);
void inthandler21(int *esp);
extern struct FIFO8 keyfifo;
#define PORT_KEYDAT				0x0060
#define	PORT_KEYCMD				0x0064

//mouse.c
struct MOUSE_DEC {
	unsigned char buf[3],phase;
	int x,y,btn;
};
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat); //判断三字节是否齐全
void inthandler2c(int *esp);
extern struct FIFO8 mousefifo;

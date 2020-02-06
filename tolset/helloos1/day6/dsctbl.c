//GDT IDT descriptor table

#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR 	  *idt = (struct GATE_DESCRIPTOR *) ADR_IDT;
	int i;
	//GDT init
	for (i = 0; i <= LIMIT_GDT / 8; i++)            //8192*8//270000-27ffff
	{
		set_segmdesc(gdt + i,0,0,0);	////上限limit：段的字节数减1，base 基址 access:访问权限 ==0
	}
	//set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);   //对段号1，2进行设定 1：上限值为ffffffff=4GB,地址为0，表示CPU所能管理的全部内存。4092：段属性
	//set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);  //2：上限值512kB，地址为280000，bootpack.hrb
	//load_gdtr(0xffff, 0x00270000);    //gdtr赋值
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);   //对段号1，2进行设定 1：上限值为ffffffff=4GB,地址为0，表示CPU所能管理的全部内存。4092：段属性
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);  //2：上限值512kB，地址为280000，bootpack.hrb
	load_gdtr(LIMIT_GDT, ADR_GDT); 
	
	//IDT init
	for(i = 0; i <= LIMIT_IDT / 8; i++)
	{
		set_gatedesc(idt + i,0,0,0 );
	}
	load_idtr(LIMIT_IDT, ADR_IDT);
	
	//idt设定
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);//asm_inthandler21注册到idt里
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);//AR_INTGATE32=008e
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);
	
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
// GDTR赋值 6个字节 低2位段上限（GDT有效字节-1） 高4位 GDT开始的地址
//base 段基址32位：low-2 mid-1 high-1
//limit 段上限-一个段有多少字节-20位 Gbit=1：limit=页 4KB*1M=4GB  -limit_low,limit_high(高4位是段属性)
//ar段属性 12位 (+limit_high4=16)
//ar高4位：扩展访问权 GD00 （D：1-32位模式 0-16位，so 1）
//ar低8位：0x00,0x92-可读写不可执行,0x9a-可执行，可读不可写,//系统专用ring0   //0xf2,0xfa 应用程序模式ring3


void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset,int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar>>8)&0xff;
	gd->access_right = ar & 0xff;
	gd->offset_low = (offset >> 16)&0xffff;
	
	return;
}




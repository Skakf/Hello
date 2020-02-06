#include "bootpack.h"

//size 缓冲区大小 可变  free 保存缓冲区里没有数据的字节数 buf 缓冲区地址 p = next_w q = next_r

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->p = 0;
	fifo->q = 0;
	fifo->flags = 0;
	return;
}

#define FLAGS_OVERRUN 0x0001   //检验溢出

int fifo8_put(struct FIFO8 *fifo, unsigned char data) //往缓存区里存储1字节信息 并确认是否溢出
{
	if(fifo->free == 0)
	{
		fifo->flags |= FLAGS_OVERRUN;
		return -1;    //没有空余了，溢出 指读的追上写的这种情况
	}
	fifo->buf[fifo->p] = data;
	fifo->p ++;
	if(fifo->p == fifo->size)
	{
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
} 

int fifo8_get(struct FIFO8 *fifo)		//从缓冲区取出1字节
{
	int data;
	if(fifo->free == fifo->size)
	{
		return -1;  //缓冲区为空，返回-1
	}
	data = fifo->buf[fifo->q];
	fifo->q ++;
	if(fifo->q == fifo->size)
	{
		fifo->q = 0; //读到头啦从新读
	}
	fifo->free++;
	return data;
}

int fifo8_status(struct FIFO8 *fifo)		//报告积攒了多少数据
{
	return fifo->size - fifo->free;
}

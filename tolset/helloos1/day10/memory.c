#include "bootpack.h"   //“”表示该头文件与源文件位于同一个文件夹里

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



void memman_init (struct MEMMAN *man)
{
	man->frees = 0;			//可用信息数目
	man->maxfrees = 0;			// 观察可用状况，free的最大值
	man->lostsize = 0;			//释放失败的内存大小的总和
	man->losts = 0;				//释放失败的次数
}

unsigned memman_total(struct MEMMAN *man)  //报告空余内存大小的合计
{
	unsigned int i,t = 0;
	for(i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i,a;
	for(i = 0; i < man->frees; i++)
	{
		if(man->free[i].size >= size)
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0)
			{
				man->frees --;    //剪掉一条可用信息
				for(; i<man->frees;i++)
				{
					man->free[i] = man->free[i+1]; //代入结构体
				}
			}
			return a;
		}
	}
	return 0; //没有可用空间
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) //释放内存
{
	int i,j;
	for( i = 0; i < man->frees; i++)
	{
		if(man->free[i].addr > addr)
		{break;}
	}
	if(i > 0)                                                        //刚好可以和前面合并的
	{
		if(man->free[i-1].addr + man->free[i-1].size == addr)		//刚好等于，可以与前合并
		{
			man->free[i-1].size += size;
			if(i < man->frees)
			{
				if(addr + size == man->free[i].addr)					//可以与后合并
				{
					man->free[i-1].size += man->free[i].size;
					man->frees--;                                 //空余数量少一个
					for(; i < man->frees; i++)
					{
						man->free[i] = man->free[i+1];             //结构体赋值，相当于合并之后，原来的i挪到后一个，或者说是下一个空余数组的编号提前了      
					}
				}
			}
			return 0;
		}
	}
	//不能与前面合并的
	if (i < man->frees)
	{
		//可以与后面的归结到一起
		if(addr + size == man->free[i].addr)
		{
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}			
	}
	//既不能和前面合并，又不能和后面合并的
	if(man->frees < MEMMAN_FREES)  //小于管理表的最大值，还能继续往里加
	{
		//free[i]之后向后移动，腾出一点可用空间
		for (j = man->frees; j > i; j--)
		{
			man->free[j]=man->free[j-1];
		}
		man->frees++;
		if(man->maxfrees < man->frees)
		{
			man->maxfrees = man->frees;   //更新最大值
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	//不能往后移动
	man->losts++;
	man->lostsize += size;
	return -1;
}	

unsigned int memman_alloc_4k( struct MEMMAN *man, unsigned int size)  //4k=以1000字节分配
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;  //向上舍入，相当于先加99再向下舍入
	a = memman_alloc(man , size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free_4k(man, addr, size);
	return i;
}






/*告诉C编译器，有个函数在别的文件里*/
void io_hlt(void);

void HariMain(void)
{
	
fin:
		io_hlt();		/*执行naskfunc里的_io_hlt*/
		goto fin;

}
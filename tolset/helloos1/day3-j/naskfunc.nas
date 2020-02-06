;naskfunc
;TAB = 4

[FORMAT "WCOFF"]					;制作目标文件的模式（将输出文件格式设定为WCOFF）
[BITS 32]						;制作32位模式下使用的机器语言

;制作目标文件的信息

[FILE "naskfunc.nas"]			;源文件名信息

		GLOBAL _io_hlt			;程序中包含的函数名（需要链接的函数名要加上global声明，_为了与C语言函数链接）
		
;以下是实际的函数
[SECTION .text]					;目标文件中写了这些之后再写程序

_io_hlt:						;void io_hlt(void);
		HLT
		RET
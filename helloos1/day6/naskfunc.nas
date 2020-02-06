;naskfunc
;TAB = 4

[FORMAT "WCOFF"]					;制作目标文件的模式（将输出文件格式设定为WCOFF）
[INSTRSET "i486p"]				;会把ECX解释为寄存器，而不是普通标号
[BITS 32]						;制作32位模式下使用的机器语言

;制作目标文件的信息

[FILE "naskfunc.nas"]			;源文件名信息

		GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt			;程序中包含的函数名（需要链接的函数名要加上global声明，_为了与C语言函数链接）
		GLOBAL _io_in8, _io_in16, io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL _asm_inthandler21,_asm_inthandler27,_asm_inthandler2c
		EXTERN _inthandler21, _inthandler27, _inthandler2c
		
;以下是实际的函数

[SECTION .text]					;目标文件中写了这些之后再写程序

_io_hlt:						;void io_hlt(void);  CPU暂停工作
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET
		
_io_in8:
		MOV 	EDX,[ESP+4]	;port
		MOV 	EAX,0
		IN		AX,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET
		
_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS 
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET

_load_gdtr:
		MOV		AX,[ESP+4]	;limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET
		
_load_idtr:
		MOV		AX,[ESP+4]
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP 	EAX
		POPAD
		POP 	DS
		POP 	ES
		IRETD
		
_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV     DS,AX
		MOV		ES,AX
		CALL    _inthandler27
		POP		EAX 
		POPAD
		POP 	DS
		POP		ES
		IRETD 
		
_asm_inthandler2c:
		PUSH 	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL 	_inthandler2c
		POP 	EAX 
		POPAD
		POP 	DS
		POP		ES
		IRETD
		
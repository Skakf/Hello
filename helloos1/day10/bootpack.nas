[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_init_gdtidt
	EXTERN	_init_pic
	EXTERN	_io_sti
	EXTERN	_keyfifo
	EXTERN	_fifo8_init
	EXTERN	_mousefifo
	EXTERN	_io_out8
	EXTERN	_init_keyboard
	EXTERN	_enable_mouse
	EXTERN	_memtest
	EXTERN	_memman_init
	EXTERN	_memman_free
	EXTERN	_init_palette
	EXTERN	_shtctl_init
	EXTERN	_sheet_alloc
	EXTERN	_memman_alloc_4k
	EXTERN	_sheet_setbuf
	EXTERN	_init_screen8
	EXTERN	_init_mouse_cursor8
	EXTERN	_sheet_slide
	EXTERN	_sheet_updown
	EXTERN	_sprintf
	EXTERN	_putfonts8_asc
	EXTERN	_memman_total
	EXTERN	_sheet_refresh
	EXTERN	_io_cli
	EXTERN	_fifo8_status
	EXTERN	_fifo8_get
	EXTERN	_mouse_decode
	EXTERN	_boxfill8
	EXTERN	_io_stihlt
[FILE "bootpack.c"]
[SECTION .data]
LC0:
	DB	"Mouse:(%3d, %3d)",0x00
LC1:
	DB	"memory: %dMB free : %d KB",0x00
LC3:
	DB	"[lcr %4d %4d]",0x00
LC4:
	DB	"(%3d, %3d)",0x00
LC2:
	DB	"k:%02X",0x00
[SECTION .text]
	GLOBAL	_HariMain
_HariMain:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	LEA	EBX,DWORD [-748+EBP]
	SUB	ESP,764
	CALL	_init_gdtidt
	CALL	_init_pic
	CALL	_io_sti
	LEA	EAX,DWORD [-348+EBP]
	PUSH	EAX
	PUSH	32
	PUSH	_keyfifo
	CALL	_fifo8_init
	LEA	EAX,DWORD [-476+EBP]
	PUSH	EAX
	PUSH	128
	PUSH	_mousefifo
	CALL	_fifo8_init
	PUSH	249
	PUSH	33
	CALL	_io_out8
	ADD	ESP,32
	PUSH	239
	PUSH	161
	CALL	_io_out8
	CALL	_init_keyboard
	LEA	EAX,DWORD [-492+EBP]
	PUSH	EAX
	CALL	_enable_mouse
	PUSH	-1073741825
	PUSH	4194304
	CALL	_memtest
	PUSH	3932160
	MOV	DWORD [-752+EBP],EAX
	CALL	_memman_init
	PUSH	647168
	PUSH	4096
	PUSH	3932160
	CALL	_memman_free
	MOV	EAX,DWORD [-752+EBP]
	ADD	ESP,36
	SUB	EAX,4194304
	PUSH	EAX
	PUSH	4194304
	PUSH	3932160
	CALL	_memman_free
	CALL	_init_palette
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	PUSH	3932160
	CALL	_shtctl_init
	PUSH	EAX
	MOV	DWORD [-756+EBP],EAX
	CALL	_sheet_alloc
	ADD	ESP,32
	PUSH	DWORD [-756+EBP]
	MOV	DWORD [-760+EBP],EAX
	CALL	_sheet_alloc
	MOVSX	EDX,WORD [4086]
	MOV	DWORD [-764+EBP],EAX
	MOVSX	EAX,WORD [4084]
	IMUL	EAX,EDX
	PUSH	EAX
	PUSH	3932160
	CALL	_memman_alloc_4k
	PUSH	-1
	MOV	DWORD [-768+EBP],EAX
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	PUSH	DWORD [-760+EBP]
	CALL	_sheet_setbuf
	ADD	ESP,32
	PUSH	99
	PUSH	16
	PUSH	16
	PUSH	EBX
	PUSH	DWORD [-764+EBP]
	CALL	_sheet_setbuf
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_init_screen8
	ADD	ESP,32
	PUSH	99
	PUSH	EBX
	MOV	EBX,2
	CALL	_init_mouse_cursor8
	PUSH	0
	PUSH	0
	PUSH	DWORD [-760+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_slide
	MOVSX	EAX,WORD [4084]
	LEA	ECX,DWORD [-16+EAX]
	MOV	EAX,ECX
	CDQ
	IDIV	EBX
	MOV	ESI,EAX
	MOVSX	EAX,WORD [4086]
	LEA	ECX,DWORD [-44+EAX]
	MOV	EAX,ECX
	CDQ
	IDIV	EBX
	PUSH	EAX
	MOV	EDI,EAX
	PUSH	ESI
	LEA	EBX,DWORD [-60+EBP]
	PUSH	DWORD [-764+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_slide
	ADD	ESP,40
	PUSH	0
	PUSH	DWORD [-760+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_updown
	PUSH	1
	PUSH	DWORD [-764+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_updown
	PUSH	EDI
	PUSH	ESI
	PUSH	LC0
	PUSH	EBX
	CALL	_sprintf
	ADD	ESP,40
	PUSH	EBX
	PUSH	7
	PUSH	31
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_putfonts8_asc
	PUSH	3932160
	CALL	_memman_total
	SHR	DWORD [-752+EBP],20
	SHR	EAX,10
	MOV	DWORD [ESP],EAX
	PUSH	DWORD [-752+EBP]
	PUSH	LC1
	PUSH	EBX
	CALL	_sprintf
	ADD	ESP,40
	PUSH	EBX
	PUSH	7
	PUSH	45
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_putfonts8_asc
	PUSH	64
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	0
L18:
	PUSH	0
	PUSH	DWORD [-760+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_refresh
	ADD	ESP,48
L2:
	CALL	_io_cli
	PUSH	_keyfifo
	CALL	_fifo8_status
	PUSH	_mousefifo
	MOV	EBX,EAX
	CALL	_fifo8_status
	LEA	EAX,DWORD [EAX+EBX*1]
	POP	EBX
	POP	EDX
	TEST	EAX,EAX
	JE	L19
	PUSH	_keyfifo
	CALL	_fifo8_status
	POP	ECX
	TEST	EAX,EAX
	JNE	L20
	PUSH	_mousefifo
	CALL	_fifo8_status
	POP	EDX
	TEST	EAX,EAX
	JE	L2
	PUSH	_mousefifo
	CALL	_fifo8_get
	MOV	EBX,EAX
	CALL	_io_sti
	MOVZX	EAX,BL
	PUSH	EAX
	LEA	EAX,DWORD [-492+EBP]
	PUSH	EAX
	CALL	_mouse_decode
	ADD	ESP,12
	TEST	EAX,EAX
	JE	L2
	PUSH	DWORD [-484+EBP]
	PUSH	DWORD [-488+EBP]
	PUSH	LC3
	LEA	EBX,DWORD [-60+EBP]
	PUSH	EBX
	CALL	_sprintf
	ADD	ESP,16
	MOV	EAX,DWORD [-480+EBP]
	TEST	EAX,1
	JE	L11
	MOV	BYTE [-59+EBP],76
L11:
	TEST	EAX,2
	JE	L12
	MOV	BYTE [-57+EBP],82
L12:
	AND	EAX,4
	JE	L13
	MOV	BYTE [-58+EBP],67
L13:
	PUSH	31
	PUSH	151
	PUSH	16
	PUSH	32
	PUSH	13
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_boxfill8
	PUSH	EBX
	PUSH	7
	PUSH	16
	PUSH	40
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_putfonts8_asc
	ADD	ESP,52
	PUSH	32
	PUSH	152
	PUSH	16
	PUSH	32
	PUSH	DWORD [-760+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_refresh
	ADD	ESP,24
	ADD	EDI,DWORD [-484+EBP]
	ADD	ESI,DWORD [-488+EBP]
	JS	L21
L14:
	TEST	EDI,EDI
	JS	L22
L15:
	MOVSX	EAX,WORD [4084]
	DEC	EAX
	CMP	ESI,EAX
	JLE	L16
	MOV	ESI,EAX
L16:
	MOVSX	EAX,WORD [4086]
	DEC	EAX
	CMP	EDI,EAX
	JLE	L17
	MOV	EDI,EAX
L17:
	PUSH	EDI
	PUSH	ESI
	PUSH	LC4
	PUSH	EBX
	CALL	_sprintf
	PUSH	15
	PUSH	87
	PUSH	0
	PUSH	8
	PUSH	13
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_boxfill8
	ADD	ESP,44
	PUSH	EBX
	PUSH	7
	PUSH	0
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_putfonts8_asc
	PUSH	16
	PUSH	88
	PUSH	0
	PUSH	8
	PUSH	DWORD [-760+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_refresh
	ADD	ESP,48
	PUSH	EDI
	PUSH	ESI
	PUSH	DWORD [-764+EBP]
	PUSH	DWORD [-756+EBP]
	CALL	_sheet_slide
	ADD	ESP,16
	JMP	L2
L22:
	XOR	EDI,EDI
	JMP	L15
L21:
	XOR	ESI,ESI
	JMP	L14
L20:
	PUSH	_keyfifo
	CALL	_fifo8_get
	MOV	EBX,EAX
	CALL	_io_sti
	PUSH	EBX
	LEA	EBX,DWORD [-60+EBP]
	PUSH	LC2
	PUSH	EBX
	CALL	_sprintf
	PUSH	31
	PUSH	31
	PUSH	16
	PUSH	0
	PUSH	13
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_boxfill8
	ADD	ESP,44
	PUSH	EBX
	PUSH	7
	PUSH	16
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [-768+EBP]
	CALL	_putfonts8_asc
	PUSH	32
	PUSH	32
	PUSH	16
	JMP	L18
L19:
	CALL	_io_stihlt
	JMP	L2

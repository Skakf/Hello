; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; bootpackのロード先
DSKCAC	EQU		0x00100000		; ディスクキャッシュの場所
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

; BOOT_INFO関係
CYLS	EQU		0x0ff0			; ブートセクタが設定する
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 色数に関する情報。何ビットカラーか？
SCRNX	EQU		0x0ff4			; 解像度のX
SCRNY	EQU		0x0ff6			; 解像度のY
VRAM	EQU		0x0ff8			; グラフィックバッファの開始番地

		ORG		0xc200			; このプログラムがどこに読み込まれるのか

; 画面モードを設定

		MOV		AL,0x13			; VGAグラフィックス、320x200x8bitカラー
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; 画面モードをメモする（C言語が参照する）
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

; キーボードのLED状態をBIOSに教えてもらう

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; PIC??一切中断
;	根据AT兼容机的?格，
;	如果要初始化PIC，必?在CLI前?行，否?有?会挂起
;	，之后再PIC初始化

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 如果???行OUT指令有些机子会无法正常?行？？等同于 ??PIC0/1_IMR 0xff 禁止主从PIC的全部中断
		OUT		0xa1,AL

		CLI						; 禁止CPU??中断

; ?了?CPU能??1MB以上的内存空?，?定A20GATE //程序基本??和init_keyboard相同，往??控制?路?指令

		CALL	waitkbdout      ;等同于wait_KBC_sendready
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20    0xdf?挺重要 ??控制?路的附属端口，?接着主板的很多地方，可?送不同指令??不同功能
		OUT		0x60,AL
		CALL	waitkbdout

; 切?到保?模式

[INSTRSET "i486p"]				; 想要使用486指令（?了使用386以后的lgdt，eax,cr0等）//函数??，?定??GDT，cr0?eax,高位置0低位置1，返回?cr0,完成模式??，?入保?模式

		LGDT	[GDTR0]			; ?定??GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31=0にする（禁止分?）
		OR		EAX,0x00000001	; bit0=1（?了切?到保?模式）
		MOV		CR0,EAX
		JMP		pipelineflush	;切?到保?模式后，机器?言?行流程会?生?化，CPU?了加速指令?行会使用pipeline管道技?，所以需要引入jmp指令
pipelineflush:
		MOV		AX,1*8			;  可?写的段 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack的?送

		MOV		ESI,bootpack	; ?送源
		MOV		EDI,BOTPAK		; ?送目的地
		MOV		ECX,512*1024/4	
		CALL	memcpy			; 从bootpack?始的512KB?制到280000地址去 就是把bootpack.hrb?制到280000地址?理  haribote.sys由asmhead.bin和bootpack.hrb?接生成

; 磁?数据最??送到它本来的位置

; 首先从??扇区?始

		MOV		ESI,0x7c00		; 転送元
		MOV		EDI,DSKCAC		; 転送目的地
		MOV		ECX,512/4		
		CALL	memcpy			; 从7c00?制512字?到DSKCAC（0x00100000）,即将??扇区?制到1MB以后的内存去

; 所有剩下的

		MOV		ESI,DSKCAC0+512	; 転送元
		MOV		EDI,DSKCAC+512	; 転送先
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面数???字?数/4   ?送的数据大小
		SUB		ECX,512/4		; ?去IPL
		CALL	memcpy			; 将始于8200的磁?内容?制到00100200?

; asmhead必?完成的工作已?完成
;	之后交?bootpack

; bootpackの??

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要?送的数据?
		MOV		ESI,[EBX+20]	; 転送元
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 転送目的地  ebx+xx是通?二?制?看??的
		CALL	memcpy			; 解析bootpack.hrb header
skip:
		MOV		ESP,[EBX+12]	; スタック初期値
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		AL,0x60			;空?，?了清空数据接收?冲区中的??数据
		JNZ		waitkbdout		; ANDの結果如果不是0就跳到waitkbdoutへ
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 引き算した結果が0でなければmemcpyへ ?制内存操作
		RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける

		ALIGNB	16				;一直添加DBO直到?机合?（16： 地址能被16整除）
GDT0:
		RESB	8				; NULL selector
		DW		0xffff,0x0000,0x9200,0x00cf	; 可以?写的段 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以?行的段 32bit（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1			;GDTR0是LGDT指令，通知GDT0有GDT?
		DD		GDT0

		ALIGNB	16
bootpack:

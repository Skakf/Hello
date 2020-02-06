;haribo-ipl
;TAB=4 
CYLS	EQU		10
		ORG		0x7c00			;以下程序装到7c00(ORG指明以下的机器语言程序装载到内存的地址)

		JMP 	entry
		DB 		0x90
		
		DB		"HELLOIPL"		; ブートセクタの名前を自由に書いてよい（8バイト）
		DW		512				; 1セクタの大きさ（512にしなければいけない）
		DB		1				; クラスタの大きさ（1セクタにしなければいけない）
		DW		1				; FATがどこから始まるか（普通は1セクタ目からにする）
		DB		2				; FATの個数（2にしなければいけない）
		DW		224				; ルートディレクトリ領域の大きさ（普通は224エントリにする）
		DW		2880			; このドライブの大きさ（2880セクタにしなければいけない）
		DB		0xf0			; メディアのタイプ（0xf0にしなければいけない）
		DW		9				; FAT領域の長さ（9セクタにしなければいけない）
		DW		18				; 1トラックにいくつのセクタがあるか（18にしなければいけない）
		DW		2				; ヘッドの数（2にしなければいけない）
		DD		0				; パーティションを使ってないのでここは必ず0
		DD		2880			; このドライブ大きさをもう一度書く
		DB		0,0,0x29		; よくわからないけどこの値にしておくといいらしい
		DD		0xffffffff		; たぶんボリュームシリアル番号
		DB		"HELLO-OS   "	; ディスクの名前（11バイト）
		DB		"FAT12   "		; フォーマットの名前（8バイト）
		RESB	18				; とりあえず18バイトあけておく
;程序核心

entry:
		MOV		AX,0
		MOV		SS,AX
		MOV 	SP,0x7c00		;
		MOV		DS,AX			;DS 置0

;Day3_New 读磁盘
		MOV 	AX,0x0820		;8000以后的地址随便选的，但8000-81ff留给启动区
		MOV 	ES,AX			;ES:BX缓冲地址：把软盘（下面这些扇区）上读出的数据装载到内存中的位置
		MOV		CH, 0			;柱面0，磁头0，扇区2  （Q：为啥从第二个扇区开始？？A：第一个扇区好像是系统加载时自动装载的扇区）
		MOV 	DH, 0
		MOV		CL, 2
readloop:	
		MOV 	SI,0			;记录失败次数的寄存器
retry:
		MOV  	AH, 0x02		;参数 2：读盘，3：写盘 4：校验，0c：寻道
		MOV		AL, 1			;处理1个扇区
		MOV 	BX,0
		MOV		DL, 0x00		;DL驱动器号0
		INT		0x13			;BIOS_0x13函数
		JNC		next			;没出错就跳转到next
		
		ADD		SI,1
		CMP		SI,5			;5次试错机会
		JAE		error
		MOV 	AH,0x00
		MOV     DX,0x00
		INT		0x13			;出错的话系统复位，重读一次，AH，DX为相应参数
		JMP		retry
next:
		MOV		AX,ES
		ADD     AX,0x0020 		;0x20=512/16,内存地址后移512字节（1个扇区512字节，ES(段):BX  ES+20=BX+512 大概就是一个换算问题）
		MOV		ES,AX
		ADD 	CL,1			;一个一个扇区的读入
		CMP		CL,18
		JBE		readloop
		
		MOV		CL,1
		ADD 	DH,1
		CMP		DH,2
		JB		readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop
		
		MOV		[0x0ff0],CH		; IPLがどこまで読んだのかをメモ
		JMP		0xc200

error:
		MOV		SI,msg			;msg标号堆应的内存地址赋给si

putloop:
		MOV		AL,[SI] 		;[]表示内存地址，si=7c74,就是7c74号内存单元的内容读入al
		ADD		SI,1			;SI+1,内存地址加一位
		CMP		AL,0

		JE		fin
		MOV 	AH,0x0e			;显示一个文字
		MOV		BX,15			;指定字符颜色
		INT		0x10 			;调用显卡BIOS（中断号由具体的参数来实现功能-AH,AL-显示的字符编码，BH-0,BL-colorcode）
		JMP 	putloop
fin:
		HLT						;CPU暂停工作，停止资源消耗
		JMP fin

msg:
	DB		0x0a,0x0a
	DB		"load error"
	DB 		0x0a
	DB		0
;;	
	RESB	0x7dfe-$		; 0x7dfeまでを0x00で埋める命令

	DB		0x55, 0xaa



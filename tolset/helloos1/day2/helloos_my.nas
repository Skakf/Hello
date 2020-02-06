;hello-os
;TAB=4 
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
		MOV		DS,AX			;DS,ES 置0
		MOV 	ES,AX

		MOV		SI,msg			;msg标号堆应的内存地址赋给si？
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
	DB		"hello,world"
	DB 		0x0a
	DB		0
;;	
	RESB	0x7dfe-$		; 0x7dfeまでを0x00で埋める命令

	DB		0x55, 0xaa

; 以下はブートセクタ以外の部分の記述

		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	4600
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	1469432

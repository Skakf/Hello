; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; bootpack�̃��[�h��
DSKCAC	EQU		0x00100000		; �f�B�X�N�L���b�V���̏ꏊ
DSKCAC0	EQU		0x00008000		; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO�֌W
CYLS	EQU		0x0ff0			; �u�[�g�Z�N�^���ݒ肷��
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; �F���Ɋւ�����B���r�b�g�J���[���H
SCRNX	EQU		0x0ff4			; �𑜓x��X
SCRNY	EQU		0x0ff6			; �𑜓x��Y
VRAM	EQU		0x0ff8			; �O���t�B�b�N�o�b�t�@�̊J�n�Ԓn

		ORG		0xc200			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; ��ʃ��[�h��ݒ�

		MOV		AL,0x13			; VGA�O���t�B�b�N�X�A320x200x8bit�J���[
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

; �L�[�{�[�h��LED��Ԃ�BIOS�ɋ����Ă��炤

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; PIC??��ؒ��f
;	����AT���e���I?�i�C
;	�@�ʗv���n��PIC�C�K?��CLI�O?�s�C��?�L?��k�N
;	�C�V�@��PIC���n��

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; �@��???�sOUT�w�ߗL�����q��ٖ@����?�s�H�H������ ??PIC0/1_IMR 0xff �֎~�嘸PIC�I�S�����f
		OUT		0xa1,AL

		CLI						; �֎~CPU??���f

; ?��?CPU�\??1MB�ȏ�I������?�C?��A20GATE //������{??�ainit_keyboard�����C��??�T��?�H?�w��

		CALL	waitkbdout      ;������wait_KBC_sendready
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20    0xdf?���d�v ??�T��?�H�I�����[���C?�ڒ���I�k���n���C��?���s���w��??�s�����\
		OUT		0x60,AL
		CALL	waitkbdout

; ��?����?�͎�

[INSTRSET "i486p"]				; �z�v�g�p486�w�߁i?���g�p386�ȍ@�Ilgdt�Ceax,cr0���j//����??�C?��??GDT�Ccr0?eax,���ʒu0��ʒu1�C�ԉ�?cr0,�����͎�??�C?����?�͎�

		LGDT	[GDTR0]			; ?��??GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31=0�ɂ���i�֎~��?�j
		OR		EAX,0x00000001	; bit0=1�i?����?����?�͎��j
		MOV		CR0,EAX
		JMP		pipelineflush	;��?����?�͎��@�C����?��?�s������?��?���CCPU?�������w��?�s��g�ppipeline�Ǔ��Z?�C���Ȏ��v����jmp�w��
pipelineflush:
		MOV		AX,1*8			;  ��?�ʓI�i 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack�I?��

		MOV		ESI,bootpack	; ?����
		MOV		EDI,BOTPAK		; ?���ړI�n
		MOV		ECX,512*1024/4	
		CALL	memcpy			; ��bootpack?�n�I512KB?����280000�n���� �A���cbootpack.hrb?����280000�n��?��  haribote.sys�Rasmhead.bin�abootpack.hrb?�ڐ���

; ��?������??�������{���I�ʒu

; ��昸??���?�n

		MOV		ESI,0x7c00		; �]����
		MOV		EDI,DSKCAC		; �]���ړI�n
		MOV		ECX,512/4		
		CALL	memcpy			; ��7c00?��512��?��DSKCAC�i0x00100000�j,����??���?����1MB�ȍ@�I������

; ���L�����I

		MOV		ESI,DSKCAC0+512	; �]����
		MOV		EDI,DSKCAC+512	; �]����
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; �����ʐ�???��?��/4   ?���I�����召
		SUB		ECX,512/4		; ?��IPL
		CALL	memcpy			; ���n��8200�I��?���e?����00100200?

; asmhead�K?�����I�H���?����
;	�V�@��?bootpack

; bootpack��??

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; �v�L�v?���I����?
		MOV		ESI,[EBX+20]	; �]����
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; �]���ړI�n  ebx+xx����?��?��?��??�I
		CALL	memcpy			; ���bootpack.hrb header
skip:
		MOV		ESP,[EBX+12]	; �X�^�b�N�����l
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		AL,0x60			;��?�C?�����󐔐��ڝ�?�t�撆�I??����
		JNZ		waitkbdout		; AND�̌��ʔ@�ʕs��0�A����waitkbdout��
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; �����Z�������ʂ�0�łȂ����memcpy�� ?����������
		RET
; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

		ALIGNB	16				;�꒼�Y��DBO����?����?�i16�F �n���\��16�����j
GDT0:
		RESB	8				; NULL selector
		DW		0xffff,0x0000,0x9200,0x00cf	; ��?�ʓI�i 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; ��?�s�I�i 32bit�ibootpack�p�j

		DW		0
GDTR0:
		DW		8*3-1			;GDTR0��LGDT�w�߁C�ʒmGDT0�LGDT?
		DD		GDT0

		ALIGNB	16
bootpack:

; DogOS ipl
; TAB=4

; VBE显卡模式
VBEMODE	equ 0x101
; 0x100 : 640*400*8bit彩色
; 0x101 : 640*480*8bit彩色
; 0x103 : 800*600*8bit彩色
; 0x105 : 1024*768*8bit彩色
; 0x107 : 1280*1024*8bit彩色

; BOOT INFO
CYLS    equ 10			;内核柱面数量
LEDS	equ 0x7e02		;键盘状态字节
VMODE	equ 0x7e03		;画面颜色位数
SCRNX	equ 0x7e04		;分辨率X
SCRNY	equ 0x7e06		;分辨率Y
VRAM	equ 0x7e08		;显存地址

section boot align=16 vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov ss,ax
    mov sp,$$

    ;向CORE位置读取CYLS个柱面(18kB/C)
	mov ax,[CORE+0x00]
	mov dx,[CORE+0x02]
	mov bx,0x10
	div bx
	mov es,ax
	mov bx,dx

    mov dl,0	            ;0:A驱动器
    mov ch,0                ;C
    mov dh,0                ;H
    mov cl,2                ;S
    read_loop:
		mov ah,2            ;read
		mov al,1            ;1
		int 0x13
		mov si,es
		add si,0x20
		mov es,si
		inc cl
		cmp cl,18
		jbe read_loop
		mov cl,1
		inc dh
		cmp dh,2
		jb read_loop
		mov dh,0
		inc ch
		cmp ch,CYLS
		jb read_loop

	;检测高分辨率支持
	mov ax,0x9000
	mov es,ax
	xor di,di
	mov ax,0x4f00
	int 0x10
	cmp ax,0x004f
	jne scrn320		;确认VBE是否存在

	mov ax,[es:di+4]
	cmp ax,0x0200
	jb scrn320		;检查VBE版本
	
	mov cx,VBEMODE
	mov ax,0x4f01
	int 0x10
	cmp ax,0x004f
	jne scrn320		;取得画面模式信息

	cmp byte [es:di+0x19],8
	jne scrn320		;确认颜色数为8
	cmp byte [es:di+0x1b],4
	jne scrn320		;确认调色板模式
	mov ax,[es:di+0x00]
	and ax,0x0080
	jz scrn320		;确认模式号码可加0x4000再进行指定

	;切换VBE画面模式
	mov bx,0x4000+VBEMODE
	mov ax,0x4f02
	int 0x10

	mov byte [VMODE],8
	mov ax,[es:di+0x12]
	mov [SCRNX],ax
	mov ax,[es:di+0x14]
	mov [SCRNY],ax
	mov eax,[es:di+0x28]
	mov [VRAM],eax
	jmp keystatus

    ;默认VGA图形模式，320*200*8bit彩色(VRAM:0x000a0000~0x000af9ff)
scrn320:
    mov al,0x13
    mov ah,0x00
    int 0x10

	mov byte [VMODE],8
	mov word [SCRNX],320
	mov word [SCRNX],200
	mov dword [VRAM],0x000a0000

	;保存键盘状态字节
keystatus:
	mov ah,0x02
	int 0x16
	mov [LEDS],al

    ;Protect mode preparing...
	mov ax,[GDTR+0x02]
	mov dx,[GDTR+0x04]
	mov bx,0x10
	div bx
	mov ds,ax
	mov bx,dx

    ;GDT#0	NULL
	mov dword [bx+0x00],0x00000000
	mov dword [bx+0x04],0x00000000

	;GDT#1	Code(4GB)
	mov dword [bx+0x08],0x0000ffff
	mov dword [bx+0x0c],0x00cf9800

	;GDT#2	Data/Stack(4GB)
	mov dword [bx+0x10],0x0000ffff
	mov dword [bx+0x14],0x00cf9200

	;GDTR
	lgdt [cs:GDTR]

    ;A20
	in al,0x92
	or al,0000_0010b
	out 0x92,al

	cli
	;PE into 32
	mov eax,cr0
	or eax,1
	mov cr0,eax

	;flush pipeline
	jmp dword 0x0008:setup

	[bits 32]
setup:
    mov eax,0x0010	;Data/Stack(4GB)
	mov ds,eax
	mov es,eax
	mov fs,eax
	mov gs,eax
	mov ss,eax
	mov esp,$$

    jmp [CORE]

CORE	dd 0x00040000
GDTR	dw 0xffff
		dd 0x00010000

times 510-($-$$) db 0
db 0x55,0xaa


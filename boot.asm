; DogOS ipl
; TAB=4

    CYLS    equ 10			;柱面数量

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
	mov es,dx
	mov bx,ax

    mov dl,0	            ;0:A驱动器
    mov ah,2                ;read
    mov al,1                ;1
    mov ch,0                ;C
    mov dh,0                ;H
    mov cl,2                ;S
    read_loop:
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

    ;设置显卡模式
    mov al,0x13     		;VGA图形模式，320*200*8位彩色模式，调色板模式
    mov ah,0x00
    int 0x10        		;VRAM:0x000a0000~0x000affff

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
	mov word [cs:GDTR],23
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
GDTR	dw 0x0000
		dd 0x00007e00

times 510-($-$$) db 0
db 0x55,0xaa


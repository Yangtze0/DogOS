; DOG-OS core
; TAB=4

    ENTRY   equ 0x00100000

    [bits 32]
section head align=16 vstart=0x00040000
    ;传送bootpack
    mov esi,dogos
    mov edi,ENTRY
    mov ecx,10*2*18*512/4
    cld
    rep movsd

    jmp dword 0x0008:ENTRY
    
    align 16
dogos:
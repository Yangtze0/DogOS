; DogOS nasm functions
; TAB=4

    GLOBAL  _io_hlt, _io_cli, _io_sti, _io_stihlt
    GLOBAL  _io_in8, _io_in16, _io_in32
    GLOBAL  _io_out8, _io_out16, _io_out32
    GLOBAL  _io_load_eflags, _io_store_eflags
    GLOBAL  _load_idtr, _load_tr
    GLOBAL  _load_cr0, _store_cr0
    GLOBAL  _farjmp, _farcall

    EXTERN  _inthandler20, _inthandler21, _inthandler27, _inthandler2c
    GLOBAL  _asm_inthandler20, _asm_inthandler21
    GLOBAL  _asm_inthandler27, _asm_inthandler2c

    EXTERN  _dogos_api
    GLOBAL  _asm_dogos_api

[bits 32]

_io_hlt:                ;   void io_hlt(void);
    hlt
    ret

_io_cli:                ;   void io_cli(void);
    cli
    ret

_io_sti:                ;   void io_sti(void);
    sti
    ret

_io_in8:                ;   int io_in8(int port);
    mov edx,[esp+4]
    mov eax,0
    in al,dx
    ret

_io_in16:               ;   int io_in16(int port);
    mov edx,[esp+4]
    mov eax,0
    in ax,dx
    ret

_io_in32:               ;   int io_in32(int port);
    mov edx,[esp+4]
    in eax,dx
    ret

_io_out8:               ;   void io_out8(int port, int data);
    mov edx,[esp+4]
    mov al,[esp+8]
    out dx,al
    ret

_io_out16:              ;   void io_out16(int port, int data);
    mov edx,[esp+4]
    mov eax,[esp+8]
    out dx,ax
    ret

_io_out32:              ;   void io_out32(int port, int data);
    mov edx,[esp+4]
    mov eax,[esp+8]
    out dx,eax
    ret

_io_load_eflags:        ;   int io_load_eflags(void);
    pushfd
    pop eax
    ret

_io_store_eflags:       ;   void io_store_eflags(int eflags);
    mov eax,[esp+4]
    push eax
    popfd
    ret

_load_idtr:             ;   void load_idtr(int limit, int addr);
    mov ax,[esp+4]
    mov [esp+6],ax
    lidt [esp+6]
    ret

_load_tr:               ;   void load_tr(int tr);
    ltr [esp+4]
    ret

_load_cr0:              ;   int load_cr0(void);
    mov eax,cr0
    ret

_store_cr0:             ;   void store_cr0(int cr0);
    mov eax,[esp+4]
    mov cr0,eax
    ret

_farjmp:                ;   void farjmp(int eip, int cs);
    jmp far [esp+4]
    ret

_farcall:               ;   void farcall(int eip, int cs);
    call far [esp+4]
    ret

_asm_inthandler20:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    call _inthandler20
    pop eax
    popad
    pop ds
    pop es
    iretd

_asm_inthandler21:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    call _inthandler21
    pop eax
    popad
    pop ds
    pop es
    iretd

_asm_inthandler27:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    call _inthandler27
    pop eax
    popad
    pop ds
    pop es
    iretd

_asm_inthandler2c:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    call _inthandler2c
    pop eax
    popad
    pop ds
    pop es
    iretd

_asm_dogos_api:         ;   int 0x30
    sti
    pushad
    pushad
    call _dogos_api
    add esp,32
    popad
    iretd

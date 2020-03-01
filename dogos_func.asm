; DogOS functions
; TAB=4

    global  _io_hlt

section .text

_io_hlt:    ;   void io_hlt(void);
    hlt
    ret

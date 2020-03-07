////////////////////////////////////////////////////////////////////////////////
/*  DogOS                                                                     */
////////////////////////////////////////////////////////////////////////////////

#include "dogos.h"

void DogOS_main(void) {
    init_idt();
    init_pic();
    io_sti();

    init_palette();
    init_screen();

    putfonts8_asc(8, 8, COL8_000000, "DogOS.");
    putfonts8_asc(30, 30, COL8_FFFFFF, "Hello world!");

    char cursor[256];
    init_mouse_cursor8(cursor, COL8_008484);
    putblock8_8(160, 100, 16, 16, cursor);

    for(;;) {
        io_hlt();
    }
}


////////////////////////////////////////////////////////////////////////////////
//  DogOS
////////////////////////////////////////////////////////////////////////////////

void io_hlt(void);

void DogOS_main(void) {
fin:
    io_hlt();
    goto fin;
}
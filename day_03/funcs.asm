; functions

section .text
    GLOBAL  io_hlt

io_hlt:  ; void io_hlt(void);
    HLT
    RET

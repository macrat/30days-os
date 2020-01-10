; functions

section .text
    GLOBAL  io_in8, io_out8
    GLOBAL  load_gdtr, load_idtr
    GLOBAL  asm_interrupt21, asm_interrupt2c
    GLOBAL  is_available_address

io_in8:  ; int io_in8(int port);
    MOV     EDX, [ESP + 4]
    MOV     EAX, 0
    IN      AL, DX
    RET

io_out8:  ; void io_out8(int port, int data);
    MOV     EDX, [ESP + 4]
    MOV     EAX, [ESP + 8]
    OUT     DX, AL
    RET

load_gdtr:  ; void load_gdtr(int limit, void* addr);
    MOV     AX, [ESP + 4]
    MOV     [ESP + 6], AX
    LGDT    [ESP + 6]
    RET

load_idtr:  ; void load_idtr(int limit, void* addr);
    MOV     AX, [ESP + 4]
    MOV     [ESP + 6], AX
    LIDT    [ESP + 6]
    RET

    EXTERN  keyboard_interrupt
asm_interrupt21:  ; void asm_interrupt21(void);
    PUSH    ES
    PUSH    DS
    PUSHAD
    MOV	    EAX, ESP
    PUSH    EAX
    MOV	    AX, SS
    MOV	    DS, AX
    MOV	    ES, AX
    CALL    keyboard_interrupt
    POP	    EAX
    POPAD
    POP	    DS
    POP	    ES
    IRETD

    EXTERN  mouse_interrupt
asm_interrupt2c:  ; void asm_interrupt2c(void);
    PUSH    ES
    PUSH    DS
    PUSHAD
    MOV	    EAX, ESP
    PUSH    EAX
    MOV	    AX, SS
    MOV	    DS, AX
    MOV	    ES, AX
    CALL    mouse_interrupt
    POP	    EAX
    POPAD
    POP	    DS
    POP	    ES
    IRETD

is_available_address:  ; bool is_available_address(void* address)
    MOV     EAX, [ESP+4]
    MOV     DWORD [EAX], 0xaa55aa55
    XOR     DWORD [EAX], 0xffffffff
    CMP     DWORD [EAX], 0x55aa55aa
    JNE     mem_unavail
    XOR     DWORD [EAX], 0xffffffff
    CMP     DWORD [EAX], 0xaa55aa55
    JNE     mem_unavail
    MOV     EAX, 1  ; return true
    RET
mem_unavail:
    MOV     EAX, 0  ; return false
    RET

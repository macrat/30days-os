; header of hello-os

BOTPAK  EQU 0x00280000
DSKCAC  EQU 0x00100000
DSKCAC0 EQU 0x00008000

CYLS    EQU 0x0ff0
LEDS    EQU 0x0ff1
VMODE   EQU 0x0ff2
SCRNX   EQU 0x0ff4
SCRNY   EQU 0x0ff6
VRAM    EQU 0x0ff8

    ORG     0xc200

; graphics setting
    MOV     AL, 0x13
    MOV     AH, 0x00
    INT     0x10
    MOV     BYTE [VMODE], 8
    MOV     WORD [SCRNX], 320
    MOV     WORD [SCRNY], 200
    MOV     DWORD [VRAM], 0x000a0000

; ger keyboard state
    MOV     AH, 0x02
    INT     0x16
    MOV     [LEDS], AL

; suspend PIC interrupt
    MOV     AL, 0xff
    OUT     0x21, AL
    NOP
    OUT     0xa1, AL
    CLI

; set A20GATE for using memory over 1MB
    CALL    waitkbdout
    MOV     AL, 0xd1
    OUT     0x64, AL
    CALL    waitkbdout
    MOV     AL, 0xdf
    OUT     0x60, AL
    CALL    waitkbdout

; enable protect mode
    LGDT    [GDTR0]
    MOV     EAX, CR0
    AND     EAX, 0x7fffffff
    OR      EAX, 0x00000001
    MOV     CR0, EAX
    JMP     pipelineflush
pipelineflush:
    MOV     AX, 1 * 8
    MOV     DS, AX
    MOV     ES, AX
    MOV     FS, AX
    MOV     GS, AX
    MOV     SS, AX

; transport main program

    MOV     ESI, main
    MOV     EDI, BOTPAK
    MOV     ECX, 512 * 1024 / 4
    CALL    memcpy

; transport disk data

    MOV     ESI, 0x7c00
    MOV     EDI, DSKCAC
    MOV     ECX, 512 / 4
    CALL    memcpy

    MOV     ESI, DSKCAC0 + 512
    MOV     EDI, DSKCAC + 512
    MOV     ECX, 0
    MOV     CL, BYTE [CYLS]
    IMUL    ECX, 512 * 18 * 2 / 4
    SUB     ECX, 512 / 4
    CALL    memcpy

; run main

    MOV     EBX, BOTPAK
    MOV     ECX, [EBX + 16]
    ADD     ECX, 3
    SHR     ECX, 2
    JZ      skip
    MOV     ESI, [EBX + 20]
    ADD     ESI, EBX
    MOV     EDI, [EBX + 12]
    CALL    memcpy
skip:
    MOV     ESP, [EBX + 12]
    JMP     DWORD 2 * 8:0x0000001b

waitkbdout:
    IN      AL, 0x64
    AND     AL, 0x02
    JNZ     waitkbdout
    RET

memcpy:
    MOV     EAX, [ESI]
    ADD     ESI, 4
    MOV     [EDI], EAX
    ADD     EDI, 4
    SUB     ECX, 1
    JNZ     memcpy
    RET

    ALIGNB  16, DB 0
GDT0:
    TIMES   8 DB 0
    DW      0xffff, 0x0000, 0x9200, 0x00cf
    DW      0xffff, 0x0000, 0x9a28, 0x0047
    DW      0
GDTR0:
    DW      8 * 3 - 1
    DD      GDT0

    ALIGNB  16, DB 0
main:

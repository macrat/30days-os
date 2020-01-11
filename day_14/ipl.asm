; IPL for hello-os

CYLS    EQU    10  ; read number of cylinders.

    ORG     0x7c00

; FAT12 on floppy
    JMP     SHORT entry
    DB      0x90
    DB      "HELLOIPL"
    DW      512
    DB      1
    DW      1
    DB      2
    DW      224
    DW      2880
    DB      0xf0
    DW      9
    DW      18
    DW      2
    DD      0
    DD      2880
    DB      0, 0, 0x29
    DD      0xffffffff
    DB      "HELLO-OS   "
    DB      "FAT12   "
    TIMES   18 DB 0

entry:
    MOV     AX, 0
    MOV     SS, AX
    MOV     SP, 0x7c00
    MOV     DS, AX

; read disk
    MOV     AX, 0x0820
    MOV     ES, AX
    MOV     CH, 0
    MOV     DH, 0
    MOV     CL, 2

readloop:
    MOV     DI, 0

retry:
    MOV     AH, 0x02
    MOV     AL, 1
    MOV     BX, 0
    MOV     DL, 0x00
    INT     0x13
    JNC     readloop_next

    ADD     DI, 1
    CMP     DI, 5
    JAE     error
    MOV     AH, 0
    MOV     DL, 0x00
    INT     0x13

    MOV     SI, retrymsg
    CALL    print

    JMP     SHORT retry

readloop_next:
    MOV     AX, ES
    ADD     AX, 0x0020
    MOV     ES, AX
    ADD     CL, 1
    CMP     CL, 18
    JBE     readloop
    MOV     CL, 1
    ADD     DH, 1
    CMP     DH, 2
    JB      readloop
    MOV     DH, 0
    ADD     CH, 1
    CMP     CH, CYLS
    JB      readloop

done:
    MOV		[0x0ff0], CH
    MOV     SI, donemsg
    CALL    print
    JMP     0xc200

error:
    MOV     SI, errmsg
    CALL    print
    JMP     SHORT fin

print:
    MOV     AL, [SI]
    ADD     SI, 1
    CMP     AL, 0
    JNE     print_next
    RET
print_next:
    MOV     AH, 0x0e
    MOV     BX, 15
    INT     0x10
    JMP     SHORT print

fin:
    HLT
    JMP     SHORT fin

errmsg:
    DB      "load error"
    DB      0x0d, 0x0a, 0x00
donemsg:
    DB      "load done"
    DB      0x0d, 0x0a, 0x00
retrymsg:
    DB      "retry..."
    DB      0x0d, 0x0a, 0x00

    TIMES   0x7dfe-0x7c00-($-$$) DB 0
    DB      0x55, 0xaa

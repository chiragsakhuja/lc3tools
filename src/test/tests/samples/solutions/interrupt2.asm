;
; Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
;

; Unfortunately we have not YET installed Windows or Linux on the LC-3,
; so we are going to have to write some operating system code to enable
; keyboard interrupts. The OS code does three things:
;
;    (1) Initializes the interrupt vector table with the starting
;        address of the interrupt service routine. The keyboard
;        interrupt vector is x80 and the interrupt vector table begins
;        at memory location x0100. The keyboard interrupt service routine
;        begins at x1000. Therefore, we must initialize memory location
;        x0180 with the value x1000.
;    (2) Sets bit 14 of the KBSR to enable interrupts.
;    (3) Pushes a PSR and PC to the system stack so that it can jump
;        to the user program at x3000 using an RTI instruction.

        .ORIG x800
        ; (1) Initialize interrupt vector table.
        LD R0, VEC
        LD R1, ISR
        STR R1, R0, #0

        ; (2) Set bit 14 of KBSR.
        LDI R0, KBSR
        LD R1, MASK
        NOT R1, R1
        AND R0, R0, R1
        NOT R1, R1
        ADD R0, R0, R1
        STI R0, KBSR

        ; (3) Set up system stack to enter user space.
        LD R0, PSR
        ADD R6, R6, #-1
        STR R0, R6, #0
        LD R0, PC
        ADD R6, R6, #-1
        STR R0, R6, #0
        ; Enter user space.
        RTI

VEC     .FILL x0180
ISR     .FILL x1000
KBSR    .FILL xFE00
MASK    .FILL x4000
PSR     .FILL x8002
PC      .FILL x3000
        .END

        .ORIG x3000
        ; *** Begin user program code here ***
        AND R2, R2, #0
SETUP   ADD R2, R2, xD
        LEA R1, PLINE
L1      LDR R0, R1, #0
        PUTS
        ADD R1, R1, #1
        ADD R2, R2, #-1
        BRz SETUP
        BR L1

PLINE   .FILL LINE0
        .FILL LINE1
        .FILL LINE2
        .FILL LINE3
        .FILL LINE4
        .FILL LINE5
        .FILL LINE6
        .FILL LINE7
        .FILL LINE8
        .FILL LINE9
        .FILL LINEA
        .FILL LINEB
        .FILL LINEC
LINE0   .STRINGZ "====================\n"
LINE1   .STRINGZ "*    *  *******\n"
LINE2   .STRINGZ "*    *     *\n"
LINE3   .STRINGZ "*    *     *\n"
LINE4   .STRINGZ "*    *     *\n"
LINE5   .STRINGZ " ****      *\n"
LINE6   .STRINGZ "\n"
LINE7   .STRINGZ "****   ****  ****\n"
LINE8   .STRINGZ "*     *      *\n"
LINE9   .STRINGZ "****  *      ****\n"
LINEA   .STRINGZ "*     *      *\n"
LINEB   .STRINGZ "****   ****  ****\n"
LINEC   .STRINGZ "====================\n"
        ; *** End user program code here ***
        .END

        .ORIG x1000
        ; *** Begin interrupt service routine code here ***
        STR R0, R6, #-1
        STR R1, R6, #-2
        STR R2, R6, #-3
        STR R3, R6, #-4
        STR R4, R6, #-5
        STR R5, R6, #-6
        STR R7, R6, #-7
        ADD R6, R6, #-7

        LDI R0, KBDR
        LD R1, LO
        ADD R1, R0, R1
        BRn SKIP
        LD R1, HI
        ADD R1, R0, R1
        BRp SKIP

        LD R1, LO
        ADD R1, R0, R1
L2      BRz L2EXIT
        OUT
        ADD R1, R1, #-1
        BR L2
L2EXIT  LD R0, NEWLINE
        OUT
        BR EXIT

SKIP    OUT
        LEA R0, SKIPMSG
        PUTS

EXIT    ADD R6, R6, #7
        LDR R7, R6, #-7
        LDR R5, R6, #-6
        LDR R4, R6, #-5
        LDR R3, R6, #-4
        LDR R2, R6, #-3
        LDR R1, R6, #-2
        LDR R0, R6, #-1
        RTI

KBDR    .FILL xFE02
LO      .FILL x-30
HI      .FILL x-39
SKIPMSG .STRINGZ " is not a decimal digit.\n"
NEWLINE .FILL xA
        ; *** End interrupt service routine code here ***
        .END

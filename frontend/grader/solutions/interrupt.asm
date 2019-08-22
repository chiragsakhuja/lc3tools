;
; Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
;
.orig x3000
        trap x30
        lea r0, mesg
main_loop
        puts
        ld r1, delay
delay_loop
        add r1, r1, #-1
        brp delay_loop
        br main_loop
        halt

mesg    .stringz "Input a capital letter from the English alphabet: \n"
delay   .fill #30000
.end

; fill TRAP table
.orig x30
        .fill x2000
.end

; fill interrupt table
.orig x180
        .fill x1500
.end

; interrupt service routine
.orig x1500
        str r0, r6, #-1
        str r1, r6, #-2
        str r2, r6, #-3
        add r6, r6, #-3

        ldi r0, kbdr
        ld r1, NegA
        add r2, r0, r1
        brn not_caps
        ld r1, NegZ
        add r2, r0, r1
        brp not_caps
        ld r2, lower
        add r1, r0, #0
        add r2, r0, r2
        lea r0, msg1
        puts
        add r0, r1, #0
        out
        lea r0, msg2
        puts
        add r0, r2, #0
        out
        br isr_exit

not_caps
        out
        lea r0, not_msg
        puts

isr_exit
        add r6, r6, #3
        ldr r2, r6, #-3
        ldr r1, r6, #-2
        ldr r0, r6, #-1
        rti
not_msg .stringz " is not a capital letter of the English alphabet.\n"
msg1    .stringz "The lower case of "
msg2    .stringz " is "
lower   .fill x20
kbdr    .fill xfe02
NegA    .fill #-65
NegZ    .fill #-90
.end

; TRAP routine to enable keyboard interrupts
.orig x2000
        str r0, r6, #-1
        str r1, r6, #-2

        ldi r0, kbsr
        ld r1, ie_enable
        add r0, r0, r1
        sti r0, kbsr

        ldr r1, r6, #-2
        ldr r0, r6, #-1
        rti
kbsr    .fill xfe00
ie_enable
        .fill x4000
.end

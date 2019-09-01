;
; Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
;
.orig x3000
          trap x30
          lea r0, Msg
MainLoop  puts
          ld r1, Delay
DelayLoop add r1, r1, #-1
          brp DelayLoop
          br MainLoop
          halt

Msg       .stringz "Input a capital letter from the English alphabet: \n"
Delay     .fill #30000
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

          ldi r0, KBDR
          ld r1, NegA
          add r2, r0, r1
          brn NotCaps
          ld r1, NegZ
          add r2, r0, r1
          brp NotCaps
          ld r2, Lower
          add r1, r0, #0
          add r2, r0, r2
          lea r0, Msg1
          puts
          add r0, r1, #0
          out
          lea r0, Msg2
          puts
          add r0, r2, #0
          out
          br ISRExit

NotCaps   out
          lea r0, NotMsg
          puts

ISRExit   add r6, r6, #3
          ldr r2, r6, #-3
          ldr r1, r6, #-2
          ldr r0, r6, #-1
          rti
NotMsg    .stringz " is not a capital letter of the English alphabet.\n"
Msg1      .stringz "The lower case of "
Msg2      .stringz " is "
Lower     .fill x20
KBDR      .fill xfe02
NegA      .fill #-65
NegZ      .fill #-90
.end

; TRAP routine to enable keyboard interrupts
.orig x2000
          str r0, r6, #-1
          str r1, r6, #-2

          ldi r0, kbsr
          ld r1, IEEnable
          add r0, r0, r1
          sti r0, kbsr

          ldr r1, r6, #-2
          ldr r0, r6, #-1
          rti
KBSR      .fill xfe00
IEEnable  .fill x4000
.end

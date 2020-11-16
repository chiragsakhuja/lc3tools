;
; Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
;
.orig x3000
               ld r6, Stack
               ld r5, Table
               ldr r3, r5, #1
               ldr r4, r5, #2
               ld r5, Compute
    
               ; compute f(lo) and f(hi)
MainLoop       add r1, r3, #0
               jsrr r5
               st r0, LoVal
               add r1, r4, #0
               jsrr r5
               st r0, HiVal

               ; compute midpoint
               add r1, r3, r4
               and r2, r2, #0
               add r2, r2, #1
               jsr RSHFA

               ; compute value at midpoint
               add r1, r0, #0
               jsrr r5

               ; at this point: r0 <- f(mid), r1 <- mid, r3 <- lo, r4 <- hi
               add r0, r0, #0
               brz MainLoopExit

               brn MainLoopMidNeg
               ld r2, LoVal
               brn MainSetHi
               br MainSetLo

MainLoopMidNeg ld r2, LoVal
               brn MainSetLo
               br MainSetHi

MainSetLo      add r3, r1, #0
               br MainLoop
MainSetHi      add r4, r1, #0
               br MainLoop

MainLoopExit   sti r1, Table
               halt

LoVal          .fill #0
HiVal          .fill #0

Compute        .fill x5000
Stack          .fill x8000
Table          .fill x4000

; right shift r1 by r2 and store result in r0
RSHFA          str r2, r6, #-1
               str r3, r6, #-2
               str r4, r6, #-3
               add r6, r6, #-3

               and r3, r3, #0
               add r3, r3, #1    ; set r3 to 1
               add r4, r2, #0    ; copy r2 into r4 (for use later)
               brz RSHFALoop
; set r3 to 1 << r1
RSHFAMask1     add r3, r3, r3
               add r2, r2, #-1
               brp RSHFAMask1

               not r4, r4
               add r4, r4, #1
               add r4, r4, #15
               add r4, r4, #1
               and r2, r2, #0
               add r2, r2, #1
               and r0, r0, #0
RSHFAMask2     add r2, r2, r2
               add r4, r4, #-1
               brp RSHFAMask2
               add r2, r2, #-1
               not r2, r2
               add r1, r1, #0
               brzp RSHFABeginLoop
               add r0, r0, r2

RSHFABeginLoop and r4, r4, #0
               add r4, r4, #1    ; set r4 to 1

; mask a bit in r0 and add to r4 if it is a 1
RSHFALoop      and r2, r1, r3
               brz RSFHANoSet
               add r0, r0, r4
RSFHANoSet     add r4, r4, r4
               add r3, r3, r3
               brnp RSHFALoop

               add r6, r6, #3
               ldr r4, r6, #-3
               ldr r3, r6, #-2
               ldr r2, r6, #-1
               ret
.end

.orig x5000
; compute the polynomial
; x is given in r1 and the result is given in r0
            str r1, r6, #-1
            str r2, r6, #-2
            str r3, r6, #-3
            str r4, r6, #-4
            str r5, r6, #-5
            str r7, r6, #-6
            add r6, r6, #-6

            ld r5, PolyTable
            ldr r4, r5, #0
            ldr r2, r5, #1
            jsr Multiply
            add r2, r0, #0
ComputeLoop add r5, r5, #1
            ldr r0, r5, #1
            add r2, r2, r0
            add r4, r4, #-1
            brnz ComputeExit
            jsr Multiply
            add r2, r0, #0
            br ComputeLoop

ComputeExit add r0, r2, #0

            add r6, r6, #6
            ldr r7, r6, #-6
            ldr r5, r6, #-5
            ldr r4, r6, #-4
            ldr r3, r6, #-3
            ldr r2, r6, #-2
            ldr r1, r6, #-1
            ret

PolyTable .fill x4003

; multiply r1 and r2, storing result in r0
Multiply          str r1, r6, #-1
                  str r2, r6, #-2
                  str r3, r6, #-3
                  add r6, r6, #-3

                  and r3, r3, #0
                  st r3, Negate

                  add r1, r1,#0
                  brzp MultiplyCheckOp2
                  not r1, r1
                  add r1, r1, #1
                  ld r0, Negate
                  not r0, r0
                  st r0, Negate
MultiplyCheckOp2  add r2, r2, #0
                  brzp MultiplyBeginLoop
                  not r2, r2
                  add r2, r2, #1
                  ld r0, Negate
                  not r0, r0
                  st r0, Negate

MultiplyBeginLoop and r0, r0, #0
                  add r2, r2, #0
                  brz MultiplyDone
MultiplyLoop      add r0, r0, r1
                  add r2, r2, #-1
                  brp MultiplyLoop

MultiplyDone      ld r1, Negate
                  brz MultiplyExit
                  not r0, r0
                  add r0, r0, #1

MultiplyExit      add r6, r6, #3
                  ldr r3, r6, #-3
                  ldr r2, r6, #-2
                  ldr r1, r6, #-1
                  ret

Negate .fill #0

.end

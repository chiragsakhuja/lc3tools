;
; Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
;
.orig x3000
        ld r6, stack
        ld r5, table
        ldr r3, r5, #1
        ldr r4, r5, #2
        ld r5, compute

main_loop
        ; compute f(lo) and f(hi)
        add r1, r3, #0
        jsrr r5
        st r0, lo_val
        add r1, r4, #0
        jsrr r5
        st r0, hi_val

        ; compute midpoint
        add r1, r3, r4
        and r2, r2, #0
        add r2, r2, #1
        jsr rshfa

        ; compute value at midpoint
        add r1, r0, #0
        jsrr r5

        ; at this point: r0 <- f(mid), r1 <- mid, r3 <- lo, r4 <- hi
        add r0, r0, #0
        brz main_loop_exit

        brn main_loop_mid_neg
        ld r2, lo_val
        brn main_set_hi
        br main_set_lo
main_loop_mid_neg
        ld r2, lo_val
        brn main_set_lo
        br main_set_hi

main_set_lo
        add r3, r1, #0
        br main_loop
main_set_hi
        add r4, r1, #0
        br main_loop

main_loop_exit
        sti r1, table
        halt

lo_val  .fill #0
hi_val  .fill #0

compute .fill x5000
stack   .fill x8000
table   .fill x4000

; right shift r1 by r2 and store result in r0
rshfa
        str r2, r6, #-1
        str r3, r6, #-2
        str r4, r6, #-3
        add r6, r6, #-3

        and r3, r3, #0
        add r3, r3, #1    ; set r3 to 1
        add r4, r2, #0    ; copy r2 into r4 (for use later)
        brz rshfa_loop
; set r3 to 1 << r1
rshfa_mask1
        add r3, r3, r3
        add r2, r2, #-1
        brp rshfa_mask1

        not r4, r4
        add r4, r4, #1
        add r4, r4, #15
        add r4, r4, #1
        and r2, r2, #0
        add r2, r2, #1
        and r0, r0, #0
rshfa_mask2
        add r2, r2, r2
        add r4, r4, #-1
        brp rshfa_mask2
        add r2, r2, #-1
        not r2, r2
        add r1, r1, #0
        brzp rshfa_begin_loop
        add r0, r0, r2

rshfa_begin_loop
        and r4, r4, #0
        add r4, r4, #1    ; set r4 to 1

; mask a bit in r0 and add to r4 if it is a 1
rshfa_loop
        and r2, r1, r3
        brz rshfa_noset
        add r0, r0, r4
rshfa_noset
        add r4, r4, r4
        add r3, r3, r3
        brnp rshfa_loop

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

        ld r5, poly_table
        ldr r4, r5, #0
        ldr r2, r5, #1
        jsr multiply
        add r2, r0, #0
compute_loop
        add r5, r5, #1
        ldr r0, r5, #1
        add r2, r2, r0
        add r4, r4, #-1
        brnz compute_exit
        jsr multiply
        add r2, r0, #0
        br compute_loop

compute_exit
        add r0, r2, #0

        add r6, r6, #6
        ldr r7, r6, #-6
        ldr r5, r6, #-5
        ldr r4, r6, #-4
        ldr r3, r6, #-3
        ldr r2, r6, #-2
        ldr r1, r6, #-1
        ret
poly_table
        .fill x4003

; multiply r1 and r2, storing result in r0
multiply
        str r1, r6, #-1
        str r2, r6, #-2
        str r3, r6, #-3
        add r6, r6, #-3

        and r3, r3, #0
        st r3, negate

        add r1, r1,#0
        brzp multiply_check_op2
        not r1, r1
        add r1, r1, #1
        ld r0, negate
        not r0, r0
        st r0, negate
multiply_check_op2
        add r2, r2, #0
        brzp multiply_begin_loop
        not r2, r2
        add r2, r2, #1
        ld r0, negate
        not r0, r0
        st r0, negate

multiply_begin_loop
        and r0, r0, #0
        add r2, r2, #0
        brz multiply_done
multiply_loop
        add r0, r0, r1
        add r2, r2, #-1
        brp multiply_loop

multiply_done
        ld r1, negate
        brz multiply_exit
        not r0, r0
        add r0, r0, #1

multiply_exit
        add r6, r6, #3
        ldr r3, r6, #-3
        ldr r2, r6, #-2
        ldr r1, r6, #-1
        ret

negate .fill #0

.end

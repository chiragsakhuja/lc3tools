.orig x3000

Main
        ; set up stack
        ld r6, stack
main_loop
        ; get name
        jsr GetName
        ; check if name is 'd' and terminate if so
        lea r0, name
        lea r1, endstr
        jsr Compare
        add r5, r5, #0
        brp done
        ; search tree
        ldi r0, root
        jsr Search
        add r0, r4, #0
        brz not_found
        ; if found, print out value
        jsr Dec2ASCII
        ld r0, enter
        out
        br main_loop
not_found
        ; if not found, print out "No entry"
        lea r0, nfound
        puts
        br main_loop
done    halt

stack  .fill x8000
root   .fill x4000
name   .blkw #21
enter  .fill #10
endstr .stringz "d"
nfound .stringz "No Entry\n"

; gets a name as input and places it in the "global variable" name
GetName
        str r0, r6, #-1
        str r1, r6, #-2
        str r2, r6, #-3
        str r3, r6, #-4
        str r7, r6, #-8
        ; init
        lea r1, name
        ld r2, nenter
        lea r0, prompt
        puts
in_loop
        ; get input until enter
        getc
        out
        add r3, r0, r2
        brz in_loop_done
        str r0, r1, #0
        add r1, r1, #1
        br in_loop
in_loop_done
        ; store null terminator
        and r0, r0, #0
        str r0, r1, #0
        ; done
        ldr r7, r6, #-8
        ldr r3, r6, #-4
        ldr r2, r6, #-3
        ldr r1, r6, #-2
        ldr r0, r6, #-1
        ret

prompt .stringz "Type a professor's name and then press enter:"

; searches a tree for the name stored in "global variable" name
; r0 is a pointer to root of the tree
; r4 is a pointer to professor's salary or 0 if professor was not found
Search
        str r0, r6, #-1
        str r1, r6, #-2
        str r2, r6, #-3
        str r5, r6, #-6
        str r7, r6, #-8
        add r6, r6, #-8
        ; clear return value
        and r4, r4, #0
        ; check if root is 0 (i.e. no leaf)
        add r2, r0, #0
        brz search_done
        ; compare strings
        lea r0, name
        ldr r1, r2, #2
        jsr Compare
        add r5, r5, #0
        brnp search_found
        ; recurse left node
        ldr r0, r2, #0
        jsr Search
        add r4, r4, #0
        brnp search_done
        ; recurse right node
        ldr r0, r2, #1
        jsr Search
        br search_done
search_found
        ; set return value if found
        ldr r4, r2, #3
search_done
        ; done
        add r6, r6, #8
        ldr r7, r6, #-8
        ldr r5, r6, #-6
        ldr r2, r6, #-3
        ldr r1, r6, #-2
        ldr r0, r6, #-1
        ret

message .stringz "Found"

; compares two strings
; r0 and r1 are pointers to the first character in each string to be compared
; r5 will contain 0 if the strings are not equal and 1 if they are
Compare
        str r0, r6, #-1
        str r1, r6, #-2
        str r2, r6, #-3
        str r3, r6, #-4
        str r7, r6, #-8
        and r5, r5, #0
comp_loop
        ldr r2, r0, #0
        ldr r3, r1, #0
        not r2, r2
        add r2, r2, #1
        add r2, r2, r3
        brnp comp_loop_done
        add r0, r0, #1
        add r1, r1, #1
        add r3, r3, #0
        brnp comp_loop
        add r5, r5, #1
comp_loop_done
        ldr r7, r6, #-8
        ldr r3, r6, #-4
        ldr r2, r6, #-3
        ldr r1, r6, #-2
        ldr r0, r6, #-1
        ret

nenter .fill #-10

; prints an unsigned decimal number (max 2^15-1)
; r0 is a pointer to the location with the number
Dec2ASCII
        str r0, r6, #-1
        str r1, r6, #-2
        str r2, r6, #-3
        str r3, r6, #-4
        str r4, r6, #-5
        str r5, r6, #-6
        str r7, r6, #-8
        ld r4, ascii0
        ; print 10000s place
        ld r1, num10k
        jsr UIntDiv
        add r0, r2, r4
        out
        ; print 1000s place
        add r0, r3, #0
        ld r1, num1k
        jsr UIntDiv
        add r0, r2, r4
        out
        ; print 100s place
        add r0, r3, #0
        ld r1, num100
        jsr UIntDiv
        add r0, r2, r4
        out
        ; print 10s place
        add r0, r3, #0
        ld r1, num10
        jsr UIntDiv
        add r0, r2, r4
        out
        ; print 1s place
        add r0, r3, r4
        out
        ; end
        ldr r7, r6, #-8
        ldr r5, r6, #-6
        ldr r4, r6, #-5
        ldr r3, r6, #-4
        ldr r2, r6, #-3
        ldr r1, r6, #-2
        ldr r0, r6, #-1
        ret

num10k  .fill #10000
num1k   .fill #1000
num100  .fill #100
num10   .fill #10
ascii0  .fill x30

; integer divides r0 by r1
; r0 and r1 are operands
; r2 contains quotient and r3 contains remainder
UIntDiv
        and r2, r2, #0
        add r3, r0, #0
        ; invert r1
        not r1, r1
        add r1, r1, #1
        ; check if done
uintdiv_loop
        add r3, r3, r1
        brn uintdiv_done
        add r2, r2, #1
        br uintdiv_loop
uintdiv_done
        ; correct remainder
        not r1, r1
        add r1, r1, #1
        add r3, r3, r1
        ret

.end

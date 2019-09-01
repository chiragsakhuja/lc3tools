; Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
.orig x3000

         ; set up stack
Main     ld r6, Stack
         ; get name
MainLoop jsr GetName
         ; check if name is 'd' and terminate if so
         lea r0, Name
         lea r1, Endstr
         jsr Compare
         add r5, r5, #0
         brp Done
         ; search tree
         ldi r0, Root
         jsr Search
         add r0, r4, #0
         brz NotFound
         ; if found, print out value
         jsr Dec2ASCII
         ld r0, Enter
         out
         br MainLoop
         ; if not found, print out "No entry"
NotFound lea r0, NFound
         puts
         br MainLoop
Done     halt

Stack   .fill x8000
Root    .fill x4000
Name    .blkw #21
Enter   .fill #10
Endstr  .stringz "d"
NFound  .stringz "No Entry\n"

; gets a name as input and places it in the "global variable" name
GetName    str r0, r6, #-1
           str r1, r6, #-2
           str r2, r6, #-3
           str r3, r6, #-4
           str r7, r6, #-8
           ; init
           lea r1, Name
           ld r2, NEnter
           lea r0, Prompt
           puts
           ; get input until enter
InLoop      getc
           out
           add r3, r0, r2
           brz InLoopDone
           str r0, r1, #0
           add r1, r1, #1
           br InLoop
           ; store null terminator
InLoopDone and r0, r0, #0
           str r0, r1, #0
           ; done
           ldr r7, r6, #-8
           ldr r3, r6, #-4
           ldr r2, r6, #-3
           ldr r1, r6, #-2
           ldr r0, r6, #-1
           ret

Prompt .stringz "Type a professor's name and then press enter:"
NEnter .fill #-10

; searches a tree for the name stored in "global variable" name
; r0 is a pointer to root of the tree
; r4 is a pointer to professor's salary or 0 if professor was not found
Search      str r0, r6, #-1
            str r1, r6, #-2
            str r2, r6, #-3
            str r5, r6, #-6
            str r7, r6, #-8
            add r6, r6, #-8
            ; clear return value
            and r4, r4, #0
            ; check if root is 0 (i.e. no leaf)
            add r2, r0, #0
            brz SearchDone
            ; compare strings
            lea r0, Name
            ldr r1, r2, #2
            jsr Compare
            add r5, r5, #0
            brnp SearchFound
            ; recurse left node
            ldr r0, r2, #0
            jsr Search
            add r4, r4, #0
            brnp SearchDone
            ; recurse right node
            ldr r0, r2, #1
            jsr Search
            br SearchDone
            ; set return value if found
SearchFound ldr r4, r2, #3
            ; done
SearchDone  add r6, r6, #8
            ldr r7, r6, #-8
            ldr r5, r6, #-6
            ldr r2, r6, #-3
            ldr r1, r6, #-2
            ldr r0, r6, #-1
            ret

; compares two strings
; r0 and r1 are pointers to the first character in each string to be compared
; r5 will contain 0 if the strings are not equal and 1 if they are
Compare      str r0, r6, #-1
             str r1, r6, #-2
             str r2, r6, #-3
             str r3, r6, #-4
             str r7, r6, #-8
             and r5, r5, #0
CompLoop     ldr r2, r0, #0
             ldr r3, r1, #0
             not r2, r2
             add r2, r2, #1
             add r2, r2, r3
             brnp CompLoopDone
             add r0, r0, #1
             add r1, r1, #1
             add r3, r3, #0
             brnp CompLoop
             add r5, r5, #1
CompLoopDone ldr r7, r6, #-8
             ldr r3, r6, #-4
             ldr r2, r6, #-3
             ldr r1, r6, #-2
             ldr r0, r6, #-1
             ret

; prints an unsigned decimal number (max 2^15-1)
; r0 is a pointer to the location with the number
Dec2ASCII str r0, r6, #-1
          str r1, r6, #-2
          str r2, r6, #-3
          str r3, r6, #-4
          str r4, r6, #-5
          str r5, r6, #-6
          str r7, r6, #-8
          ld r4, ASCII0
          ; print 10000s place
          ld r1, Num10k
          jsr UIntDiv
          add r0, r2, r4
          out
          ; print 1000s place
          add r0, r3, #0
          ld r1, Num1k
          jsr UIntDiv
          add r0, r2, r4
          out
          ; print 100s place
          add r0, r3, #0
          ld r1, Num100
          jsr UIntDiv
          add r0, r2, r4
          out
          ; print 10s place
          add r0, r3, #0
          ld r1, Num10
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

Num10k  .fill #10000
Num1k   .fill #1000
Num100  .fill #100
Num10   .fill #10
ASCII0  .fill x30

; integer divides r0 by r1
; r0 and r1 are operands
; r2 contains quotient and r3 contains remainder
UIntDiv     and r2, r2, #0
            add r3, r0, #0
            ; invert r1
            not r1, r1
            add r1, r1, #1
            ; check if done
UIntDivLoop add r3, r3, r1
            brn UIntDivDone
            add r2, r2, #1
            br UIntDivLoop
            ; correct remainder
UIntDivDone not r1, r1
            add r1, r1, #1
            add r3, r3, r1
            ret

.end

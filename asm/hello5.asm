%include "stud_io.inc"
global _main
section .text
_main: mov eax, 0
again: PRINT "Hello"
PUTCHAR 10
inc eax
cmp eax, 5
jl again
FINISH
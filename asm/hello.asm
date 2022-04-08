          global    _main

          section   .data
message:  db        "Hello, World", 10      ; note the newline at the end

          section   .text
_main:    mov       eax, 1
            mov ah, 2
            add eax, ah
          mov       rax, 0x02000001         ; system call for exit
          xor       rdi, rdi                ; exit code 0
          syscall                           ; invoke operating system to exit


          mov       rax, 0x02000004         ; system call for write
          mov       rdi, 1                  ; file handle 1 is stdout
          mov       rsi, message            ; address of string to output
          mov       rdx, 13                 ; number of bytes
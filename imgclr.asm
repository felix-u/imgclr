format PE64 console
entry start

include 'include/win64ax.inc'

section '.data' data readable
msg db 'Hello, world!',0
msg_len equ $ - msg

section '.text' code readable executable
start:
    cinvoke printf, "Hello, world!%c",10

    mov rax, 0
    ret

section '.idata' import data readable writeable

library \
    kernel32, 'kernel32.dll', \
    msvcrt, 'msvcrt.dll'

import msvcrt, \
    printf, 'printf'


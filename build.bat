@echo off

set dependency_compiler=zig cc -O0 -w

set compiler=zig cc -std=c99 -pedantic ^
    -Wall -Werror -Wextra -Wshadow -Wconversion -Wdouble-promotion ^
    -Wno-unused-function -Wno-sign-conversion -fno-strict-aliasing ^
    -g3 -fsanitize=undefined -fsanitize-trap -DDEBUG

%dependency_compiler% -c src\stbi.c -o src\stbi.obj

%compiler% -c src\main.c -o src\main.obj && ^
%compiler% src\main.obj src\stbi.obj -o imgclr.exe

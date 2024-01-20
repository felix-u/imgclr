@echo off

set version=0.2
set cc=zig cc -O3 -w -static -s src\main.c

%cc% -target x86_64-windows     -o release/imgclr-v%version%-x86_64-win.exe 
%cc% -target aarch64-windows    -o release/imgclr-v%version%-aarch64-win.exe
%cc% -target x86_64-linux-musl  -o release/imgclr-v%version%-x86_64-linux   
%cc% -target aarch64-linux-musl -o release/imgclr-v%version%-aarch64-linux  
%cc% -target x86_64-macos       -o release/imgclr-v%version%-x86_64-macos   
%cc% -target aarch64-macos      -o release/imgclr-v%version%-aarch64-macos  

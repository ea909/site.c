@echo off
setlocal enableextensions

clang -O2 -DNDEBUG -DUNITY_BUILD site.c -o site.exe


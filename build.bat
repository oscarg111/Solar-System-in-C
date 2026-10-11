@echo off
set files=src\glad.c src\main.c src\math\math.c src\models\cube.c src\models\sphere.c
set libs=C:\work\lib\SDL2main.lib C:\work\lib\SDL2.lib

cl /nologo /Zi /I C:\work\include /I src %files% /link %libs% /OUT:learning-sdl.exe

mkdir bin
xcopy ressources "bin/ressources" /E /I /Y
xcopy levels "bin/levels" /E /I /Y
clang++ code/main.cpp code/microui.c ^
    -o bin/game.exe ^
    -std=c++20   ^
    -Wall -Wextra -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated-declarations -Wno-missing-field-initializers ^
    -I libs/include ^
    libs/win-x64/minifb.lib ^
    -lkernel32 -luser32 -lshell32 -ldxgi -ld3d11 -lole32 -lgdi32 -lwinmm -lopengl32 -v

bin\game.exe

gcc -o ./build/main.exe main.c file.c base.c debugger.c disassembly.c \
    -I"C:/raylib/include" \
    -I"." \
    -L"C:/raylib/lib" \
    -I./vendor/capstone/include \
    -L./vendor/capstone/lib -lcapstone \
    -g -Wall -lraylib -lopengl32 -lgdi32 -lwinmm && ./build/main.exe

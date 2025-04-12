# About

- Not finished probably and wont be either since `windows.h` is boring and annoying
- Definitely not usable, barely works on example code :D
- Mainly just to test Clay and make something flashy

**Example**:

https://github.com/user-attachments/assets/7a464bad-8a3c-461b-8d48-dc62d268d912

# How to run

- You must have: 
    - Raylib on `C:/raylib/`
    - Ninja
    - Windows 10+
    - MINGW and gcc
- This project uses `mate.h` build system I created, for more info check it [here](https://github.com/TomasBorquez/mate.h)
- To run use your compiler of choice and run the build script with: `gcc ./mate.c -o ./mate.exe && ./mate.exe` 
- For better LSP support run: `ninja -t compdb > compile_commands.json`

# TODOS:

## General:
- [x] Implement `gg` and `G`
- [x] Change functions to static and lowerCase if not used globally
- [ ] Fix all warnings
- [ ] Add hashMap abstraction

## Debugger:
- [x] Remove non existent registers
- [x] Array of function names and addresses
- [x] When performance problems change to hashMap instead of Vector
- [x] Once we know where main is, then center view on there
- [x] Change `DebuggerGoToNextBreakpoint` to `Next` and step through SIGSEGV and SIGTRAPS (ep. 3)
- [x] Remove breakpoint on each step
- [ ] Show segfault
- [ ] Make it not reliant on disable-dynamicbase add offset


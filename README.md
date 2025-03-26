# About

- Not finished probably and wont be either since `windows.h` is boring and annoying
- Definitely not usable, barely works on example code :D
- Mainly just to test Clay and make something flashy

**Example**:

https://github.com/user-attachments/assets/7a464bad-8a3c-461b-8d48-dc62d268d912

# How to run

- You must have raylib installed because yes
- You must have ninja installed because yes
- You must be on windows because yes
- You must use be using MINGW and gcc because yes
- You must have gdb installed because yes
- Run `ninja run`, create a `compile_commands.json` for better LSP support `ninja -t compdb > compile_commands.json`

# TODOS:

## General:
- [ ] Implement `gg` and `G`
- [ ] Add asserts everywhere
- [ ] Fix all warnings
- [ ] Change functions to static and lowerCase if not used globally
- [ ] Don't use relative keypress but QWERTY ones
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

## UI:
- [x] Add align, tl, tc, tr, etc.
- [x] Border radius.
- [x] Move color to renderers add all the colors from 50-950
- [x] Scroll in general
- [x] Border
- [x] Fix font height size
- [ ] Antialiasing on border radius
- [ ] Create function that selects fontId based on fontSize, load 8-72 to avoid blurriness
- [ ] ^ Better way of creating `Clay_TextElementConfig`
- [ ] Figure out how to get correct textHeight
- [ ] Follow tailwind guidelines better

## Renderer - When Available:
- [ ] Individual CornerRadius when available
- [ ] Reverse when available
- [ ] Margin when available


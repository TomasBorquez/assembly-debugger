#define MATE_IMPLEMENTATION
#include "mate.h"

i32 main() {
  StartBuild();
  {
    CreateExecutable((ExecutableOptions){.output = "main", .flags = "-Wall -ggdb"});

    AddFile("./src/main.c");
    AddFile("./src/file.c");
    AddFile("./src/base.c");
    AddFile("./src/debugger.c");
    AddFile("./src/disassembly.c");

    AddIncludePaths("C:/raylib/include", "./src", "./", "./vendor", "./vendor/capstone", "./vendor/renderer");
    AddLibraryPaths("C:/raylib/lib", "./vendor/capstone");
    LinkSystemLibraries("capstone", "raylib", "opengl32", "gdi32", "winmm");

    String exePath = InstallExecutable();
    RunCommand(exePath);

    CreateCompileCommands();
  }
  EndBuild();
}

#define MATE_IMPLEMENTATION
#include "mate.h"

i32 main() {
  StartBuild();
  {
    CreateExecutable((Executable){.output = S("main"), .flags = S("-Wall -ggdb")});

    AddFile(S("./src/main.c"));
    AddFile(S("./src/file.c"));
    AddFile(S("./src/base.c"));
    AddFile(S("./src/debugger.c"));
    AddFile(S("./src/disassembly.c"));

    AddIncludePaths(S("C:/raylib/include"), S("./src"), S("./"), S("./vendor"), S("./vendor/capstone"), S("./vendor/renderer"));
    AddLibraryPaths(S("C:/raylib/lib"), S("./vendor/capstone"));
    LinkSystemLibraries(S("capstone"), S("raylib"), S("opengl32"), S("gdi32"), S("winmm"));

    String exePath = InstallExecutable();
    RunCommand(exePath);

    CreateCompileCommands();
  }
  EndBuild();
}

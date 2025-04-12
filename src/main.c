#define CLAY_IMPLEMENTATION
#define RENDERER_IMPLEMENTATION
#include "capstone.h"
#include "renderer.h"

#include "clay.h"

#include "debugger.h"
#include "disassembly.h"
#include "file.h"
#include <stdint.h>

enum Tab {
  FILE_MANAGER,
  DEBUGGER,
};

typedef struct {
  int32_t selection;
  int8_t tab;
  Arena arena; // For UI text allocations
  Arena permanentArena;
  char *filePath;
  char *fileName;
  bool init;
  double startTime;
  double lastPressedG;
} State;
State state = {0};

Clay_String FormatFileSize(Arena *arena, uint64_t size) {
  const char *units[] = {"B", "KB", "MB", "GB", "TB"};
  int unit_index = 0;
  double formatted_size = (double)size;

  while (formatted_size >= 1024.0 && unit_index < 4) {
    formatted_size /= 1024.0;
    unit_index++;
  }

  if (unit_index == 0) {
    return F(arena, "%llu %s", size, units[unit_index]);
  }

  if (formatted_size >= 100) {
    return F(arena, "%.0f %s", formatted_size, units[unit_index]);
  }

  if (formatted_size >= 10) {
    return F(arena, "%.1f %s", formatted_size, units[unit_index]);
  }

  return F(arena, "%.2f %s", formatted_size, units[unit_index]);
}

void FileManagerUI() {
  Clay_TextElementConfig *textConfig = CLAY_TEXT_CONFIG({.fontId = FONT_24, .fontSize = 24, .textColor = CREAM});
  Box(.id = "Body", .pt = 42, .align = "tc", .w = "grow-0", .h = "grow-0", .bg = NEUTRAL_950) {
    Column(.bg = NEUTRAL_900, .pb = 7, .w = "fit-450", .gap = 8, .borderRadius = "a-lg") {
      Row(.id = "Top-Row", .bg = NEUTRAL_800, .w = "grow-0", .pt = 10, .pb = 5, .pl = 9, .pr = 7, .gap = 5, .borderRadius = "t-lg") {
        TextS("Names", textConfig);
        Separator(.px = 10);
        TextS("Size", textConfig);
      }

      Column(.id = "ContainerScroll", .w = "grow-0", .gap = 5, .scroll = "v") {
        Box(.id = "Up-Folder", .w = "grow-0", .bg = state.selection == 0 ? NEUTRAL_800 : NEUTRAL_900, .px = 10, .pt = 2) {
          TextS("../", textConfig);
        }

        for (int32_t i = 0; i < fileData->folderCount; i++) {
          Folder currentFolder = fileData->folders[i];
          float index = i + 1;
          Box(.bg = state.selection == index ? NEUTRAL_800 : NEUTRAL_900, .w = "grow-0", .px = 10, .pt = 4, .pb = 2) {
            Text(F(&state.arena, "%s/", currentFolder.name), textConfig);
          }
        }

        for (int32_t i = 0; i < fileData->fileCount; i++) {
          File currentFile = fileData->files[i];
          float index = i + fileData->folderCount + 1;

          Row(.bg = state.selection == index ? NEUTRAL_800 : NEUTRAL_900, .w = "grow-0", .px = 10, .pt = 4, .pb = 2) {
            Text(F(&state.arena, "%s.%s", currentFile.name, currentFile.extension), textConfig);
            Separator(.px = 10);
            Text(FormatFileSize(&state.arena, currentFile.size), textConfig);
          }
        }
      }
    }
  }
}

void DebuggerUI() {
  Clay_TextElementConfig *subTitleConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = CREAM});
  Clay_TextElementConfig *secondaryConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = NEUTRAL_300});
  Clay_TextElementConfig *secondValueConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = GREEN_200});

  Clay_TextElementConfig *noneConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = NONE});

  Clay_TextElementConfig *addressConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = BLUE_200});
  Clay_TextElementConfig *mnemoniConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = GREEN_300});
  Clay_TextElementConfig *valueConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = RED_300});

  Clay_TextElementConfig *highlightedConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = GREEN_200});
  Clay_TextElementConfig *breakPointConfig = CLAY_TEXT_CONFIG({.fontId = FONT_20, .fontSize = 20, .textColor = RED_500});
  Row(.id = "Body", .w = "grow-0", .bg = NEUTRAL_950, .p = 7, .gap = 7) {
    Box(.id = "Code", .w = "grow-0", .bg = NEUTRAL_900, .borderRadius = "a-xl") {
      Column(.id = "CodeScroll", .w = "grow-0", .h = "fit-0", .borderRadius = "a-xl", .p = 10, .gap = 2, .scroll = "v") {
        int pcIndex = FindInstructionIndex(GetPC());
        const size_t CONTEXT_SIZE = 200;
        size_t start = (pcIndex >= CONTEXT_SIZE) ? pcIndex - CONTEXT_SIZE : 0;
        size_t end = (pcIndex + CONTEXT_SIZE < ctx.count) ? pcIndex + CONTEXT_SIZE : ctx.count - 1;

        for (size_t i = start; i <= end; i++) {
          cs_insn *ins = &ctx.instructions[i];
          bool isCurrentInstruction = (i == pcIndex);
          BreakPoint *breakPoint = GetBreakPoint(ins->address);
          FunctionInfo *functionInfo = FindFunctionByAddress(ins->address);
          bool isBreakPoint = breakPoint != NULL && breakPoint->enabled;

          if (functionInfo != NULL) {
            Row(.gap = 10, .pt = 23) {
              Text(F(&state.arena, "0x%llx:", functionInfo->address), addressConfig);
              Text(F(&state.arena, "<%s>", functionInfo->name), subTitleConfig);
            }
          }

          Row(.gap = 10, .bg = i == start + state.selection ? NEUTRAL_800 : NEUTRAL_900, .w = "grow-0") {
            TextS("O", isBreakPoint ? breakPointConfig : noneConfig);

            if (isCurrentInstruction) {
              TextS("=>", highlightedConfig);
              Text(F(&state.arena, "0x%llx", (uintptr_t)ins->address), highlightedConfig);
            } else {
              TextS("  ", noneConfig);
              Text(F(&state.arena, "0x%llx", (uintptr_t)ins->address), isBreakPoint ? breakPointConfig : addressConfig);
            }
            Box(.w = "fit-36") {
              Text(F(&state.arena, "%s", ins->mnemonic), mnemoniConfig);
            }
            Text(F(&state.arena, "%s", ins->op_str), valueConfig);
          }
        }
      }
    }

    Box(.id = "SideBar", .w = "fit-300", .h = "grow-0", .bg = NEUTRAL_950, .borderRadius = "a-xl", .gap = 7) {
      Row(.id = "Target", .w = "grow-0", .bg = NEUTRAL_900, .borderRadius = "a-xl", .p = 10, .pb = 7, .gap = 5) {
        TextS("Target", subTitleConfig);
        Text(F(&state.arena, "./%s", state.fileName), secondValueConfig);
      }

      Column(.id = "Registers", .w = "grow-0", .bg = NEUTRAL_900, .borderRadius = "a-xl", .p = 10) {
        TextS("Registers", subTitleConfig);

        Row(.px = 10, .pt = 4, .gap = 7) {
          Column(.w = "grow-0") {
            for (int i = 0; i < 12; i++) {
              Row(.gap = 5) {
                Register currentRegister = GetRegisterDescriptor(i);
                Text(F(&state.arena, ".%s", currentRegister.name), secondaryConfig);
                Text(F(&state.arena, "0x%llx", GetRegisterValue(currentRegister.reg)), valueConfig);
              }
            }
          }

          Separator();

          Column(.w = "grow-0") {
            for (int i = 12; i < NUMBER_REGISTERS; i++) {
              Row(.gap = 5) {
                Register currentRegister = GetRegisterDescriptor(i);
                Text(F(&state.arena, ".%s", currentRegister.name), secondaryConfig);
                Text(F(&state.arena, "0x%llx", GetRegisterValue(currentRegister.reg)), valueConfig);
              }
            }
          }
        }
      }

      Column(.id = "Functions", .w = "grow-0", .h = "grow-0", .bg = NEUTRAL_900, .borderRadius = "a-xl", .p = 10) {
        TextS("Functions", subTitleConfig);

        Column(.px = 10, .pt = 4, .gap = 2, .h = "grow-0", .w = "grow-0") {
          for (int i = 0; i < functionVector.length; i++) {
            Row(.w = "grow-0") {
              FunctionInfo currentFunction = functionVector.data[i];
              Text(F(&state.arena, "0x%llx", currentFunction.address), secondaryConfig);
              Separator();
              Box(.w = "fit-150") {
                Text(F(&state.arena, "%s", currentFunction.name), strcmp(currentFunction.name, "main") ? valueConfig : highlightedConfig);
              }
            }
          }
        }
      }
    }
  }
}

Clay_RenderCommandArray CreateLayout(void) {
  Clay_BeginLayout();
  {
    if (state.tab == FILE_MANAGER) {
      FileManagerUI();
    } else {
      DebuggerUI();
    }
  }

  return Clay_EndLayout();
}

void update() {
  if (state.tab == FILE_MANAGER) {
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_U)) {
      ScrollContainerBottom("ContainerScroll");
      state.selection = fileData->totalCount;
      return;
    }

    if (IsKeyPressed(KEY_U)) {
      if (state.lastPressedG == true) {
        ScrollContainerTop("ContainerScroll");
        state.lastPressedG = false;
        state.selection = 0;
        return;
      }

      state.lastPressedG = true;
      return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_H)) {
      if (state.selection > 8) {
        ScrollContainerByY("ContainerScroll", -35 * 5);
      }

      if (fileData->totalCount > state.selection + 5) {
        state.selection += 5;
        return;
      }

      state.selection = fileData->totalCount;
      return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)) {
      ScrollContainerByY("ContainerScroll", 35 * 5);
      if (state.selection - 5 > 0) {
        state.selection -= 5;
        return;
      }

      state.selection = 0;
      return;
    }

    if (IsKeyPressed(KEY_C) && fileData->totalCount > state.selection) {
      if (state.selection > 8) {
        ScrollContainerByY("ContainerScroll", -35);
      }

      state.selection++;
      return;
    }

    if (IsKeyPressed(KEY_V) && state.selection > 0) {
      ScrollContainerByY("ContainerScroll", 35);
      state.selection--;
      return;
    }

    if (IsKeyPressed(KEY_ENTER)) {
      if (state.selection == 0) {
        SwitchDirectory("..");
        return;
      }

      if (fileData->folderCount > 0 && state.selection <= fileData->folderCount) {
        Folder folder = fileData->folders[state.selection - 1];
        SwitchDirectory(folder.name);

        if (state.selection > fileData->totalCount) {
          state.selection = fileData->totalCount;
        }
        return;
      }

      int32_t fileIndex = (state.selection - fileData->folderCount) - 1;
      assert(fileIndex < fileData->fileCount && "Selection should always be lower than fileCount");
      assert(fileData->fileCount > 0 && "Should always be higher than 0 if it gets to this point");

      File file = fileData->files[fileIndex];
      if (strcmp(file.extension, "exe") == 0) {
        char *cwd = GetCwd();
        state.filePath = (char *)F(&state.permanentArena, "%s", cwd).chars;
        state.fileName = (char *)F(&state.permanentArena, "%s.%s", file.name, file.extension).chars;
        state.tab = DEBUGGER;
        state.selection = 0;
        state.init = true;
        state.startTime = GetTime();
        InitDebugger(state.filePath, state.fileName);
      }
    }

    return;
  }

  if (state.tab == DEBUGGER) {
    // double currTime = GetTime();
    // if (state.init == true && currTime - state.startTime > 0.05) {
    //   state.init = false;
    //   FunctionInfo *main = FindFunctionByName("main");
    //
    //   const size_t CONTEXT_SIZE = 200;
    //   uintptr_t start = (main->address >= CONTEXT_SIZE) ? main->address - CONTEXT_SIZE : 0;
    //   double relativeIndex = main->address - start;
    //
    //   state.selection = relativeIndex;
    //   ScrollContainerByY("CodeScroll", -22 * (relativeIndex - 15));
    // }

    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_U)) {
      const size_t CONTEXT_SIZE = 200;
      state.selection = CONTEXT_SIZE * 2;
      ScrollContainerBottom("CodeScroll");
      return;
    }

    if (IsKeyPressed(KEY_U)) {
      if (state.lastPressedG == true) {
        ScrollContainerTop("CodeScroll");
        state.lastPressedG = false;
        state.selection = 0;
        return;
      }

      state.lastPressedG = true;
      return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_H)) {
      if (state.selection > 15) {
        ScrollContainerByY("CodeScroll", -22 * 5);
      }
      state.selection += 5;
      return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)) {
      ScrollContainerByY("CodeScroll", 22 * 5);
      if (state.selection - 5 > 0) {
        state.selection -= 5;
        return;
      }
      state.selection = 0;
      return;
    }

    if (IsKeyPressed(KEY_C)) {
      if (state.selection > 15) {
        ScrollContainerByY("CodeScroll", -22);
      }
      state.selection++;
      return;
    }

    if (IsKeyPressed(KEY_V) && state.selection > 0) {
      ScrollContainerByY("CodeScroll", 22);
      state.selection--;
      return;
    }

    if (IsKeyPressed(KEY_N)) {
      int pcIndex = FindInstructionIndex(GetPC());
      const size_t CONTEXT_SIZE = 200;
      size_t start = (pcIndex >= CONTEXT_SIZE) ? pcIndex - CONTEXT_SIZE : 0;
      uintptr_t address = ctx.instructions[start + state.selection].address;
      printf("%llx\n", address);
      SetBreakPoint(address);
      return;
    }

    if (IsKeyPressed(KEY_L)) {
      DebuggerContinueExecution();
      return;
    }

    if (IsKeyPressed(KEY_SEMICOLON)) {
      DebuggerStepInstruction();
      return;
    }

    return;
  }
}

void draw() {
  Clay_RenderCommandArray renderCommands = CreateLayout();

  BeginDrawing();
  {
    Clay_Raylib_Render(renderCommands, renderer.fonts);
  }
  EndDrawing();

  ArenaReset(&state.arena);
}

void init() {
  GetCwd();
  NewFileData();
  GetDirFiles();
  state.arena = ArenaInit(40096);
  state.permanentArena = ArenaInit(1028);
  state.tab = FILE_MANAGER;
  state.init = true;
  state.startTime = 0;
}

void clean() {
  FreeFileData();
}

int main() {
  init();

  {
    RenderOptions options = {
        .width = 1400,
        .height = 900,
        .windowName = "Assembly Debugger",
        .fontPath = "./resources/ComicMono.ttf",
    };
    RenderSetup(options, update, draw);
  }

  clean();
}

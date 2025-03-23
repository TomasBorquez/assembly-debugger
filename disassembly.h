#pragma once

#define NOGDI
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include "capstone/capstone.h"
#include "debugger.h"
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

typedef struct {
  csh handle;
  cs_insn *instructions;
  size_t count;
  uintptr_t startAddress;
  uintptr_t endAddress;
} DisassemblyContext;
extern DisassemblyContext ctx;

typedef struct {
  uintptr_t address;
  char *name;
} FunctionInfo;

typedef struct {
  FunctionInfo *data;
  size_t length;
  size_t capacity;
} FunctionVector;
extern FunctionVector functionVector;

void InitializeAssemblyDebugging();
int FindInstructionIndex(uintptr_t address);
FunctionInfo *FindFunctionByAddress(uintptr_t address);
FunctionInfo *FindFunctionByName(char *name);

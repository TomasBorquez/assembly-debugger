#pragma once
#define NOGDI
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include "khash.h"
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

typedef struct {
  intptr_t address;
  intptr_t savedData;
  bool enabled;
} BreakPoint;

KHASH_MAP_INIT_INT64(BreakPointMap, BreakPoint)

typedef struct {
  khash_t(BreakPointMap) * breakPoints;
  char *programName;
  DWORD pid;
  HANDLE processHandle;
  DEBUG_EVENT debugEvent;
  uintptr_t baseAddress;
} Debugger;
extern Debugger debugger;

BreakPoint *GetBreakPoint(uintptr_t address);
BreakPoint *InsertBreakPoint(BreakPoint breakPoint);
void SetBreakPoint(uintptr_t address);

BYTE ReadMemoryByte(uintptr_t address);

void DebuggerHandleCommand(char *line);
void DebuggerContinueExecution();
void DebuggerStepInstruction();
uintptr_t GetPC();

void InitDebugger(char *filePath, char *fileName);

/* Registers */
typedef enum { REG_RAX, REG_RBX, REG_RCX, REG_RDX, REG_RDI, REG_RSI, REG_RBP, REG_RSP, REG_R8, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15, REG_RIP, REG_RFLAGS, REG_CS, REG_FS, REG_GS, REG_SS, REG_DS, REG_ES } RegisterType;

#define NUMBER_REGISTERS 24

typedef struct {
  RegisterType reg;
  int32_t dwarfReg;
  char *name;
} Register;

static const Register registerDescriptors[NUMBER_REGISTERS] = {{REG_R15, 15, "r15"}, {REG_R14, 14, "r14"}, {REG_R13, 13, "r13"}, {REG_R12, 12, "r12"}, {REG_RBP, 6, "rbp"}, {REG_RBX, 3, "rbx"},
                                                               {REG_R11, 11, "r11"}, {REG_R10, 10, "r10"}, {REG_R9, 9, "r9"},    {REG_R8, 8, "r8"},    {REG_RAX, 0, "rax"}, {REG_RCX, 2, "rcx"},
                                                               {REG_RDX, 1, "rdx"},  {REG_RSI, 4, "rsi"},  {REG_RDI, 5, "rdi"},  {REG_RIP, -1, "rip"}, {REG_CS, 51, "cs"},  {REG_RFLAGS, 49, "eflags"},
                                                               {REG_RSP, 7, "rsp"},  {REG_SS, 52, "ss"},   {REG_DS, 53, "ds"},   {REG_ES, 50, "es"},   {REG_FS, 54, "fs"},  {REG_GS, 55, "gs"}};

Register GetRegisterDescriptor(size_t index);
uint64_t GetRegisterValue(RegisterType reg);
void SetRegisterValue(RegisterType reg, uint64_t value);

// Static
static void debuggerInit(char *programName, DWORD pid);
static void debuggerRun();
static void debuggerWaitForSignal();

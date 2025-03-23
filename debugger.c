#include "debugger.h"

#include "disassembly.h"
#include <fcntl.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Debugger debugger;

HANDLE getThreadHandle() {
  HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, debugger.debugEvent.dwThreadId);
  if (hThread == NULL) {
    fprintf(stderr, "Error opening thread: %lu\n", GetLastError());
    assert(0);
  }
  return hThread;
}

uint64_t getRegisterValue(HANDLE hThread, RegisterType reg) {
  CONTEXT context;
  context.ContextFlags = CONTEXT_ALL;

  if (!GetThreadContext(hThread, &context)) {
    fprintf(stderr, "Error getting thread context: %lu\n", GetLastError());
    assert(0 && "Error getting thread context: %lu\n");
  }

  switch (reg) {
  case REG_RAX:
    return context.Rax;
  case REG_RBX:
    return context.Rbx;
  case REG_RCX:
    return context.Rcx;
  case REG_RDX:
    return context.Rdx;
  case REG_RDI:
    return context.Rdi;
  case REG_RSI:
    return context.Rsi;
  case REG_RBP:
    return context.Rbp;
  case REG_RSP:
    return context.Rsp;
  case REG_R8:
    return context.R8;
  case REG_R9:
    return context.R9;
  case REG_R10:
    return context.R10;
  case REG_R11:
    return context.R11;
  case REG_R12:
    return context.R12;
  case REG_R13:
    return context.R13;
  case REG_R14:
    return context.R14;
  case REG_R15:
    return context.R15;
  case REG_RIP:
    return context.Rip;
  case REG_RFLAGS:
    return context.EFlags;
  case REG_CS:
    return context.SegCs;
  case REG_FS:
    return context.SegFs;
  case REG_GS:
    return context.SegGs;
  case REG_SS:
    return context.SegSs;
  case REG_DS:
    return context.SegDs;
  case REG_ES:
    return context.SegEs;
  default:
    return 0;
  }
}

uint64_t GetRegisterValue(RegisterType reg) {
  HANDLE hThread = getThreadHandle();
  uint64_t value = getRegisterValue(hThread, reg);
  CloseHandle(hThread);
  return value;
}

void setRegisterValue(HANDLE hThread, RegisterType reg, uint64_t value) {
  CONTEXT context;
  context.ContextFlags = CONTEXT_ALL;

  if (!GetThreadContext(hThread, &context)) {
    fprintf(stderr, "Error getting thread context: %lu\n", GetLastError());
    return;
  }

  switch (reg) {
  case REG_RAX:
    context.Rax = value;
    break;
  case REG_RBX:
    context.Rbx = value;
    break;
  case REG_RCX:
    context.Rcx = value;
    break;
  case REG_RDX:
    context.Rdx = value;
    break;
  case REG_RDI:
    context.Rdi = value;
    break;
  case REG_RSI:
    context.Rsi = value;
    break;
  case REG_RBP:
    context.Rbp = value;
    break;
  case REG_RSP:
    context.Rsp = value;
    break;
  case REG_R8:
    context.R8 = value;
    break;
  case REG_R9:
    context.R9 = value;
    break;
  case REG_R10:
    context.R10 = value;
    break;
  case REG_R11:
    context.R11 = value;
    break;
  case REG_R12:
    context.R12 = value;
    break;
  case REG_R13:
    context.R13 = value;
    break;
  case REG_R14:
    context.R14 = value;
    break;
  case REG_R15:
    context.R15 = value;
    break;
  case REG_RIP:
    context.Rip = value;
    break;
  case REG_RFLAGS:
    context.EFlags = value;
    break;
  case REG_CS:
    context.SegCs = value;
    break;
  case REG_FS:
    context.SegFs = value;
    break;
  case REG_GS:
    context.SegGs = value;
    break;
  case REG_SS:
    context.SegSs = value;
    break;
  case REG_DS:
    context.SegDs = value;
    break;
  case REG_ES:
    context.SegEs = value;
    break;
  default:
    break;
  }

  if (!SetThreadContext(hThread, &context)) {
    fprintf(stderr, "Error setting thread context: %lu\n", GetLastError());
  }
}

void SetRegisterValue(RegisterType reg, uint64_t value) {
  HANDLE hThread = getThreadHandle();
  setRegisterValue(hThread, reg, value);
  CloseHandle(hThread);
}

Register GetRegisterDescriptor(size_t index) {
  return registerDescriptors[index];
}

uint64_t ReadMemory(uintptr_t address) {
  int64_t originalByte;
  SIZE_T bytesRead;
  ReadProcessMemory(debugger.processHandle, (LPVOID)address, &originalByte, 1, &bytesRead);
  return originalByte;
}

BYTE ReadMemoryByte(uintptr_t address) {
  BYTE originalByte;
  SIZE_T bytesRead;
  ReadProcessMemory(debugger.processHandle, (LPVOID)address, &originalByte, 1, &bytesRead);
  return originalByte;
}

void WriteMemory(uintptr_t address, int64_t value) {
  SIZE_T bytesWritten;
  WriteProcessMemory(debugger.processHandle, (LPVOID)address, &value, 1, &bytesWritten);
}

void breakPointDisable(BreakPoint *breakPoint) {
  if (!breakPoint->enabled) {
    return;
  }

  WriteMemory(breakPoint->address, breakPoint->savedData);
  breakPoint->enabled = false;
}

void breakPointEnable(BreakPoint *breakPoint) {
  breakPoint->savedData = ReadMemory(breakPoint->address);
  WriteMemory(breakPoint->address, 0xCC);
  breakPoint->enabled = true;
}

BreakPoint *GetBreakPoint(uintptr_t address) {
  khiter_t k = kh_get(BreakPointMap, debugger.breakPoints, address);
  if (k == kh_end(debugger.breakPoints)) {
    return NULL;
  }
  return &kh_value(debugger.breakPoints, k);
}

BreakPoint *InsertBreakPoint(BreakPoint breakPoint) {
  int32_t ret;
  khiter_t k = kh_put(BreakPointMap, debugger.breakPoints, breakPoint.address, &ret);
  kh_value(debugger.breakPoints, k) = breakPoint;
  return &kh_value(debugger.breakPoints, k);
}

void SetBreakPoint(uintptr_t address) {
  BreakPoint *currBreakPoint = GetBreakPoint(address);
  if (currBreakPoint == NULL) {
    currBreakPoint = InsertBreakPoint((BreakPoint){.address = address, .enabled = false});
    breakPointEnable(currBreakPoint);
    return;
  }

  if (currBreakPoint->enabled == true) {
    breakPointDisable(currBreakPoint);
    return;
  }

  breakPointEnable(currBreakPoint);
}

uintptr_t GetPC() {
  return GetRegisterValue(REG_RIP);
}

void SetPC(uint64_t value) {
  SetRegisterValue(REG_RIP, value);
}

void StepOverBreakpoint() {
  uintptr_t possibleBreakpointLocation = GetPC() - 1;
  BreakPoint *currBreakPoint = GetBreakPoint(possibleBreakpointLocation);

  if (currBreakPoint == NULL) {
    return;
  }

  if (currBreakPoint->enabled == false) {
    return;
  }

  intptr_t previousInstructionAddress = possibleBreakpointLocation;
  SetPC(previousInstructionAddress);

  breakPointDisable(currBreakPoint);

  HANDLE hThread = getThreadHandle();
  uint64_t flags = getRegisterValue(hThread, REG_RFLAGS);
  flags |= 0x100;
  setRegisterValue(hThread, REG_RFLAGS, flags);

  ContinueDebugEvent(debugger.debugEvent.dwProcessId, debugger.debugEvent.dwThreadId, DBG_CONTINUE);
  debuggerWaitForSignal();

  flags = getRegisterValue(hThread, REG_RFLAGS);
  flags &= ~0x100;
  setRegisterValue(hThread, REG_RFLAGS, flags);

  breakPointEnable(currBreakPoint);
}

void DebuggerStepInstruction() {
  HANDLE hThread = getThreadHandle();
  StepOverBreakpoint();

  uint64_t flags = getRegisterValue(hThread, REG_RFLAGS);
  flags |= 0x100;
  setRegisterValue(hThread, REG_RFLAGS, flags);

  ContinueDebugEvent(debugger.debugEvent.dwProcessId, debugger.debugEvent.dwThreadId, DBG_CONTINUE);
  debuggerWaitForSignal();

  flags = getRegisterValue(hThread, REG_RFLAGS);
  flags &= ~0x100;
  setRegisterValue(hThread, REG_RFLAGS, flags);
}

void handleBreakPoint(EXCEPTION_RECORD exception) {
  HANDLE hThread = getThreadHandle();
  CONTEXT context;
  context.ContextFlags = CONTEXT_CONTROL;
  GetThreadContext(hThread, &context);
  printf("Hit breakpoint at address 0x%p\n", (void *)context.Rip);
  CloseHandle(hThread);
}

void debuggerWaitForSignal() {
  BOOL status = WaitForDebugEvent(&debugger.debugEvent, INFINITE);
  if (!status) {
    printf("WaitForDebugEvent failed with error %lu\n", GetLastError());
    return;
  }

  switch (debugger.debugEvent.dwDebugEventCode) {
  case CREATE_PROCESS_DEBUG_EVENT:
    debugger.baseAddress = (uintptr_t)debugger.debugEvent.u.CreateProcessInfo.lpBaseOfImage;
    CloseHandle(debugger.debugEvent.u.CreateProcessInfo.hFile);
    break;
  case EXCEPTION_DEBUG_EVENT:
    switch (debugger.debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
    case EXCEPTION_BREAKPOINT:
      handleBreakPoint(debugger.debugEvent.u.Exception.ExceptionRecord);
      break;
    case EXCEPTION_ACCESS_VIOLATION:
      printf("Segmentation Fault. Reason: %llx\n", debugger.debugEvent.u.Exception.ExceptionRecord.ExceptionInformation[0]);
      break;
    case EXCEPTION_SINGLE_STEP:
      printf("Single step at address: 0x%p\n", (void *)debugger.debugEvent.u.Exception.ExceptionRecord.ExceptionAddress);
      break;
    default:
      printf("Got exception 0x%lx\n", debugger.debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
    }
    break;
  case CREATE_THREAD_DEBUG_EVENT:
  case EXIT_THREAD_DEBUG_EVENT:
  case EXIT_PROCESS_DEBUG_EVENT:
  case LOAD_DLL_DEBUG_EVENT:
    CloseHandle(debugger.debugEvent.u.LoadDll.hFile);
    break;
  case UNLOAD_DLL_DEBUG_EVENT:
  case OUTPUT_DEBUG_STRING_EVENT:
  case RIP_EVENT:
    break;
  }

  ContinueDebugEvent(debugger.debugEvent.dwProcessId, debugger.debugEvent.dwThreadId, DBG_CONTINUE);
}

void DebuggerContinueExecution() {
  StepOverBreakpoint();
  ContinueDebugEvent(debugger.debugEvent.dwProcessId, debugger.debugEvent.dwThreadId, DBG_CONTINUE);
  debuggerWaitForSignal();
}

void debuggerRun() {
  debuggerWaitForSignal();
  InitializeAssemblyDebugging();
  FunctionInfo *mainFunction = FindFunctionByName("main");
  assert(mainFunction != NULL && "Main function should never be NULL");
  SetBreakPoint(mainFunction->address);

  for (size_t i = 0; i < 6; i++) {
    DebuggerContinueExecution();
  }
}

bool executeDebugee(char *applicationName, char *directoryPath, PROCESS_INFORMATION *piOut) {
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  si.cb = sizeof(si);

  bool result = CreateProcess(applicationName, NULL, NULL, NULL, FALSE, DEBUG_PROCESS, NULL, directoryPath, &si, &pi);
  if (!result) {
    fprintf(stderr, "Error creating process: %lu\n", GetLastError());
    return false;
  }

  *piOut = pi;
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return true;
}

static void debuggerInit(char *programName, DWORD pid) {
  debugger = (Debugger){0};
  debugger.programName = _strdup(programName);
  debugger.pid = pid;
  debugger.processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  debugger.breakPoints = kh_init(BreakPointMap);
}

void InitDebugger(char *filePath, char *fileName) {
  assert(fileName != NULL && filePath != NULL && "Program name not specified\n");

  PROCESS_INFORMATION pi = {0};
  if (!executeDebugee(fileName, filePath, &pi)) {
    return;
  }

  DWORD pid = pi.dwProcessId;
  debuggerInit(fileName, pid);
  debuggerRun();
}

#include "disassembly.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

DisassemblyContext ctx = {0};

typedef struct {
  cs_insn *data;
  size_t length;
  size_t capacity;
} InstructionVector;

FunctionVector functionVector = {0};

void pushFunction(uintptr_t address, const char *name) {
  if (functionVector.length >= functionVector.capacity) {
    size_t newCapacity = functionVector.capacity == 0 ? 8 : functionVector.capacity * 2;
    FunctionInfo *newData = (FunctionInfo *)realloc(functionVector.data, newCapacity * sizeof(FunctionInfo));
    if (!newData) {
      fprintf(stderr, "Memory allocation failed for function vector\n");
      return;
    }
    functionVector.data = newData;
    functionVector.capacity = newCapacity;
  }

  functionVector.data[functionVector.length].address = address;
  functionVector.data[functionVector.length].name = _strdup(name);
  functionVector.length++;
}

bool detectFunctions() {
  assert((ctx.instructions != NULL && ctx.count != 0) && "No disassembled instructions available\n");

  for (size_t i = 0; i < ctx.count; i++) {
    cs_insn *insn = &ctx.instructions[i];
    bool isPossibleFunctionStart = false;
    char functionName[256] = {0};

    if (i + 1 < ctx.count && strcmp(insn->mnemonic, "push") == 0 && strcmp(insn->op_str, "rbp") == 0 && strcmp(ctx.instructions[i + 1].mnemonic, "mov") == 0 && strstr(ctx.instructions[i + 1].op_str, "rbp, rsp") != NULL) {
      isPossibleFunctionStart = true;
      snprintf(functionName, sizeof(functionName), "func_%llx", (unsigned long long)insn->address);
    }

    if (strcmp(insn->mnemonic, "sub") == 0 && strstr(insn->op_str, "rsp") != NULL) {
      if (i > 0 && strcmp(ctx.instructions[i - 1].mnemonic, "push") == 0) {
        isPossibleFunctionStart = true;
        snprintf(functionName, sizeof(functionName), "func_%llx", (unsigned long long)ctx.instructions[i - 1].address);
      }
    }

    if (isPossibleFunctionStart) {
      uintptr_t funcAddr = (i > 0 && strcmp(ctx.instructions[i - 1].mnemonic, "push") == 0) ? ctx.instructions[i - 1].address : insn->address;
      pushFunction(funcAddr, functionName);
    }
  }

  return functionVector.length > 0;
}

bool ReadMemoryBlock(uintptr_t address, unsigned char *buffer, size_t size, size_t *bytesRead) {
  SIZE_T actualBytesRead = 0;
  BOOL success = ReadProcessMemory(debugger.processHandle, (LPVOID)address, buffer, size, &actualBytesRead);

  if (bytesRead != NULL) {
    *bytesRead = actualBytesRead;
  }

  return success && (actualBytesRead == size);
}

bool DisassembleSource(uintptr_t startAddress, uintptr_t endAddress) {
  if (ctx.instructions != NULL) {
    cs_free(ctx.instructions, ctx.count);
    ctx.instructions = NULL;
    ctx.count = 0;
  }

  size_t size = endAddress - startAddress;
  assert((size != 0 && endAddress > startAddress) && "Invalid address range for disassembly");

  const size_t BUFFER_SIZE = 4096;
  uint8_t *byteBuffer = malloc(size);
  size_t validBytes = 0;
  for (uintptr_t addr = startAddress; addr < endAddress; addr += BUFFER_SIZE) {
    size_t chunkSize = (addr + BUFFER_SIZE > endAddress) ? (endAddress - addr) : BUFFER_SIZE;
    size_t bytesRead = 0;

    if (ReadMemoryBlock(addr, byteBuffer + validBytes, chunkSize, &bytesRead)) {
      validBytes += bytesRead;
      continue;
    }

    printf("Warning: Could not read memory at 0x%llx (skipping %zu bytes)\n", (unsigned long long)addr, chunkSize);

    // Fill with NOPs to maintain correct addresses
    memset(byteBuffer + validBytes, 0x90, chunkSize);
    validBytes += chunkSize;
  }

  printf("Attempting to disassemble %zu bytes from 0x%llx\n", validBytes, (unsigned long long)startAddress);
  ctx.count = cs_disasm(ctx.handle, byteBuffer, validBytes, startAddress, 0, &ctx.instructions);
  ctx.startAddress = startAddress;
  ctx.endAddress = endAddress;

  free(byteBuffer);

  if (ctx.count <= 0) {
    fprintf(stderr, "Failed to disassemble code from 0x%llx to 0x%llx\n", (unsigned long long)startAddress, (unsigned long long)endAddress);
    return false;
  }

  printf("Successfully disassembled %zu instructions from 0x%llx to 0x%llx\n", ctx.count, (unsigned long long)startAddress, (unsigned long long)endAddress);
  return true;
}

int FindInstructionIndex(uintptr_t address) {
  for (size_t i = 0; i < ctx.count; i++) {
    if (ctx.instructions[i].address == address) {
      return i;
    }
  }
  return -1;
}

bool getSectionBoundaries(const char *sectionName, uintptr_t *startAddr, uintptr_t *endAddr) {
  if (strcmp(sectionName, ".text") == 0) {
    // TODO: Get these from actual PE parsing
    *startAddr = debugger.baseAddress + 0x1000; // Typical text section offset
    *endAddr = debugger.baseAddress + 0x10000;  // Conservative estimate
    return true;
  }

  return false;
}

bool disassembleSection(const char *sectionName) {
  uintptr_t startAddr, endAddr;

  if (!getSectionBoundaries(sectionName, &startAddr, &endAddr)) {
    fprintf(stderr, "Could not find section %s\n", sectionName);
    return false;
  }

  return DisassembleSource(startAddr, endAddr);
}

bool disassembleTextSection() {
  uintptr_t startAddr, endAddr;

  if (!getSectionBoundaries(".text", &startAddr, &endAddr)) {
    fprintf(stderr, "Could not determine .text section boundaries\n");
    return false;
  }

  printf("Disassembling .text section from 0x%llx to 0x%llx\n", (uintptr_t)startAddr, (uintptr_t)endAddr);

  return DisassembleSource(startAddr, endAddr);
}

bool detectMainFunction() {
  if (ctx.instructions == NULL || ctx.count == 0) {
    fprintf(stderr, "No disassembled instructions available\n");
    return false;
  }

  uintptr_t mainCandidates[10] = {0};
  int candidateCount = 0;

  for (size_t i = 0; i < ctx.count; i++) {
    cs_insn *insn = &ctx.instructions[i];

    if (strcmp(insn->mnemonic, "call") == 0) {
      uintptr_t callTarget = 0;
      if (sscanf(insn->op_str, "0x%llx", (unsigned long long *)&callTarget) == 1) {
        for (size_t j = 0; j < functionVector.length; j++) {
          if (functionVector.data[j].address == callTarget) {
            for (size_t k = 0; k < functionVector.length; k++) {
              if (functionVector.data[k].address <= insn->address && (k == functionVector.length - 1 || functionVector.data[k + 1].address > insn->address)) {

                if (strstr(functionVector.data[k].name, "start") != NULL || functionVector.data[k].address < ctx.startAddress + 0x1000) {

                  if (candidateCount < 10) {
                    mainCandidates[candidateCount++] = callTarget;
                  }
                }
                break;
              }
            }
            break;
          }
        }
      }
    }
  }

  if (candidateCount > 0) {
    uintptr_t mainAddress = mainCandidates[0];
    for (size_t i = 0; i < functionVector.length; i++) {
      if (functionVector.data[i].address == mainAddress) {
        free(functionVector.data[i].name);
        functionVector.data[i].name = _strdup("main");
        printf("Found main function at 0x%llx\n", (unsigned long long)mainAddress);
        return true;
      }
    }
  }

  printf("Could not definitively identify main function\n");
  return false;
}

FunctionInfo *FindFunctionByAddress(uintptr_t address) {
  for (int i = 0; i < functionVector.length; i++) {
    FunctionInfo *currentFunction = &(functionVector.data[i]);
    if (address == currentFunction->address) {
      return currentFunction;
    }
  }
  return NULL;
}

FunctionInfo *FindFunctionByName(char *name) {
  for (int i = 0; i < functionVector.length; i++) {
    FunctionInfo *currentFunction = &(functionVector.data[i]);
    if (strcmp(name, currentFunction->name) == 0) {
      return currentFunction;
    }
  }
  return NULL;
}

void InitializeAssemblyDebugging() {
  if (cs_open(CS_ARCH_X86, CS_MODE_64, &ctx.handle) != CS_ERR_OK) {
    fprintf(stderr, "Failed to initialize Capstone disassembly engine\n");
    return;
  }
  cs_option(ctx.handle, CS_OPT_DETAIL, CS_OPT_ON);

  if (disassembleTextSection()) {
    detectFunctions();
    detectMainFunction();
  }
}

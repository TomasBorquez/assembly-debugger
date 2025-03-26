#pragma once
#include <stdint.h>
#define MAX_FILES 200
#define MAX_PATH 260

typedef struct {
  char *type;
  char *name;
  char *extension; 
  int64_t size;
} File;

typedef struct {
  char *name;
} Folder;

typedef struct {
  Folder *folders;
  size_t folderCount;

  File *files;
  size_t fileCount;

  size_t totalCount;
} FileData;

extern FileData *fileData;
extern char currentPath[MAX_PATH];

char *GetCwd();
char *SetCwd(char *destination);

void GetDirFiles();

void NewFileData();

void FreeFileData();
void ResetFileData();
void SwitchDirectory(char *path);


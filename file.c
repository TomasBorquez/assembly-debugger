#include "file.h"
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

FileData *fileData;
char currentPath[MAX_PATH];

char *GetCwd() {
  DWORD length = GetCurrentDirectory(MAX_PATH, currentPath);

  if (length == 0) {
    printf("Error getting current directory: %lu\n", GetLastError());
    return "";
  }

  return currentPath;
}

char *SetCwd(char *destination) {
  bool result = SetCurrentDirectory(destination);
  if (!result) {
    printf("Error setting cwd: %lu\n", GetLastError());
    return currentPath;
  }

  GetCwd();
  return currentPath;
}

void GetDirFiles() {
  WIN32_FIND_DATA findData;
  HANDLE hFind;
  char searchPath[MAX_PATH];
  snprintf(searchPath, MAX_PATH, "%s\\*", currentPath);

  hFind = FindFirstFile(searchPath, &findData);
  if (hFind == INVALID_HANDLE_VALUE) {
    printf("Error finding files: %lu\n", GetLastError());
    return;
  }

  do {
    if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
      continue;
    }

    if (fileData->totalCount >= MAX_FILES) {
      printf("Warning: Maximum file count reached (%d). Some files might be skipped.\n", MAX_FILES);
      break;
    }

    bool isDirectory = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    File *currFile = &fileData->files[fileData->fileCount];
    Folder *currFolder = &fileData->folders[fileData->folderCount];

    if (isDirectory) {
      currFolder->name = _strdup(findData.cFileName);
      fileData->folderCount++;
    }

    if (!isDirectory) {
      currFile->type = _strdup("FILE");

      char *dot = strrchr(findData.cFileName, '.');
      if (dot != NULL) {
        currFile->extension = _strdup(dot + 1);

        size_t baseNameLength = dot - findData.cFileName;
        char *baseName = (char *)malloc(baseNameLength + 1);
        memcpy(baseName, findData.cFileName, baseNameLength);
        baseName[baseNameLength] = '\0';
        currFile->name = baseName;
      }

      if (dot == NULL) {
        currFile->extension = _strdup("");
        currFile->name = _strdup(findData.cFileName);
      }

      currFile->size = (((int64_t)findData.nFileSizeHigh) << 32) | findData.nFileSizeLow;
      fileData->fileCount++;
    }

    fileData->totalCount++;
  } while (FindNextFile(hFind, &findData) != 0);

  DWORD dwError = GetLastError();
  if (dwError != ERROR_NO_MORE_FILES) {
    printf("Error searching for files: %lu\n", dwError);
  }

  FindClose(hFind);
  return;
}

void NewFileData() {
  fileData = (FileData *)malloc(sizeof(FileData));
  fileData->files = (File *)malloc(MAX_FILES * sizeof(File));
  fileData->fileCount = 0;
  fileData->folders = (Folder *)malloc(MAX_FILES * sizeof(Folder));
  fileData->folderCount = 0;
  fileData->totalCount = 0;
};

void FreeFileData() {
  if (fileData->files == NULL && fileData->folders == NULL) return;

  for (size_t i = 0; i < fileData->fileCount; i++) {
    File currentFile = fileData->files[i];
    free(currentFile.type);
    free(currentFile.name);
    free(currentFile.extension);
  }

  free(fileData->files);

  for (size_t i = 0; i < fileData->folderCount; i++) {
    Folder currentFolder = fileData->folders[i];
    free(currentFolder.name);
  }

  free(fileData->folders);
  free(fileData);
}

void ResetFileData() {
  fileData->fileCount = 0;
  fileData->folderCount = 0;
  fileData->totalCount = 0;
}

void SwitchDirectory(char *path) {
  ResetFileData();
  SetCwd(path);
  GetDirFiles();
}

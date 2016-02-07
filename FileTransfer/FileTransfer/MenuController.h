#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "Global.h"

void FindDialogItems();
void EnableGUI(BOOL isEnable);
void FileManager();
void loadFileToView(std::string * readFrom, LPSTR file);
void InitFileOpener();
DWORD WINAPI createFileReader(LPVOID lpParam);
void AppendToStatus(HWND edit, char * buffer);
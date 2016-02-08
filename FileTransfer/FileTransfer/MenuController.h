#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "Global.h"

void	FindDialogItems();
void	EnableGUI(BOOL isEnable);
void	AppendToStatus(HWND edit, char * buffer);
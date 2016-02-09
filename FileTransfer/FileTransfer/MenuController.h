/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	MenuController.c - For initialization of menu item handlers and filling default parameters
--
-- PROGRAM:		Protocol Analyzer
--
-- FUNCTIONS:	void FindDialogItems()
--				void EnableGUI(BOOL isEnable)
--				void AppendToStatus(HWND handle, char * buffer)
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- NOTES:
--------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "Global.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	FindDialogItems
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void FindDialogItems()
--
-- RETURNS: void
--
-- NOTES: Initializes menu item handlers
--------------------------------------------------------------------------------------------------------------------*/
void	FindDialogItems();

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	FillDefaultValues
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void FillDefaultValues()
--
-- RETURNS: void
--
-- NOTES: Fill in some default values for the ease of troubleshooting
--------------------------------------------------------------------------------------------------------------------*/
void	FillDefaultValues();

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	EnableGUI
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void EnableGUI(BOOL isEnable)
--
-- RETURNS: void
--
-- NOTES: Enable and disable menu items depending on which mode is checked (Server or Client)
--------------------------------------------------------------------------------------------------------------------*/
void	EnableGUI(BOOL isEnable);

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	AppendToStatus
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void AppendToStatus(HWND handle, char * buffer)
--
-- RETURNS: void
--
-- NOTES: Wrapper function to update string into a edit field
--------------------------------------------------------------------------------------------------------------------*/
void	AppendToStatus(HWND edit, char * buffer);
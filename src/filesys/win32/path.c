/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 * $Id: path.c,v 1.4 2001/08/26 12:23:37 darkfiber Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <windows.h>
#include <stdio.h>
#include "sarien.h"
#include "agi.h"


/* Date: Tue, 22 May 2001 16:22:06 +0400
 * From: Igor Nesterov <nest@rtsnet.ru>
 * To: sarien-devel@lists.sourceforge.net
 * Subject: Fw: [sarien-devel] Improved savegames
 * 
 * I remember yet another "ms-compatible solution" common for Win9X and
 * WinNT/2K: ApplicationData directory. Value AppData in registry key
 * "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\
 * Shell Folders" hold path to this directory. This maps to "c:\windows\
 * appilcation data" for Win9X and "C:\Documents and Settings\UserName\
 * Application Data for NT
 */

char ini_path[MAX_PATH];

int get_app_dir (char *app_dir, unsigned int size)
{
	const char *szapp = "AppData";
	const char *szpath = "Software\\Microsoft\\Windows\\CurrentVersion\\"
		"Explorer\\Shell Folders";
	HKEY hkey;
	DWORD type;

	if (RegOpenKey (HKEY_CURRENT_USER, szpath, &hkey) != ERROR_SUCCESS)
		return -1;

	if (RegQueryValueEx (hkey, szapp, NULL, &type,
		(unsigned char *)app_dir, &size) != ERROR_SUCCESS)
	{
		RegCloseKey (hkey);
		return -1;
	}

	RegCloseKey (hkey);

	return 0;
}

char* get_config_file(void)
{
	strcpy(ini_path, "./");

	if(getenv("SARIEN")!=NULL)
	{
		sprintf(ini_path, "%s/%s", getenv("SARIEN"), "sarien.ini");
	}
	strcat(ini_path, "/sarien.ini");

	return (char*)ini_path;
}

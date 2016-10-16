#ifndef _MFDCORE_H_
#define _MFDCORE_H_

#include "Orbitersdk.h"

using namespace std;

typedef struct _MFDSettings {
	OBJHANDLE vessel = 0;
	bool isActive;
	VECTOR3 tgtVel;
	bool isNav;
	int navNo;
	string navRefName;
	bool isTgt;
	VESSEL* tgtVessel;
} MFDSettings;

MFDSettings* loadSettings(VESSEL* vessel);
void freeAllSettings();

#endif
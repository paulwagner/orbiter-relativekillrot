#include "MFDCore.h"
#include <vector>

static vector<MFDSettings*> settingsMap;

MFDSettings* loadSettings(VESSEL* vessel) {
	MFDSettings* settings;
	bool hasSettings = false;
	for (vector<MFDSettings*>::iterator it = settingsMap.begin(); it != settingsMap.end(); ++it) {
		if ((*it) && (*it)->vessel == vessel->GetHandle()) {
			hasSettings = true;
			settings = (*it);
			break;
		}
	}
	if (!hasSettings) {
		settings = new MFDSettings();
		settings->vessel = vessel->GetHandle();
		settings->isActive = false;
		settings->tgtVel = _V(0, 0, 0);
		settings->isNav = false;
		settings->isTgt = false;
		settings->tgtVessel = 0;
		settings->navNo = 0;
		settingsMap.push_back(settings);
	}
	return settings;
}

void freeAllSettings() {
	for (vector<MFDSettings*>::iterator it = settingsMap.begin(); it != settingsMap.end(); ++it) {
		if (*it) {
			delete (*it);
		}
	}
}
#define STRICT
#define ORBITER_MODULE

#include "RelativeKillrot.h"
#include "windows.h"
#include "orbitersdk.h"

#include <cctype>
#include <string>

// ==============================================================
// Global variables
static int MFDMode;
static RelativeKillrot* CurrentMFD = 0;
static 	MFDSettings* settings = 0;

// ==============================================================
// API interface

DLLCLBK void InitModule (HINSTANCE hDLL)
{
	TRACE("[RelativeKillrot] Enter: InitModule");
	static char *name = "RelativeKillrot";   // MFD mode name
	MFDMODESPECEX spec;
	spec.name = name;
	spec.key = OAPI_KEY_R;                // MFD mode selection key
	spec.context = NULL;
	spec.msgproc = RelativeKillrot::MsgProc;  // MFD mode callback function

	// Register the new MFD mode with Orbiter
	MFDMode = oapiRegisterMFDMode(spec);
	TRACE("[RelativeKillrot] Leave: InitModule");
}

DLLCLBK void ExitModule (HINSTANCE hDLL)
{
	TRACE("[RelativeKillrot] Enter: ExitModule");
	freeAllSettings();
	// Unregister the custom MFD mode when the module is unloaded
	oapiUnregisterMFDMode(MFDMode);
}

// ==============================================================
// MFD class implementation

// Constructor
RelativeKillrot::RelativeKillrot(DWORD w, DWORD h, UINT mfd, VESSEL *vessel)
: MFD2 (w, h, vessel)
{
	TRACE("[RelativeKillrot] Enter: _cdecl");
	penBlue = oapiCreatePen(1, 1, BLUE);
	penGreen = oapiCreatePen(1, 1, GREEN);
	penGreen2 = oapiCreatePen(1, 1, GREEN2);
	penRed = oapiCreatePen(1, 1, RED);
	penWhite = oapiCreatePen(1, 1, WHITE);
	penBlack = oapiCreatePen(1, 1, BLACK);
	penYellow2 = oapiCreatePen(1, 1, YELLOW2);
	penYellow3 = oapiCreatePen(1, 2, YELLOW2);
	brushBlue = oapiCreateBrush(BLUE);
	brushGreen = oapiCreateBrush(GREEN);
	brushGreen2 = oapiCreateBrush(GREEN2);
	brushRed = oapiCreateBrush(RED);
	brushWhite = oapiCreateBrush(WHITE);
	brushBlack = oapiCreateBrush(BLACK);
	brushYellow2 = oapiCreateBrush(YELLOW2);
	// MFD initialisation
	settings = loadSettings(vessel);
	chw = round((double)H / 20);
	chw = min(chw,round((double)W / 20));
	chw2 = chw / 2;
	chw3 = chw / 3;
	TRACE("[AttitudeIndicatorMFD] Leave: _cdecl");
}

// Destructor
RelativeKillrot::~RelativeKillrot()
{
	TRACE("[RelativeKillrot] Enter: _ddecl");
	// MFD cleanup code
	oapiReleasePen(penBlue);
	oapiReleasePen(penGreen);
	oapiReleasePen(penGreen2);
	oapiReleasePen(penRed);
	oapiReleasePen(penWhite);
	oapiReleasePen(penBlack);
	oapiReleasePen(penYellow2);
	oapiReleasePen(penYellow3);
	oapiReleaseBrush(brushBlue);
	oapiReleaseBrush(brushGreen);
	oapiReleaseBrush(brushGreen2);
	oapiReleaseBrush(brushRed);
	oapiReleaseBrush(brushWhite);
	oapiReleaseBrush(brushBlack);
	oapiReleaseBrush(brushYellow2);
	CurrentMFD = 0;
}

// Return button labels
char *RelativeKillrot::ButtonLabel(int bt)
{
	// The labels for the buttons used by our MFD mode
	static char *label[12] = { "RST", "CUR", "NAV", "TGT", "NT", "ACT", "PIT", "P-", "YAW", "Y-", "BNK", "B-" };
	return (bt < 12 ? label[bt] : 0);
}

// Return button menus
int RelativeKillrot::ButtonMenu(const MFDBUTTONMENU **menu) const
{
	// The menu descriptions for the buttons
	static MFDBUTTONMENU mnu[12] = {
		{ "Reset Target", 0, 'R' },
		{ "Choose Current", 0, 'C' },
		{ "Choose NAV", 0, 'N' },
		{ "Choose Target", 0, 'T' },
		{ "Delete Target", 0, 'G'},
		{ "Activate", 0, 'A' },
		{ "Increase Pitch", 0, 'P' },
		{ "Decrease Pitch", 0, 'D' },
		{ "Increase Yaw", 0, 'Y' },
		{ "Decrease Yaw", 0, 'E' },
		{ "Increase Bank", 0, 'B' },
		{ "Decrease Bank", 0, 'F' }
	};

	if (menu) *menu = mnu;
	return 12; // return the number of buttons used
}

static int ctr = 0;
bool RelativeKillrot::ConsumeButton(int bt, int event)
{
	static const DWORD btkey[12] = { OAPI_KEY_R, OAPI_KEY_C, OAPI_KEY_N, OAPI_KEY_T, OAPI_KEY_G, OAPI_KEY_A, OAPI_KEY_P, OAPI_KEY_D, OAPI_KEY_Y, OAPI_KEY_E, OAPI_KEY_B, OAPI_KEY_F };
	if (event & PANEL_MOUSE_LBPRESSED) {
		if (bt >= 6 && bt < 12) {
			if (ctr < 5) { ctr++; return true; }
			ctr = 0;
			return ConsumeKeyBuffered(btkey[bt]);
		}
	}

	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	if (bt < 12) return ConsumeKeyBuffered(btkey[bt]);
	else return false;
}

bool RelativeKillrot::ConsumeKeyBuffered(DWORD key)
{
	switch (key) {
	case OAPI_KEY_R:
		settings->isNav = false;
		settings->isTgt = false;
		settings->tgtVel = _V(0,0,0);
		return true;
	case OAPI_KEY_C:
		pV->GetAngularVel(settings->tgtVel);
		settings->isNav = false;
		settings->isTgt = false;
		return true;
	case OAPI_KEY_N:
		if (settings->isNav) {
			if (settings->navNo < (int)pV->GetNavCount()-1)
				settings->navNo++;
			else
				settings->isNav = false;

		}
		else if (pV->GetNavCount() > 0) {
			settings->isTgt = false;
			settings->isNav = true;
			settings->navNo = 0;
		}
		settings->tgtVel = _V(0, 0, 0);
		if (settings->isNav) {
			settings->navRefName = "[no signal]";

			NAVHANDLE navhandle = pV->GetNavSource(settings->navNo);
			if (navhandle) {
				char refname[50];
				oapiGetNavDescr(navhandle, refname, 50);
				settings->navRefName.assign(refname);
				NAVDATA ndata;
				oapiGetNavData(navhandle, &ndata);
				VESSEL* tgtV = 0;
				if (ndata.type == TRANSMITTER_IDS) {
					settings->tgtVessel = oapiGetVesselInterface(ndata.ids.hVessel);
				}
				else if (ndata.type == TRANSMITTER_XPDR) {
					settings->tgtVessel = oapiGetVesselInterface(ndata.xpdr.hVessel);
				}
			}
		}
		return true;
	case OAPI_KEY_T:
		oapiOpenInputBox("Enter Target", DialogClbk, "", 30, this);
		return true;
	case OAPI_KEY_G:
		settings->isTgt = false;
		settings->tgtVel = _V(0, 0, 0);
		return true;
	case OAPI_KEY_A:
		settings->isActive = !settings->isActive;
		return true;
	case OAPI_KEY_P:
		if (!settings->isNav && !settings->isTgt)
			settings->tgtVel.x += 0.1 * RAD;
		return true;
	case OAPI_KEY_D:
		if (!settings->isNav && !settings->isTgt)
			settings->tgtVel.x -= 0.1 * RAD;
		return true;
	case OAPI_KEY_Y:
		if (!settings->isNav && !settings->isTgt)
			settings->tgtVel.y += 0.1 * RAD;
		return true;
	case OAPI_KEY_E:
		if (!settings->isNav && !settings->isTgt)
			settings->tgtVel.y -= 0.1 * RAD;
		return true;
	case OAPI_KEY_B:
		if (!settings->isNav && !settings->isTgt)
			settings->tgtVel.z += 0.1 * RAD;
		return true;
	case OAPI_KEY_F:
		if (!settings->isNav && !settings->isTgt)
			settings->tgtVel.z -= 0.1 * RAD;
		return true;

	}
	return false;
}

bool DialogClbk(void *id, char *str, void *usrdata) {
	for (char *iter = str; *iter != '\0'; ++iter) {
		*iter = std::tolower(*iter);
		++iter;
	}

	OBJHANDLE hVessel = oapiGetVesselByName(str);
	if (!hVessel) return false;
	settings->tgtVessel = oapiGetVesselInterface(hVessel);
	settings->isNav = false;
	settings->isTgt = true;

	return true;
}

void writeCenteredText(oapi::Sketchpad *skp, string s, int x, int y) {
	int woff = (int)round((double)skp->GetTextWidth(s.c_str(), s.length()) / 2);
	skp->Text(x - woff, y, s.c_str(), s.length());
}

void writeLine(oapi::Sketchpad *skp, string s, double d, int w, int iy) {
	s.append("  ");
	if (d >= 0) s.append(" ");
	s.append(std::to_string(DEG*d));
	s = s.substr(0, 17);
	writeCenteredText(skp, s, w, iy);
}

// Repaint the MFD
bool RelativeKillrot::Update(oapi::Sketchpad *skp) {
	Title(skp, "RelativeKillrot");
	
	int th = skp->GetCharSize() & 0xFFFF;
	int h4 = (int)round((double)H / 4);
	int h2 = (int)round((double)H / 2);
	int h23 = (int)round((double)(2 * H) / 3);
	int h25 = (int)round((double)(2 * H) / 5);
	int h34 = (int)round((double)(3 * H) / 4);
	int w2 = (int)round((double)W / 2);
	int w4 = (int)round((double)W / 4);
	int w34 = (int)round((double)(3*W) / 4);

	skp->SetPen(penWhite);
	skp->Line(w2, 5*th, w2, h25 + th);

	writeCenteredText(skp, "CURRENT", w4, 3*th);
	writeCenteredText(skp, "TARGET", w34, 3*th);
	writeCenteredText(skp, "DELTA", w2, h25 + 2 * th);

	VECTOR3 vVel;
	pV->GetAngularVel(vVel);
	skp->SetTextColor(GREEN);
	int iy = 3 * th + 2 * th;
	writeLine(skp, "PTCH", vVel.x, w4, iy);
	writeLine(skp, "PTCH", settings->tgtVel.x, w34, iy);
	iy += 2 * th;
	writeLine(skp, "YAW ", vVel.y, w4, iy);
	writeLine(skp, "YAW ", settings->tgtVel.y, w34, iy);
	iy += 2 * th;
	writeLine(skp, "BANK", vVel.z, w4, iy);
	writeLine(skp, "BANK", settings->tgtVel.z, w34, iy);

	VECTOR3 diffVel = vVel - settings->tgtVel;
	skp->SetTextColor(YELLOW2);
	iy = h25 + 2 * th + 2 * th;
	writeLine(skp, "PTCH", diffVel.x, w2, iy);
	iy += 2 * th;
	writeLine(skp, "YAW ", diffVel.y, w2, iy);
	iy += 2 * th;
	writeLine(skp, "BANK", diffVel.z, w2, iy);

	iy += 2*th;
	skp->Line(w2, iy, w2, iy + th);
	skp->Line(0, iy + th, W, iy + th);

	string refstr = "";
	if (settings->isNav)
		refstr.append("NAV").append(std::to_string(settings->navNo+1)).append(" ").append(settings->navRefName);
	else if (settings->isTgt)
		refstr.append("TGT ").append(settings->tgtVessel->GetName());
	skp->SetTextColor(WHITE);
	writeCenteredText(skp, refstr, w34, H - 2 * th);

	if (settings->isActive) {
		skp->SetTextColor(RED);
		writeCenteredText(skp, "ACTIVE", w4, H - 2*th);
	}

	return true;
}

// MFD message parser
int RelativeKillrot::MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	TRACE("[RelativeKillrot] Enter: MsgProc");
	switch (msg) {
	case OAPI_MSG_MFD_OPENED: {
		// Our new MFD mode has been selected, so we create the MFD and
		// return a pointer to it.
		CurrentMFD = new RelativeKillrot(LOWORD(wparam), HIWORD(wparam), mfd, (VESSEL*)lparam);
		return (int)CurrentMFD;
	}
	}
	TRACE("[RelativeKillrot] Leave: MsgProc");
	return 0;
}

DLLCLBK void opcPreStep(double SimT, double SimDT, double mjd)
{
	VESSEL *vessel = oapiGetFocusInterface();
	if (!settings || !vessel) return;

	if (settings->tgtVessel && (settings->isNav || settings->isTgt)) {
		settings->tgtVessel->GetAngularVel(settings->tgtVel);
		// Rotate to local vessel frame
		settings->tgtVessel->GlobalRot(settings->tgtVel, settings->tgtVel);
		MATRIX3 r;
		vessel->GetRotationMatrix(r);
		settings->tgtVel = tmul(r, settings->tgtVel);
	}

	if (settings->isActive) {
		if (vessel->GetNavmodeState(NAVMODE_KILLROT)) {
			vessel->DeactivateNavmode(NAVMODE_KILLROT);
			if (vessel->Version() > 1) {
				((VESSEL2*)vessel)->clbkNavMode(NAVMODE_KILLROT, true);
			} else 
				vessel->SetAngularVel(settings->tgtVel);
		}

	}


}
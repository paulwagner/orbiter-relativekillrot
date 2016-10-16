#ifndef __RELATIVEKILLROT_H
#define __RELATIVEKILLROT_H

#include "MFDCore.h"

#ifdef _DEBUG
#define TRACE(X) oapiWriteLog(X);
#else
#define TRACE(X)
#endif

#define WHITE RGB(255,255,255)
#define BLACK RGB(0,0,0)
#define RED RGB(255,0,0)
#define GREEN RGB(0,255,0)
#define GREEN2 RGB(0,172,0)
#define BLUE RGB(0,0,255)
#define YELLOW RGB(255,255,0)
#define YELLOW2 RGB(230,230,0)

class RelativeKillrot: public MFD2 {
public:
	RelativeKillrot(DWORD w, DWORD h, UINT mfd, VESSEL *vessel);
	~RelativeKillrot();
	char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	bool ConsumeButton(int bt, int event);
	bool ConsumeKeyBuffered(DWORD key);
	bool Update (oapi::Sketchpad *skp);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

protected:
	double chw;
	double chw2;
	double chw3;

private:
	oapi::Pen *penBlue, *penGreen, *penGreen2, *penRed, *penWhite, *penBlack, *penYellow2, *penYellow3;
	oapi::Brush *brushBlue, *brushGreen, *brushGreen2, *brushRed, *brushWhite, *brushBlack, *brushYellow2;

};

bool DialogClbk(void *id, char *str, void *usrdata);

#endif // !__RELATIVEKILLROT_H
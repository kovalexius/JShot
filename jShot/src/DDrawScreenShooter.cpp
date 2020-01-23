#include <iostream>

#include "DDrawScreenShooter.h"
#include "ddraw_decode_error.h"

void CDDrawScreenShooter::GetScreenShot(const CRectangle& _region)
{
	LPDIRECTDRAW lpDDraw;
	HRESULT hResult = DirectDrawCreate(NULL, &lpDDraw, NULL);
	if (hResult != DD_OK)
	{
		return;
	}

	HWND desktopHwnd = GetDesktopWindow();
	hResult = lpDDraw->SetCooperativeLevel(desktopHwnd, DDSCL_NORMAL);
	if (hResult != DD_OK)
	{
		return;
	}

	DDSURFACEDESC ddsd;
	DDSCAPS ddsc;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddsd.dwFlags = DDSD_CAPS;

	IDirectDrawSurface * lpPrimarySurface;
	hResult = lpDDraw->CreateSurface(&ddsd, &lpPrimarySurface, NULL);
	if (hResult != DD_OK)
	{
		std::cout << "Error: " << decodeCreateSurfaceError(hResult) << std::endl;
		return;
	}


	HDC hDC;
	hResult = lpPrimarySurface->GetDC(&hDC);
	if (hResult != DD_OK)
	{
		return;
	}


	std::cout << "Success" << std::endl;
}
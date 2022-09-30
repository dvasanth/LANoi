
#ifndef _CREATEGRAYSCALEICON_H
#define _CREATEGRAYSCALEICON_H

COLORREF defaultGrayPalette[256];
BOOL bGrayPaletteSet = FALSE;

HICON CreateGrayscaleIcon( HICON hIcon, COLORREF* pPalette )
{
	if (hIcon == NULL)
	{
		return NULL;
	}

	HDC hdc = ::GetDC(NULL);

	HICON      hGrayIcon      = NULL;
	ICONINFO   icInfo         = { 0 };
	ICONINFO   icGrayInfo     = { 0 };
	LPDWORD    lpBits         = NULL;
	LPBYTE     lpBitsPtr      = NULL;
	SIZE sz;
	DWORD c1 = 0;
	BITMAPINFO bmpInfo        = { 0 };
	bmpInfo.bmiHeader.biSize  = sizeof(BITMAPINFOHEADER);

	if (::GetIconInfo(hIcon, &icInfo))
	{
		if (::GetDIBits(hdc, icInfo.hbmColor, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS) != 0)
		{
			bmpInfo.bmiHeader.biCompression = BI_RGB;

			sz.cx = bmpInfo.bmiHeader.biWidth;
			sz.cy = bmpInfo.bmiHeader.biHeight;
			c1 = sz.cx * sz.cy;

			lpBits = (LPDWORD)::GlobalAlloc(GMEM_FIXED, (c1) * 4);

			if (lpBits && ::GetDIBits(hdc, icInfo.hbmColor, 0, sz.cy, lpBits, &bmpInfo, DIB_RGB_COLORS) != 0)
			{
				lpBitsPtr     = (LPBYTE)lpBits;
				UINT off      = 0;

				for (UINT i = 0; i < c1; i++)
				{
					off = (UINT)( 255 - (( lpBitsPtr[0] + lpBitsPtr[1] + lpBitsPtr[2] ) / 3) );

					if (lpBitsPtr[3] != 0 || off != 255)
					{
						if (off == 0)
						{
							off = 1;
						}

						lpBits[i] = pPalette[off] | ( lpBitsPtr[3] << 24 );
					}

					lpBitsPtr += 4;
				}

				icGrayInfo.hbmColor = ::CreateCompatibleBitmap(hdc, sz.cx, sz.cy);

				if (icGrayInfo.hbmColor != NULL)
				{
					::SetDIBits(hdc, icGrayInfo.hbmColor, 0, sz.cy, lpBits, &bmpInfo, DIB_RGB_COLORS);

					icGrayInfo.hbmMask = icInfo.hbmMask;
					icGrayInfo.fIcon   = TRUE;

					hGrayIcon = ::CreateIconIndirect(&icGrayInfo);

					::DeleteObject(icGrayInfo.hbmColor);
				}

				::GlobalFree(lpBits);
				lpBits = NULL;
			}
		}

		::DeleteObject(icInfo.hbmColor);
		::DeleteObject(icInfo.hbmMask);
	}

	::ReleaseDC(NULL,hdc);

	return hGrayIcon;
}

HICON CreateGrayscaleIcon( HICON hIcon )
{
	if (hIcon == NULL)
	{
		return NULL;
	}

	if (!bGrayPaletteSet)
	{
		for(int i = 0; i < 256; i++)
		{
			defaultGrayPalette[i] = RGB(255-i, 255-i, 255-i);
		}

		bGrayPaletteSet = TRUE;
	}

	return CreateGrayscaleIcon(hIcon, defaultGrayPalette);
}


#endif

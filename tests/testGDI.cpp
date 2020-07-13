#include <fstream>
#include <cstdint>
#include <iostream>
#include <chrono>
#include <string>

#include <conio.h>

#include "../jShot/src/GDIScreenShooter.h"
#include "../jShot/src/DXScreenShooter.h"
#include "../jShot/src/DDrawScreenShooter.h"

const WORD g_B = static_cast<WORD>('B');
const WORD g_M = static_cast<WORD>('M');
const WORD g_BM = g_B + (g_M << 8);


void writeBmpFile(const std::string& _filename, 
				  const std::vector<char>& _header, 
				  const std::vector<char>& _data)
{
	std::fstream outFile(_filename, std::ios::binary | std::ios::out | std::ios::trunc);
    outFile.write(_header.data(), _header.size());
    outFile.write(_data.data(), _data.size());
}

void makeBmp(const CRectangle& _region)
{
	std::vector<char> header(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	BITMAPFILEHEADER* pFHeader = reinterpret_cast<BITMAPFILEHEADER*>(header.data());
	BITMAPINFOHEADER* pInfoHeader = reinterpret_cast<BITMAPINFOHEADER*>(header.data() + sizeof(BITMAPFILEHEADER));


	CGDIScreenShooter scrShooter(_region);
	std::vector<char> body;
	if (!scrShooter.getScreenshot(_region, body))
	{
		std::cout << __FUNCTION__ << " getScreenshot failed" << std::endl;
		return;
	}
	scrShooter.getBitMapInfoHeader(*pInfoHeader);

	pFHeader->bfType = g_BM;
	pFHeader->bfSize = pInfoHeader->biSizeImage + header.size();
	pFHeader->bfReserved1 = 0;
	pFHeader->bfReserved2 = 0;
	pFHeader->bfOffBits = header.size();
	writeBmpFile("snapshot.bmp", header, body);
}

void makeBmpHeader(const CRectangle& _region, std::vector<char>& _header)
{
	_header.resize(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	BITMAPFILEHEADER* pFHeader = reinterpret_cast<BITMAPFILEHEADER*>(_header.data());
	BITMAPINFOHEADER* pInfoHeader = reinterpret_cast<BITMAPINFOHEADER*>(_header.data() + sizeof(BITMAPFILEHEADER));

	auto sizeimage = _region.getSize().m_x * _region.getSize().m_y * _region.getBytesPerPixel();
	pInfoHeader->biSize = sizeof(BITMAPINFOHEADER) - 
		sizeof(decltype(BITMAPINFOHEADER::biClrUsed)) - 
		sizeof(decltype(BITMAPINFOHEADER::biClrImportant));
	pInfoHeader->biWidth = _region.getSize().m_x;
	pInfoHeader->biHeight = _region.getSize().m_y;
	pInfoHeader->biPlanes = 1;
	pInfoHeader->biBitCount = _region.getBitsPerPixel();
	pInfoHeader->biCompression = 0;
	pInfoHeader->biSizeImage = sizeimage;
	pInfoHeader->biXPelsPerMeter = 0;
	pInfoHeader->biYPelsPerMeter = 0;
	pInfoHeader->biClrUsed = 0;
	pInfoHeader->biClrImportant = 0;

	pFHeader->bfType = g_BM;
	pFHeader->bfSize = pInfoHeader->biSizeImage + _header.size();
	pFHeader->bfReserved1 = 0;
	pFHeader->bfReserved2 = 0;
	pFHeader->bfOffBits = _header.size();
}

void testPerfomanceGDI(const CRectangle& _region)
{
	std::cout << "testPerfomanceGDI" << std::endl;
	CGDIScreenShooter scrShooter(_region);

	uint64_t numIterations = 0;

	auto startTime = std::chrono::system_clock::now();
	std::vector<char> buffer;
	for (numIterations = 0; _kbhit() == 0; numIterations++)
	{
		if (!scrShooter.getScreenshot(_region, buffer))
		{
			std::cout << __FUNCTION__ << " failed" << std::endl;
			return;
		}
	}
	auto endTime = std::chrono::system_clock::now();
	getch();

	std::chrono::duration<double> diffTime = endTime - startTime;
	std::cout << "Interval: \'" << diffTime.count() << "\' numIterations: \'" << numIterations << "\'" << std::endl;
}

void testPerfomanceDX(const CRectangle& _region)
{
	std::cout << "testPerfomanceDX" << std::endl;
	CDxScreenShooter dxScreenShooter;

	uint64_t numIterations = 0;
	auto startTime = std::chrono::system_clock::now();
	 
	std::vector<char> buffer;
	for (numIterations = 0; _kbhit() == 0; numIterations++)
	{
		dxScreenShooter.GetScreenShot(_region, buffer);

		/*
		// write the entire surface to the requested file 
		std::string fileName(std::string("screenshotDx") + std::to_string(numIterations) + ".bmp");
		//D3DXSaveSurfaceToFile(fileName.c_str(), D3DXIFF_BMP, m_surf, NULL, NULL);
		std::vector<char> header;
		makeBmpHeader(_region, header);
		writeBmpFile(fileName, header, buffer);
		*/
	}
	auto endTime = std::chrono::system_clock::now();
	getch();

	std::chrono::duration<double> diffTime = endTime - startTime;
	std::cout << "Interval: \'" << diffTime.count() << "\' numIterations: \'" << numIterations << "\'" << std::endl;
}

void testPerfomanceDDraw(const CRectangle& _region)
{
	std::cout << "testPerfomanceDDraw" << std::endl;
	CDDrawScreenShooter ddScreenShooter;

	std::vector<char> buffer;
	uint64_t numIterations = 0;
	auto startTime = std::chrono::system_clock::now();

	for (numIterations = 0; _kbhit() == 0; numIterations++)
	{
		ddScreenShooter.GetScreenShot(_region, buffer);
	}
	auto endTime = std::chrono::system_clock::now();
	getch();

	std::chrono::duration<double> diffTime = endTime - startTime;
	std::cout << "Interval: \'" << diffTime.count() << "\' numIterations: \'" << numIterations << "\'" << std::endl;
}

void main()
{
    CRectangle region(Vector2(0,0), Vector2(1920,1080), 4);
	

	makeBmp(region);
	testPerfomanceGDI(region);

	testPerfomanceDX(region);

	testPerfomanceDDraw(region);

}
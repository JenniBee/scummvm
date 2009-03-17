/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SCREEN_H
#define KYRA_SCREEN_H

#include "common/util.h"
#include "common/func.h"

class OSystem;

namespace Kyra {

typedef Common::Functor0<void> UpdateFunctor;

class KyraEngine_v1;

struct Rect {
	int x, y;
	int x2, y2;
};

struct ScreenDim {
	uint16 sx;
	uint16 sy;
	uint16 w;
	uint16 h;
	uint16 unk8;
	uint16 unkA;
	uint16 unkC;
	uint16 unkE;
};

struct Font {
	uint8 *fontData;
	uint8 *charWidthTable;
	uint16 charSizeOffset;
	uint16 charBitmapOffset;
	uint16 charWidthTableOffset;
	uint16 charHeightTableOffset;
};

class Screen {
public:
	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SCREEN_PAGE_SIZE = 320 * 200 + 1024,
		SCREEN_OVL_SJIS_SIZE = 640 * 400,
		SCREEN_PAGE_NUM = 16,
		SCREEN_OVLS_NUM = 4
	};

	enum CopyRegionFlags {
		CR_NO_P_CHECK = 0x01
	};

	enum DrawShapeFlags {
		DSF_X_FLIPPED  = 0x01,
		DSF_Y_FLIPPED  = 0x02,
		DSF_SCALE      = 0x04,
		DSF_WND_COORDS = 0x10,
		DSF_CENTER     = 0x20
	};

	enum FontId {
		FID_6_FNT = 0,
		FID_8_FNT,
		FID_9_FNT,
		FID_CRED6_FNT,
		FID_CRED8_FNT,
		FID_BOOKFONT_FNT,
		FID_GOLDFONT_FNT,
		FID_INTRO_FNT,
		FID_NUM
	};

	Screen(KyraEngine_v1 *vm, OSystem *system);
	virtual ~Screen();

	// init
	virtual bool init();
	virtual void setResolution();


	void updateScreen();

	// debug functions
	bool queryScreenDebug() const { return _debugEnabled; }
	bool enableScreenDebug(bool enable);

	// page cur. functions
	int setCurPage(int pageNum);

	void copyFromCurPageBlock(int x, int y, int w, int h, const uint8 *src);
	void copyCurPageBlock(int x, int y, int w, int h, uint8 *dst);

	void clearCurPage();

	// page 0 functions
	void copyToPage0(int y, int h, uint8 page, uint8 *seqBuf);
	void shakeScreen(int times);

	// page functions
	void copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags=0);
	void copyPage(uint8 srcPage, uint8 dstPage);

	void copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest);
	void copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src);

	void shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent);
	void fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum = -1, bool xored = false);

	void clearPage(int pageNum);

	uint8 getPagePixel(int pageNum, int x, int y);
	void setPagePixel(int pageNum, int x, int y, uint8 color);

	const uint8 *getCPagePtr(int pageNum) const;
	uint8 *getPageRect(int pageNum, int x, int y, int w, int h);

	// palette handling
	void fadeFromBlack(int delay=0x54, const UpdateFunctor *upFunc = 0);
	void fadeToBlack(int delay=0x54, const UpdateFunctor *upFunc = 0);

	void fadePalette(const uint8 *palData, int delay, const UpdateFunctor *upFunc = 0);
	virtual void getFadeParams(const uint8 *palette, int delay, int &delayInc, int &diff);
	int fadePalStep(const uint8 *palette, int diff);

	void setPaletteIndex(uint8 index, uint8 red, uint8 green, uint8 blue);
	void setScreenPalette(const uint8 *palData);
	const uint8 *getScreenPalette() const { return _screenPalette; }

	void getRealPalette(int num, uint8 *dst);
	uint8 *getPalette(int num);

	// gui specific (processing on _curPage)
	void drawLine(bool vertical, int x, int y, int length, int color);
	void drawClippedLine(int x1, int y1, int x2, int y2, int color);
	void drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2);
	void drawBox(int x1, int y1, int x2, int y2, int color);

	// font/text handling
	bool loadFont(FontId fontId, const char *filename);
	FontId setFont(FontId fontId);

	int getFontHeight() const;
	int getFontWidth() const;

	int getCharWidth(uint16 c) const;
	int getTextWidth(const char *str) const;

	void printText(const char *str, int x, int y, uint8 color1, uint8 color2);

	virtual void setTextColorMap(const uint8 *cmap) = 0;
	void setTextColor(const uint8 *cmap, int a, int b);

	virtual void setScreenDim(int dim) = 0;
	virtual const ScreenDim *getScreenDim(int dim) = 0;

	const ScreenDim *_curDim;

	// shape handling
	uint8 *encodeShape(int x, int y, int w, int h, int flags);

	int setNewShapeHeight(uint8 *shape, int height);
	int resetShapeHeight(uint8 *shape);

	void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...);

	// mouse handling
	void hideMouse();
	void showMouse();
	bool isMouseVisible() const;
	void setMouseCursor(int x, int y, const byte *shape);

	// rect handling
	virtual int getRectSize(int w, int h) = 0;

	void rectClip(int &x, int &y, int w, int h);

	// misc
	void loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData, bool skip=false);

	bool loadPalette(const char *filename, uint8 *palData);
	void loadPalette(const byte *data, uint8 *palData, int bytes);

	void setAnimBlockPtr(int size);

	void setShapePages(int page1, int page2, int minY = -1, int maxY = 201);

	virtual byte getShapeFlag1(int x, int y);
	virtual byte getShapeFlag2(int x, int y);

	virtual int getDrawLayer(int x, int y);
	virtual int getDrawLayer2(int x, int y, int height);

	void blockInRegion(int x, int y, int width, int height);
	void blockOutRegion(int x, int y, int width, int height);

	int _charWidth;
	int _charOffset;
	int _curPage;
	uint8 *_currentPalette;
	uint8 *_shapePages[2];
	int _maskMinY, _maskMaxY;
	FontId _currentFont;
	bool _disableScreen;

	// decoding functions
	static void decodeFrame3(const uint8 *src, uint8 *dst, uint32 size);
	static uint decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize);
	static void decodeFrameDelta(uint8 *dst, const uint8 *src, bool noXor = false);
	static void decodeFrameDeltaPage(uint8 *dst, const uint8 *src, const int pitch, bool noXor);
	static void convertAmigaGfx(uint8 *data, int w, int h, bool offscreen = true);
	static void convertAmigaMsc(uint8 *data);

protected:
	uint8 *getPagePtr(int pageNum);
	void updateDirtyRects();
	void updateDirtyRectsOvl();

	void scale2x(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h);
	void mergeOverlay(int x, int y, int w, int h);

	// overlay specific
	byte *getOverlayPtr(int pageNum);
	void clearOverlayPage(int pageNum);
	void clearOverlayRect(int pageNum, int x, int y, int w, int h);
	void copyOverlayRegion(int x, int y, int x2, int y2, int w, int h, int srcPage, int dstPage);

	// font/text specific
	void drawCharANSI(uint8 c, int x, int y);
	void drawCharSJIS(uint16 c, int x, int y);

	enum {
		SJIS_CHARSIZE = 18,
		SJIS_CHARS = 8192
	};

	int16 encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size);
	void restoreMouseRect();
	void copyMouseToScreen();
	void copyScreenFromRect(int x, int y, int w, int h, const uint8 *ptr);
	void copyScreenToRect(int x, int y, int w, int h, uint8 *ptr);

	template<bool noXor> static void wrapped_decodeFrameDelta(uint8 *dst, const uint8 *src);
	template<bool noXor> static void wrapped_decodeFrameDeltaPage(uint8 *dst, const uint8 *src, const int pitch);

	uint8 *_pagePtrs[16];
	uint8 *_sjisOverlayPtrs[SCREEN_OVLS_NUM];

	bool _useOverlays;
	bool _useSJIS;

	uint8 *_sjisFontData;
	uint8 *_sjisTempPage;
	uint8 *_sjisTempPage2;
	uint8 *_sjisSourceChar;
	uint8 _sjisInvisibleColor;

	uint8 *_screenPalette;
	uint8 *_palettes[6];

	Font _fonts[FID_NUM];
	uint8 _textColorsMap[16];

	uint8 *_decodeShapeBuffer;
	int _decodeShapeBufferSize;

	uint8 *_animBlockPtr;
	int _animBlockSize;

	int _mouseLockCount;

	enum {
		kMaxDirtyRects = 50
	};

	bool _forceFullUpdate;
	int _numDirtyRects;
	Rect *_dirtyRects;

	void addDirtyRect(int x, int y, int w, int h);

	OSystem *_system;
	KyraEngine_v1 *_vm;

	// shape
	int drawShapeMarginNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeMarginNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeMarginScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeMarginScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeSkipScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeSkipScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt);
	void drawShapeProcessLineNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, int scaleState);
	void drawShapeProcessLineNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, int scaleState);
	void drawShapeProcessLineScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, int scaleState);
	void drawShapeProcessLineScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, int scaleState);

	void drawShapePlotType0(uint8 *dst, uint8 cmd);
	void drawShapePlotType1(uint8 *dst, uint8 cmd);
	void drawShapePlotType3_7(uint8 *dst, uint8 cmd);
	void drawShapePlotType4(uint8 *dst, uint8 cmd);
	void drawShapePlotType5(uint8 *dst, uint8 cmd);
	void drawShapePlotType6(uint8 *dst, uint8 cmd);
	void drawShapePlotType8(uint8 *dst, uint8 cmd);
	void drawShapePlotType9(uint8 *dst, uint8 cmd);
	void drawShapePlotType11_15(uint8 *dst, uint8 cmd);
	void drawShapePlotType12(uint8 *dst, uint8 cmd);
	void drawShapePlotType13(uint8 *dst, uint8 cmd);
	void drawShapePlotType14(uint8 *dst, uint8 cmd);
	void drawShapePlotType37(uint8 *dst, uint8 cmd);
	void drawShapePlotType52(uint8 *dst, uint8 cmd);

	typedef int (Screen::*DsMarginSkipFunc)(uint8 *&dst, const uint8 *&src, int &cnt);
	typedef void (Screen::*DsLineFunc)(uint8 *&dst, const uint8 *&src, int &cnt, int scaleState);
	typedef void (Screen::*DsPlotFunc)(uint8 *dst, uint8 cmd);

	DsMarginSkipFunc _dsProcessMargin;
	DsMarginSkipFunc _dsScaleSkip;
	DsLineFunc _dsProcessLine;
	DsPlotFunc _dsPlot;

	const uint8 *_dsTable;
	int _dsTableLoopCount;
	const uint8 *_dsTable2;
	const uint8 *_dsTable3;
	const uint8 *_dsTable4;
	const uint8 *_dsTable5;
	int _dsDrawLayer;
	uint8 *_dsDstPage;
	int _dsTmpWidth;
	int _dsOffscreenLeft;
	int _dsOffscreenRight;
	int _dsScaleW;
	int _dsScaleH;
	int _dsOffscreenScaleVal1;
	int _dsOffscreenScaleVal2;
	int _drawShapeVar1;
	int _drawShapeVar3;
	int _drawShapeVar4;
	int _drawShapeVar5;

	// debug
	bool _debugEnabled;
};

} // End of namespace Kyra

#endif


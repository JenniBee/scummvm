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
 */
#include "common/debug.h"
#include "common/random.h"

#include "prince/hero.h"
#include "prince/hero_set.h"
#include "prince/animation.h"
#include "prince/resource.h"


namespace Prince {

Hero::Hero() : _number(0), _visible(false), _state(MOVE), _middleX(0), _middleY(0)
	, _boreNum(1), _currHeight(0), _moveDelay(0), _shadMinus(1), _moveSetType(0)
	, _lastDirection(DOWN), _destDirection(DOWN), _talkTime(0), _boredomTime(0), _phase(0)
	, _specAnim(0), _drawX(0), _drawY(0), _randomSource("prince"), _zoomFactor(0), _scaleValue(0)
{
	_zoomBitmap = new Animation();
}

Hero::~Hero() {
	delete _zoomBitmap;
}

bool Hero::loadAnimSet(uint32 animSetNr) {
	if (animSetNr > sizeof(heroSetTable)) {
		return false;
	}

	for (uint32 i = 0; i < _moveSet.size(); ++i) {
		delete _moveSet[i];
	}

	const HeroSetAnimNames &animSet = *heroSetTable[animSetNr];

	_moveSet.resize(kMoveSetSize);
	for (uint32 i = 0; i < kMoveSetSize; ++i) {
		debug("Anim set item %d %s", i, animSet[i]);
		Animation *anim = NULL;
		if (animSet[i] != NULL) {
			anim = new Animation();
			Resource::loadResource(anim, animSet[i]);
		}
		_moveSet[i] = anim;
	}

	return true;
}

const Graphics::Surface * Hero::getSurface() {
	if (_moveSet[_moveSetType]) {
		debug("BaseX: %d", _moveSet[_moveSetType]->getBaseX());
		debug("BaseY: %d", _moveSet[_moveSetType]->getBaseY());
		//debug("FrameCount: %d", _moveSet[_moveSetType]->getFrameCount());
		//debug("LoopCount: %d", _moveSet[_moveSetType]->getLoopCount());
		//debug("PhaseCount: %d", _moveSet[_moveSetType]->getPhaseCount());
		//debug("PhaseFrameIndex(%d): %d", _frame, _moveSet[_moveSetType]->getPhaseFrameIndex(_frame));
		//debug("PhaseOffsetX(%d): %d", _frame, _moveSet[_moveSetType]->getPhaseOffsetX(_frame));
		//debug("PhaseOffsetY(%d) %d", _frame, _moveSet[_moveSetType]->getPhaseOffsetY(_frame));
		//debug("FrameSizeX(%d) %d", _frame, _moveSet[_moveSetType]->getFrameWidth(_frame));
		//debug("FrameSizeY(%d) %d", _frame, _moveSet[_moveSetType]->getFrameHeight(_frame));
		//getState();
		int16 phaseFrameIndex = _moveSet[_moveSetType]->getPhaseFrameIndex(_phase);
		Graphics::Surface *heroFrame = _moveSet[_moveSetType]->getFrame(phaseFrameIndex);
		//return _moveSet[_moveSetType]->getFrame(phaseFrameIndex);

		return heroFrame;
	}
	return NULL;
}

//TEMP
void Hero::getState() {
	switch (_state) {
	case STAY:
		debug("STAY");
		break;
	case TURN:
		debug("TURN");
		break;
	case MOVE:
		debug("MOVE");
		break;
	case BORE:
		debug("BORE");
		break;
	case SPEC:
		debug("SPEC");
		break;
	case TALK:
		debug("TALK");
		break;
	case MVAN:
		debug("MVAN");
		break;
	case TRAN:
		debug("TRAN");
		break;
	case RUN:
		debug("RUN");
		break;
	case DMOVE:
		debug("DMOVE");
		break;
	}
}

int Hero::getScaledValue(int size) {
	int newSize = 0;
	int16 initScaleValue = _scaleValue;
	if(_scaleValue != 10000) {
		for(int i = 0; i < size; i++) {
			initScaleValue -= 100;
			if(initScaleValue >= 0) {
				newSize++;
			} else {
				initScaleValue += _scaleValue;
			}
		}
		return newSize;
	} else {
		return size;
	}
}

void Hero::checkNak() {

}

void Hero::zoomSprite(int16 tempMiddleY) {
	if(_zoomFactor == 0) {
		//notfullSize
		int sprWidth = _moveSet[_moveSetType]->getFrameWidth(_phase);
		int temp = sprWidth / 2;
		int sprFullHeight = _moveSet[_moveSetType]->getFrameHeight(_phase); // edx
		int sprModulo = tempMiddleY; // ebp ???
		int sprSkipX = 0;
		int sprSkipY = 0;

	} else {
		//fullSize

	}
}

//TODO
/*
void Hero::countDrawPosition() {
	int16 frameXSize = _moveSet[_moveSetType]->getFrameWidth(_phase);
	int16 frameYSize = _moveSet[_moveSetType]->getFrameHeight(_phase);
	_drawX = _middleX - frameXSize/2;
	_drawY = _middleY - frameYSize;
}
*/
void Hero::countDrawPosition() {
	int16 tempMiddleX;
	int16 tempMiddleY;
	int16 baseX = _moveSet[_moveSetType]->getBaseX();
	int16 baseY = _moveSet[_moveSetType]->getBaseY();
	// any chance?
	if(baseX == 320) {
		tempMiddleY = _middleY - (baseY - 240);
	} else {
		tempMiddleY = _middleY;
	}
	int16 frameXSize = _moveSet[_moveSetType]->getFrameWidth(_phase);
	int16 frameYSize = _moveSet[_moveSetType]->getFrameHeight(_phase);
	int scaledX = getScaledValue(frameXSize); // ebx
	int scaledY = getScaledValue(frameYSize); // edx
	int tempHeroHeight = scaledY; // not used? global?
	
	int width = scaledX / 2;
	tempMiddleX = _middleX - width; //eax
	int z = _middleY; //ebp
	int y = _middleY - scaledY; //ecx
	//TODO
	checkNak();

	//zoomSprite(tempMiddleY);
	// zoomSprite:
	int sprModulo;
	if(_zoomFactor != 0) {
		//notfullSize
		int sprFullHeight = frameXSize;
		sprModulo = tempMiddleY; // ebp ???
		//int sprSkipX = 0;
		//int sprSkipY = 0;
		int sprWidth = scaledX;
		int sprHeight = scaledY; // after resize
		int allocMemSize = sprWidth*sprHeight;
		sprFullHeight--;
		int imulEDX = sprFullHeight*sprModulo; // edx
		int sprZoomX;
		int sprZoomY = _scaleValue;
		int loop_lin;

		for(int i = 0; i < sprHeight; i++) {
			// linear_loop:
			while(1) {
				sprZoomY -= 100;
				if(sprZoomY >= 0 || _scaleValue == 10000) {
					// all_r_y
					// push esi
					loop_lin = sprWidth;
					// mov ah, -1
					sprZoomX = _scaleValue;
					break; // to loop_lin
				} else {
					sprZoomY += _scaleValue;
					// add esi, sprModulo /?
				}
			}
			// end of linear_loop
			// loop_lin:
			for(int i = 0; i < loop_lin; i++) {
				// mov al, b[esi]
				// inc esi
				sprZoomX -= 100;
				if(sprZoomX >= 0) {
					//its_all_r
					// without ZOOMFIX
					//mov [edi], al
					// inc edi
				} else {
					sprZoomX += _scaleValue;
				}
			}
			//pop esi
			//add esi, sprModulo
			debug("loop_lin: %d", loop_lin);
		}
		debug("scaledX: %d", scaledX);
		debug("scaledY: %d", scaledY);
		_drawX = _middleX - frameXSize/2;
		_drawY = _middleY - frameYSize;
	} else {
		//fullSize
		sprModulo = 0; //?
		_drawX = _middleX - frameXSize / 2;
		_drawY = tempMiddleY + 1 - frameYSize;
	}

	// ShowSprite
}
/*
AnimHeader	struc		;struktura naglowka pliku z animacja
AH_ID		dw	0	;ID = "AN"
AH_Loop		dw	0	;numer fazy do petli
AH_Fazy		dw	0	;ilosc faz animacji
AH_Ramki	dw	0	;ilosc ramek grafiki
AH_X		dw	0	;poczatkowa wsp�lrzedna X
AH_Y		dw	0	;poczatkowa wsp�lrzedna Y
AH_Tablica	dd	0	;offset tablicy faz
AH_RamkiAddr	dd	0	;poczatek tablicy z offsetami ramek
*/
void Hero::showHeroAnimFrame() {
	if (_phase < _moveSet[_moveSetType]->getFrameCount() - 1) {
		_phase++;
	} else {
		_phase = 0;
	}
	countDrawPosition();
	//debug("_drawX: %d", _drawX);
	//debug("_drawY: %d", _drawY);
	//debug("_middleX: %d", _middleX);
	//debug("_middleY: %d", _middleY);
}

void Hero::setScale(int8 zoomBitmapValue) {
	if (zoomBitmapValue == 0) {
		_zoomFactor = 0;
		_scaleValue = 10000;
	} else {
		_zoomFactor = zoomBitmapValue;
		_scaleValue = 10000 / _zoomFactor;
	}
	debug("_zoomFactor: %d", _zoomFactor);
	debug("_scaleValue: %d", _scaleValue);
}

void Hero::selectZoom() {
	int8 zoomBitmapValue = _zoomBitmap->getZoom(_middleY / 4 * kZoomBitmapWidth + _middleX / 4);
	debug("offset: %d", _middleY / 4 * kZoomBitmapWidth + _middleX / 4);
	debug("zoomBitmapValue: %d", _zoomFactor);
	setScale(zoomBitmapValue);
}

void Hero::specialAnim() {
}

void Hero::rotateHero() {
	switch (_lastDirection) {
	case LEFT:
		switch (_destDirection) {
		case RIGHT:
			_moveSetType = Move_MLR;
			break;
		case UP:
			_moveSetType = Move_MLU;
			break;
		case DOWN:
			_moveSetType = Move_MLD;
			break;
		}
		break;
	case RIGHT:
		switch (_destDirection) {
		case LEFT:
			_moveSetType = Move_MRL;
			break;
		case UP:
			_moveSetType = Move_MRU;
			break;
		case DOWN:
			_moveSetType = Move_MRD;
			break;
		}
		break;
	case UP:
		switch (_destDirection) {
		case LEFT:
			_moveSetType = Move_MUL;
			break;
		case RIGHT:
			_moveSetType = Move_MUR;
			break;
		case DOWN:
			_moveSetType = Move_MUD;
			break;
		}
		break;
	case DOWN:
		switch (_destDirection) {
		case LEFT:
			_moveSetType = Move_MDL;
			break;
		case RIGHT:
			_moveSetType = Move_MDR;
			break;
		case UP:
			_moveSetType = Move_MDU;
			break;
		}
		break;
	}
}

void Hero::showHero() {
	if (_visible) {
		// Is he talking?
		if (_talkTime == 0) { //?
			// Scale of hero
			selectZoom();
			switch (_state) {
			case STAY:
				//if(OptionsFlag == false) {
				//if(OpcodePC == null) {
				_boredomTime++;
				if (_boredomTime == 200) { // 140 for second hero
					_boredomTime = 0;
					_state = BORE;
				}
				switch (_lastDirection) {
				case LEFT:
					_moveSetType = Move_SL;
					break;
				case RIGHT:
					_moveSetType = Move_SR;
					break;
				case UP:
					_moveSetType = Move_SU;
					break;
				case DOWN:
					_moveSetType = Move_SD;
					break;
				}
				break;
			case TURN:
				/*
				if(_lastDirection == _destDirection) {
					_state = STAY;
				} else {
					_frame = 0;
					rotateHero();
					_lastDirection = _destDirection;
				}
				*/
				break;
			case MOVE:
				switch (_lastDirection) {
				case LEFT:
					_moveSetType = Move_ML;
					break;
				case RIGHT:
					_moveSetType = Move_MR;
					break;
				case UP:
					_moveSetType = Move_MU;
					break;
				case DOWN:
					_moveSetType = Move_MD;
					break;
				}
				break;
			case BORE:
				//if (_direction == UP) {
				switch (_boreNum) {
				case 0:
					_moveSetType = Move_BORED1;
					break;
				case 1:
					_moveSetType = Move_BORED2;
					break;
				}
				if (_phase == _moveSet[_moveSetType]->getFrameCount() - 1) {
					_boreNum = _randomSource.getRandomNumber(1); // rand one of two 'bored' animation
					_lastDirection = DOWN;
					_state = STAY;
				}
				break;
			case SPEC:
				//specialAnim();
				break;
			case TALK:
				switch (_lastDirection) {
				case LEFT:
					_moveSetType = Move_TL;
					break;
				case RIGHT:
					_moveSetType = Move_TR;
					break;
				case UP:
					_moveSetType = Move_TU;
					break;
				case DOWN:
					_moveSetType = Move_TD;
					break;
				}
				break;
			case TRAN:
				break;
			case RUN:
				break;
			case DMOVE:
				break;
			}
		} else {
			_talkTime--; // o ile?
		}
		showHeroAnimFrame();
	} else {
		// no hero visible
		return;
	}
}
}

/* vim: set tabstop=4 noexpandtab: */

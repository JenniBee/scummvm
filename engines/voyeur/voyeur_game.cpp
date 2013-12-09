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

#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"

namespace Voyeur {

void VoyeurEngine::addVideoEventStart() {
	VoyeurEvent &e = _voy._events[_voy._evidence[19]];
	e._computerNum = _eventsManager._videoComputerNum;
	e._computerBut[0] = _eventsManager._videoComputerBut1;
	e._computerBut[1] = _voy._delaySecs;
	e._computerBut[2] = 1;
	e._computerBut[3] = _eventsManager._videoComputerBut4;
	e._dead = _eventsManager._videoDead;
}

void VoyeurEngine::playStamp() {
	_stampLibPtr = NULL;
	_filesManager.openBoltLib("stampblt.blt", _stampLibPtr);

	_stampLibPtr->getBoltGroup(0x10000);
	_voy._resolvePtr = &RESOLVE_TABLE[0];
	initStamp();

	PtrResource *threadsList = _stampLibPtr->boltEntry(3)._ptrResource;
	ThreadResource *threadP = threadsList->_entries[0]->_threadResource;
	initThreadStruct(threadP, 0, 0);

	_voy._delaySecs = 0;
	_eventsManager._videoComputerNum = 9;
	_eventsManager._videoComputerBut1 = 0;
	_eventsManager._v2A0A2 = 0;
	_voy._eCursorOff[53] = 1;

	bool breakFlag = false;
	while (!breakFlag && !shouldQuit()) {
		breakFlag = true;
	}

	_voy._field4386 = 0;
	closeStamp();
	_stampLibPtr->freeBoltGroup(0x10000);
	delete _stampLibPtr;
}

void VoyeurEngine::initStamp() {
	ThreadResource::_stampFlags &= ~1;
	_stackGroupPtr = _controlGroupPtr;

	if (!_controlPtr->_entries[0])
		error("No control entries");

	ThreadResource::initUseCount();
}

void VoyeurEngine::closeStamp() {
	ThreadResource::unloadAllStacks(this);
}

void VoyeurEngine::initThreadStruct(ThreadResource *thread, int idx, int v3) {
	thread->_controlIndex = -1;
	if (thread->loadAStack(idx)) {
		thread->_field4 = thread->_field6 = -1;
		thread->_field0 = v3;
		thread->_field3A = -1;
		thread->_field3E = -1;

		thread->doState();
	}
}

} // End of namespace Voyeur

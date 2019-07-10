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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hdb/hdb.h"

namespace HDB {

Common::Error HDBGame::saveGameState(int slot, const Common::String &desc) {

	// If no map is loaded, don't try to save
	if (!g_hdb->_map->isLoaded())
		return Common::kCreatingFileFailed;

	Common::OutSaveFile *out;

	Common::String saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	if (!(out = _saveFileMan->openForSaving(saveFileName)))
		error("Unable to open save file");

	Graphics::saveThumbnail(*out);

	// Actual Save Data
	saveGame(out);
	_lua->save(out, _targetName.c_str(), slot);

	out->finalize();
	if (out->err())
		warning("Can't write file '%s'. (Disk full?)", saveFileName.c_str());

	delete out;

	return Common::kNoError;
}

Common::Error HDBGame::loadGameState(int slot) {
	Common::InSaveFile *in;

	Common::String saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		warning("missing savegame file %s", saveFileName.c_str());
		if (g_hdb->_map->isLoaded())
			g_hdb->setGameState(GAME_PLAY);
		return Common::kReadingFailed;
	}

	Graphics::skipThumbnail(*in);

	// Actual Save Data
	loadGame(in);

	_lua->loadLua(_currentLuaName); // load the Lua code FIRST! (if no file, it's ok)

	saveFileName = Common::String::format("%s.l.%03d", _targetName.c_str(), slot);
	_lua->loadSaveFile(in, saveFileName.c_str());

	delete in;

	return Common::kNoError;
}

void HDBGame::saveGame(Common::OutSaveFile *out) {
	debug(1, "HDBGame::saveGame: start at %u", out->pos());

	// Save Map Name and Time
	out->writeUint32LE(_timeSeconds + (_timePlayed / 1000));
	out->write(_inMapName, 32);

	debug(1, "HDBGame::saveGame: map at %u", out->pos());
	// Save Map Object Data
	_map->save(out);

	// Save Window Object Data
	debug(1, "HDBGame::saveGame: window at %u", out->pos());
	_window->save(out);

	// Save Gfx Object Data
	debug(1, "HDBGame::saveGame: gfx at %u", out->pos());
	_gfx->save(out);

	// Save Sound Object Data
	debug(1, "HDBGame::saveGame: sound at %u", out->pos());
	_sound->save(out);

	// Save Game Object Data
	debug(1, "HDBGame::saveGame: game object at %u", out->pos());
	save(out);

	// Save AI Object Data
	debug(1, "HDBGame::saveGame: ai at %u", out->pos());
	_ai->save(out);

	debug(1, "HDBGame::saveGame: end at %u", out->pos());
}

void HDBGame::loadGame(Common::InSaveFile *in) {
	debug(1, "HDBGame::loadGame: start at %u", in->pos());

	// Load Map Name and Time
	_timeSeconds = in->readUint32LE();;
	_timePlayed = 0;
	in->read(_inMapName, 32);

	g_hdb->_sound->stopMusic();

	// Load Map Object Data
	debug(1, "HDBGame::loadGame: map at %u", in->pos());
	_map->loadSaveFile(in);

	// Load Window Object Data
	debug(1, "HDBGame::loadGame: window at %u", in->pos());
	_window->loadSaveFile(in);

	// Load Gfx Object Data
	debug(1, "HDBGame::loadGame: gfx at %u", in->pos());
	_gfx->loadSaveFile(in);

	// Load Sound Object Data
	debug(1, "HDBGame::loadGame: sound at %u", in->pos());
	_sound->loadSaveFile(in);

	// Load Game Object Data
	debug(1, "HDBGame::loadGame: game object at %u", in->pos());
	loadSaveFile(in);

	// Load AI Object Data
	debug(1, "HDBGame::loadGame: ai at %u", in->pos());
	_ai->loadSaveFile(in);

	debug(1, "HDBGame::loadGame: end at %u", in->pos());

	_gfx->turnOffFade();
}

} // End of Namespace
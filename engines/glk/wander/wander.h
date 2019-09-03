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

#ifndef GLK_WANDER_WANDER
#define GLK_WANDER_WANDER

#include "common/scummsys.h"
#include "glk/glk_api.h"

namespace Glk {
namespace Wander {

/**
 * Wander game interpreter
 */
class Wander : public GlkAPI {
private:

public:
	/**
	 * Constructor
	 */
	Wander(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc) {}

	/**
	 * Run the game
	 */
	virtual void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_WANDER; }

	/**
	 * Load a savegame from the passed stream
	 */
	virtual Common::Error loadGameData(strid_t save) override;

	/**
	 * Save the game to the passed stream
	 */
	virtual Common::Error saveGameData(strid_t save, const Common::String &desc) override;
};

} // End of namespace Wander
} // End of namespace Glk

#endif
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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "moai/moai.h"

namespace Moai {

struct MoaiGameDescription {
	ADGameDescription desc;

	MoaiGameID gameID;
	uint16 version;
};

MoaiGameID MoaiEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform MoaiEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MoaiEngine::getVersion() const {
	return _gameDescription->version;
}

Common::Language MoaiEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool MoaiEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

} // End of Namespace Moai

static const PlainGameDescriptor moaiGames[] = {
	{ "moai", "Moai Game" },
	{ "doug", "Doug's Moai Adventure" },
	{ 0, 0 }
};

#include "moai/detection_tables.h"

class MoaiMetaEngine : public AdvancedMetaEngine {
public:
	MoaiMetaEngine() : AdvancedMetaEngine(Moai::gameDescriptions, sizeof(Moai::MoaiGameDescription), moaiGames) {
		_singleId = "moai";
	}

	virtual const char *getName() const {
		return "Moai";
	}

	virtual const char *getOriginalCopyright() const {
		return "Moai (C) Zipline Games";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool MoaiMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Moai::MoaiGameDescription *gd = (const Moai::MoaiGameDescription *)desc;

	if (gd)
		*engine = new Moai::MoaiEngine(syst, gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(MOAI)
	REGISTER_PLUGIN_DYNAMIC(MOAI, PLUGIN_TYPE_ENGINE, MoaiMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MOAI, PLUGIN_TYPE_ENGINE, MoaiMetaEngine);
#endif

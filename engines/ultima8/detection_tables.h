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

namespace Ultima8 {

static const Ultima8GameDescription gameDescriptions[] = {
	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("eusecode.flx", "c61f1dacde591cb39d452264e281f234", 1251108),
			Common::EN_ANY,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("fusecode.flx", "4017eb8678ee24af0ce8c7647a05509b", 1300957),
			Common::FR_FRA,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("gusecode.flx", "d69599a46870b66c1b7c02710ed185bd", 1378604),
			Common::DE_DEU,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Hopkins

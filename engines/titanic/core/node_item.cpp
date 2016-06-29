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

#include "titanic/core/node_item.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CNodeItem, CNamedItem)

CNodeItem::CNodeItem() : CNamedItem(), _nodeNumber(0) {
}

void CNodeItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeQuotedLine("N", indent);
	file->writeNumberLine(_nodePos.x, indent + 1);
	file->writeNumberLine(_nodePos.y, indent + 1);

	file->writeQuotedLine("N", indent);
	file->writeNumberLine(_nodeNumber, indent + 1);

	CNamedItem::save(file, indent);
}

void CNodeItem::load(SimpleFile *file) {
	file->readNumber();
	file->readBuffer();
	_nodePos.x = file->readNumber();
	_nodePos.y = file->readNumber();

	file->readBuffer();
	_nodeNumber = file->readNumber();

	CNamedItem::load(file);
}

} // End of namespace Titanic

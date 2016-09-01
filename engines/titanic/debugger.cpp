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

#include "titanic/debugger.h"
#include "titanic/titanic.h"
#include "titanic/core/tree_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

Debugger::Debugger(TitanicEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("continue",		WRAP_METHOD(Debugger, cmdExit));
	registerCmd("dump",			WRAP_METHOD(Debugger, cmdDump));
	registerCmd("room",			WRAP_METHOD(Debugger, cmdRoom));
	registerCmd("pet",			WRAP_METHOD(Debugger, cmdPET));
	registerCmd("item",			WRAP_METHOD(Debugger, cmdItem));
}

int Debugger::strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

CRoomItem *Debugger::findRoom(const char *name) {
	CTreeItem *root = g_vm->_window->_gameManager->_project;
	CRoomItem *roomItem = dynamic_cast<CRoomItem *>(root->findByName(name));
	if (roomItem)
		return roomItem;

	int roomNumber = strToInt(name);
	for (CTreeItem *treeItem = root; treeItem; treeItem = treeItem->scan(root)) {
		roomItem = dynamic_cast<CRoomItem *>(treeItem);
		if (roomItem && roomItem->_roomNumber == roomNumber)
			return roomItem;
	}

	return nullptr;
}

CNodeItem *Debugger::findNode(CRoomItem *room, const char *name) {
	CNodeItem *nodeItem = dynamic_cast<CNodeItem *>(room->findByName(name));
	if (nodeItem)
		return nodeItem;

	int nodeNumber = strToInt(name);
	nodeItem = dynamic_cast<CNodeItem *>(room->findChildInstanceOf(CNodeItem::_type));
	while (nodeItem) {
		if (nodeItem->_nodeNumber == nodeNumber)
			return nodeItem;

		nodeItem = dynamic_cast<CNodeItem *>(room->findNextInstanceOf(CNodeItem::_type, nodeItem));
	}

	return nullptr;
}

CViewItem *Debugger::findView(CNodeItem *node, const char *name) {
	CViewItem *viewItem = dynamic_cast<CViewItem *>(node->findByName(name));
	if (viewItem)
		return viewItem;

	int viewNumber = strToInt(name);
	viewItem = dynamic_cast<CViewItem *>(node->findChildInstanceOf(CViewItem::_type));
	while (viewItem) {
		if (viewItem->_viewNumber == viewNumber)
			return viewItem;

		viewItem = dynamic_cast<CViewItem *>(node->findNextInstanceOf(CViewItem::_type, viewItem));
	}

	return nullptr;
}

void Debugger::listRooms() {
	CGameManager &gm = *g_vm->_window->_gameManager;
	CTreeItem *root = gm._project;
	CViewItem *view = gm._gameState._gameLocation.getView();
	CNodeItem *node = gm._gameState._gameLocation.getNode();
	CRoomItem *room = gm._gameState._gameLocation.getRoom();
	debugPrintf("Current location: %s, %s, %s\n", room->getName().c_str(),
		node->getName().c_str(), view->getName().c_str());
	
	debugPrintf("Available rooms:\n");
	for (CTreeItem *treeItem = root; treeItem; treeItem = treeItem->scan(root)) {
		CRoomItem *roomItem = dynamic_cast<CRoomItem *>(treeItem);
		if (roomItem)
			debugPrintf("%d - %s\n", roomItem->_roomNumber, roomItem->_name.c_str());
	}
}

void Debugger::listRoom(CRoomItem *room) {
	for (CTreeItem *treeItem = room; treeItem; treeItem = treeItem->scan(room)) {
		CNodeItem *nodeItem = dynamic_cast<CNodeItem *>(treeItem);
		if (nodeItem)
			debugPrintf("%s\n", nodeItem->_name.c_str());
	}
}

void Debugger::listNode(CNodeItem *node) {
	for (CTreeItem *treeItem = node; treeItem; treeItem = treeItem->scan(node)) {
		CViewItem *viewItem = dynamic_cast<CViewItem *>(treeItem);
		if (viewItem)
			debugPrintf("%s\n", viewItem->_name.c_str());
	}
}

bool Debugger::cmdDump(int argc, const char **argv) {
	// Get the starting node
	CTreeItem *root = g_vm->_window->_gameManager->_project;
	if (argc == 2)
		root = root->findByName(argv[1]);
	
	if (root == nullptr) {
		debugPrintf("Could not find item\n");
	} else {
		root->dump(0);
		debugPrintf("Item and it's content were dumped to stdout\n");
	}

	return true;
}

bool Debugger::cmdRoom(int argc, const char **argv) {
	if (argc == 1) {
		listRooms();
	} else if (argc >= 2) {
		CRoomItem *roomItem = findRoom(argv[1]);

		if (!roomItem)
			debugPrintf("Could not find room - %s\n", argv[1]);
		else if (argc == 2)
			listRoom(roomItem);
		else {
			CNodeItem *nodeItem = findNode(roomItem, argv[2]);

			if (!nodeItem)
				debugPrintf("Could not find node - %s\n", argv[2]);
			else if (argc == 3)
				listNode(nodeItem);
			else {
				CViewItem *viewItem = findView(nodeItem, argv[3]);

				if (!viewItem) {
					debugPrintf("Could not find view - %s\n", argv[3]);
				} else {
					// Change to the specified view
					g_vm->_window->_gameManager->_gameState.changeView(viewItem, nullptr);
					return false;
				}
			}
		}
	}

	return true;
}

bool Debugger::cmdPET(int argc, const char **argv) {
	CGameManager &gameManager = *g_vm->_window->_gameManager;
	CGameState &gameState = gameManager._gameState;

	if (argc == 2) {
		CString s(argv[1]);
		s.toLowercase();

		if (s == "on") {
			gameState._petActive = true;
			gameManager.initBounds();
			debugPrintf("PET is now on\n");
			return true;
		} else if (s == "off") {
			gameState._petActive = false;
			gameManager.update();
			debugPrintf("PET is now off\n");
			return true;
		}
	}
	
	debugPrintf("%s [on | off]\n", argv[0]);
	return true;
}

bool Debugger::cmdItem(int argc, const char **argv) {
	CGameManager &gameManager = *g_vm->_window->_gameManager;
	CGameState &gameState = gameManager._gameState;

	if (argc == 1) {
		// No parameters, so list the available items
		debugPrintf("item [<name> [ add ]]\n");
		for (int idx = 0; idx < 40; ++idx)
			debugPrintf("%s\n", g_vm->_itemIds[idx].c_str());
	} else {
		// Ensure the specified name is a valid inventory item
		int itemIndex;
		for (itemIndex = 0; itemIndex < 40; ++itemIndex) {
			if (g_vm->_itemIds[itemIndex] == argv[1])
				break;
		}
		if (itemIndex == 40) {
			debugPrintf("Could not find item with that name\n");
			return true;
		}

		// Get the item
		CCarry *item = dynamic_cast<CCarry *>(
			g_vm->_window->_project->findByName(argv[1]));
		assert(item);

		if (argc == 2) {
			// List it's details
			CTreeItem *treeItem = item;
			CString fullName;
			while ((treeItem = treeItem->getParent()) != nullptr) {
				if (!treeItem->getName().empty())
					fullName = treeItem->getName() + "." + fullName;
			}

			debugPrintf("Current location: %s\n", fullName.c_str());
		} else if (CString(argv[2]) == "add") {
			// Ensure the PET is active and add the item to the inventory
			gameState._petActive = true;
			gameManager.initBounds();
			item->petAddToInventory();

			return false;
		} else {
			debugPrintf("Unknown command\n");
		}
	}

	return true;
}

} // End of namespace Titanic

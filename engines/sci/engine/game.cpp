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

#include "common/system.h"
#include "common/file.h"

#include "sci/scicore/resource.h"
#include "sci/engine/state.h"
#include "sci/scicore/versions.h"
#include "sci/engine/kernel.h"
#include "sci/engine/kernel_types.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/menubar.h"

namespace Sci {

// Structures and data from vm.c:
extern CallsStruct *send_calls;
extern int send_calls_allocated;
extern int bp_flag;

static int _init_vocabulary(EngineState *s) { // initialize vocabulary and related resources
	s->parser_lastmatch_word = SAID_NO_MATCH;
	s->parser_rules = NULL;

	sciprintf("Initializing vocabulary\n");

	if ((s->resmgr->_sciVersion < SCI_VERSION_01_VGA) && (s->parser_words = vocab_get_words(s->resmgr, &(s->parser_words_nr)))) {
		s->parser_suffices = vocab_get_suffices(s->resmgr, &(s->parser_suffices_nr));
		if ((s->parser_branches = vocab_get_branches(s->resmgr, &(s->parser_branches_nr))))
			// Now build a GNF grammar out of this
			s->parser_rules = vocab_build_gnf(s->parser_branches, s->parser_branches_nr);
	} else {
		sciprintf("Assuming that this game does not use a parser.\n");
		s->parser_rules = NULL;
	}

	s->opcodes = vocabulary_get_opcodes(s->resmgr);

	if (!vocabulary_get_snames(s->resmgr, s->version, s->_selectorNames)) {
		sciprintf("_init_vocabulary(): Could not retrieve selector names (vocab.997)!\n");
		return 1;
	}

	script_map_selectors(s, &(s->selector_map));
	// Maps a few special selectors for later use

	return 0;
}

extern int _allocd_rules;

static void _free_vocabulary(EngineState *s) {
	sciprintf("Freeing vocabulary\n");

	if (s->parser_words) {
		vocab_free_words(s->parser_words, s->parser_words_nr);
		vocab_free_suffices(s->resmgr, s->parser_suffices, s->parser_suffices_nr);
		vocab_free_branches(s->parser_branches);
		vocab_free_rule_list(s->parser_rules);
	}

	s->_selectorNames.clear();
	vocabulary_free_knames(s->kernel_names);
	vocabulary_free_opcodes(s->opcodes);
	s->opcodes = NULL;

	s->kernel_names = NULL;
	s->opcodes = NULL;
}


static int _init_graphics_input(EngineState *s) {
	s->pic_priority_table = NULL;
	s->pics = NULL;
	s->pics_nr = 0;
	return 0;
}

static void _sci1_alloc_system_colors(EngineState *s) {
	gfx_color_t white;
	gfx_color_t black;

	white.visual.global_index = 255;
	white.visual.r = white.visual.g = white.visual.b = 255;
	white.alpha = 0;
	white.priority = white.control = 0;
	white.mask = GFX_MASK_VISUAL;
	gfxop_set_system_color(s->gfx_state, &white);

	black.visual.global_index = 0;
	black.visual.r = black.visual.g = black.visual.b = 0;
	black.alpha = 0;
	black.priority = black.control = 0;
	black.mask = GFX_MASK_VISUAL;
	gfxop_set_system_color(s->gfx_state, &black);
}

int _reset_graphics_input(EngineState *s) {
	Resource *resource;
	int font_nr;
	gfx_color_t transparent = { { -1, 0, 0, 0 }, 0, -1, -1, 0 };
	sciprintf("Initializing graphics\n");

	if (s->resmgr->_sciVersion <= SCI_VERSION_01) {
		int i;

		for (i = 0; i < 16; i++) {
			if (gfxop_set_color(s->gfx_state, &(s->ega_colors[i]), gfx_sci0_image_colors[sci0_palette][i].r,
					gfx_sci0_image_colors[sci0_palette][i].g, gfx_sci0_image_colors[sci0_palette][i].b, 0, -1, -1)) {
				return 1;
			}
			gfxop_set_system_color(s->gfx_state, &(s->ega_colors[i]));
		}
	} else {
		// Check for Amiga palette file.
		Common::File file;
		if (file.open("spal")) {
			s->gfx_state->resstate->static_palette = gfxr_read_pal1_amiga(&s->gfx_state->resstate->static_palette_entries, file);
			file.close();
			_sci1_alloc_system_colors(s);
		} else {
			resource = s->resmgr->findResource(kResourceTypePalette, 999, 1);
			if (resource) {
				if (s->version < SCI_VERSION(1, 001, 000))
					s->gfx_state->resstate->static_palette = gfxr_read_pal1(999, &s->gfx_state->resstate->static_palette_entries,
																			resource->data, resource->size);
				else
					s->gfx_state->resstate->static_palette = gfxr_read_pal11(999, &s->gfx_state->resstate->static_palette_entries,
																			resource->data, resource->size);
				_sci1_alloc_system_colors(s);
				s->resmgr->unlockResource(resource, 999, kResourceTypePalette);
			} else {
				sciprintf("Couldn't find the default palette!\n");
			}
		}
	}

	gfxop_fill_box(s->gfx_state, gfx_rect(0, 0, 320, 200), s->ega_colors[0]); // Fill screen black
	gfxop_update(s->gfx_state);

	s->mouse_pointer_view = s->mouse_pointer_loop = s->mouse_pointer_cel = -1; // No mouse pointer resource
	s->save_mouse_pointer_view = s->save_mouse_pointer_loop = s->save_mouse_pointer_cel = -1; // No mouse pointer resource
	gfxop_set_pointer_position(s->gfx_state, Common::Point(160, 150));

	s->mouse_pointer_view = s->mouse_pointer_loop = s->mouse_pointer_cel = -1; // No mouse pointer resource
	s->save_mouse_pointer_view = s->save_mouse_pointer_loop = s->save_mouse_pointer_cel = -1; // No mouse pointer resource


	s->pic_is_new = 0;
	s->pic_visible_map = GFX_MASK_NONE; // Other values only make sense for debugging
	s->dyn_views = NULL; // no DynViews
	s->drop_views = NULL; // And, consequently, no list for dropped views

	s->priority_first = 42; // Priority zone 0 ends here

	if (s->version < SCI_VERSION_FTU_PRIORITY_14_ZONES)
		s->priority_last = 200;
	else
		s->priority_last = 190;

	font_nr = -1;
	do {
		resource = s->resmgr->testResource(kResourceTypeFont, ++font_nr);
	} while ((!resource) && (font_nr < sci_max_resource_nr[s->resmgr->_sciVersion]));

	if (!resource) {
		sciprintf("No text font was found.\n");
		return 1;
	}

	s->visual = gfxw_new_visual(s->gfx_state, font_nr);

	s->wm_port = gfxw_new_port(s->visual, NULL, s->gfx_state->options->pic_port_bounds, s->ega_colors[0], transparent);
	s->iconbar_port = gfxw_new_port(s->visual, NULL, gfx_rect(0, 0, 320, 200), s->ega_colors[0], transparent);
	s->iconbar_port->flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	if (s->resmgr->_sciVersion >= SCI_VERSION_01_VGA) {
		// This bit sets the foreground and background colors in VGA SCI games
		gfx_color_t fgcolor;
		gfx_color_t bgcolor;
		memset(&fgcolor, 0, sizeof(gfx_color_t));
		memset(&bgcolor, 0, sizeof(gfx_color_t));

#if 0
		fgcolor.visual = s->gfx_state->resstate->static_palette[0];
		fgcolor.mask = GFX_MASK_VISUAL;
		bgcolor.visual = s->gfx_state->resstate->static_palette[255];
		bgcolor.mask = GFX_MASK_VISUAL;
#endif
		s->titlebar_port = gfxw_new_port(s->visual, NULL, gfx_rect(0, 0, 320, 10), fgcolor, bgcolor);
	} else {
		s->titlebar_port = gfxw_new_port(s->visual, NULL, gfx_rect(0, 0, 320, 10), s->ega_colors[0], s->ega_colors[15]);
	}
	s->titlebar_port->color.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->color.priority = 11;
	s->titlebar_port->bgcolor.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->bgcolor.priority = 11;
	s->titlebar_port->flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	// but this is correct
	s->picture_port = gfxw_new_port(s->visual, NULL, s->gfx_state->options->pic_port_bounds, s->ega_colors[0], transparent);

	s->pics_drawn_nr = 0;

	s->visual->add(GFXWC(s->visual), GFXW(s->wm_port));
	s->visual->add(GFXWC(s->visual), GFXW(s->titlebar_port));
	s->visual->add(GFXWC(s->visual), GFXW(s->picture_port));
	s->visual->add(GFXWC(s->visual), GFXW(s->iconbar_port));
	// Add ports to visual

	s->port = s->picture_port; // Currently using the picture port

#if 0
	s->titlebar_port->bgcolor.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->bgcolor.priority = 11; // Standard priority for the titlebar port
#endif

	return 0;
}

int game_init_graphics(EngineState *s) {
#ifndef WITH_PIC_SCALING
	if (s->gfx_state->options->pic0_unscaled == 0)
		warning("Pic scaling was disabled; your version of FreeSCI has no support for scaled pic drawing built in.");

	s->gfx_state->options->pic0_unscaled = 1;
#endif
	return _reset_graphics_input(s);
}

static void _free_graphics_input(EngineState *s) {
	sciprintf("Freeing graphics\n");

	s->visual->widfree(GFXW(s->visual));

	s->wm_port = s->titlebar_port = s->picture_port = NULL;
	s->visual = NULL;
	s->dyn_views = NULL;
	s->port = NULL;

	free(s->pics);
	s->pics = NULL;
}

int game_init_sound(EngineState *s, int sound_flags) {
	if (s->resmgr->_sciVersion >= SCI_VERSION_01)
		sound_flags |= SFX_STATE_FLAG_MULTIPLAY;

	s->sfx_init_flags = sound_flags;
	sfx_init(&s->sound, s->resmgr, sound_flags);

	return 0;
}


/* Maps a class ID to the script the corresponding class is contained in
   Returns the script number suggested by vocab.996, or -1 if there's none */
static int suggested_script(Resource *res, unsigned int classId) {
	int offset;

	if (!res || classId >= res->size >> 2)
		return -1;

	offset = 2 + (classId << 2);

	return getInt16(res->data + offset);
}

int test_cursor_style(EngineState *s) {
	int resource_nr = 0;
	int ok = 0;

	do {
		ok |= s->resmgr->testResource(kResourceTypeCursor, resource_nr++) != NULL;
	} while (resource_nr < 1000 && !ok);

	return ok;
}

int create_class_table_sci11(EngineState *s) {
	int scriptnr;
	unsigned int seeker_offset;
	char *seeker_ptr;
	int classnr;

	Resource *vocab996 = s->resmgr->findResource(kResourceTypeVocab, 996, 1);

	if (!vocab996)
		s->classtable_size = 20;
	else
		s->classtable_size = vocab996->size >> 2;

	s->classtable = (Class*)sci_calloc(sizeof(Class), s->classtable_size);

	for (scriptnr = 0; scriptnr < 1000; scriptnr++) {
		Resource *heap = s->resmgr->findResource(kResourceTypeHeap, scriptnr, 0);

		if (heap) {
			int global_vars = getUInt16(heap->data + 2);

			seeker_ptr = (char*)heap->data + 4 + global_vars * 2;
			seeker_offset = 4 + global_vars * 2;

			while (getUInt16((byte*)seeker_ptr) == SCRIPT_OBJECT_MAGIC_NUMBER) {
				if (getUInt16((byte*)seeker_ptr + 14) & SCRIPT_INFO_CLASS) {
					classnr = getUInt16((byte*)seeker_ptr + 10);
					if (classnr >= s->classtable_size) {
						if (classnr >= SCRIPT_MAX_CLASSTABLE_SIZE) {
							fprintf(stderr, "Invalid class number 0x%x in script.%d(0x%x), offset %04x\n",
							        classnr, scriptnr, scriptnr, seeker_offset);
							return 1;
						}

						s->classtable = (Class*)sci_realloc(s->classtable, sizeof(Class) * (classnr + 1));
						// Clear after resize
						memset(&(s->classtable[s->classtable_size]), 0, sizeof(Class) * (1 + classnr - s->classtable_size));

						s->classtable_size = classnr + 1; // Adjust maximum number of entries
					}

					s->classtable[classnr].reg.offset = seeker_offset;
					s->classtable[classnr].reg.segment = 0;
					s->classtable[classnr].script = scriptnr;
				}

				seeker_ptr += getUInt16((byte*)seeker_ptr + 2) * 2;
				seeker_offset += getUInt16((byte*)seeker_ptr + 2) * 2;
			}
		}
	}

	return 0;
}

static int create_class_table_sci0(EngineState *s) {
	int scriptnr;
	unsigned int seeker;
	int classnr;
	int magic_offset; // For strange scripts in older SCI versions

	Resource *vocab996 = s->resmgr->findResource(kResourceTypeVocab, 996, 1);

	if (!vocab996)
		s->classtable_size = 20;
	else
		s->classtable_size = vocab996->size >> 2;

	s->classtable = (Class*)sci_calloc(sizeof(Class), s->classtable_size);

	for (scriptnr = 0; scriptnr < 1000; scriptnr++) {
		int objtype = 0;
		Resource *script = s->resmgr->findResource(kResourceTypeScript, scriptnr, 0);

		if (script) {
			if (s->version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER)
				magic_offset = seeker = 2;
			else
				magic_offset = seeker = 0;

			do {
				while (seeker < script->size)	{
					unsigned int lastseeker = seeker;
					objtype = getInt16(script->data + seeker);
					if (objtype == sci_obj_class || objtype == sci_obj_terminator)
						break;
					seeker += getInt16(script->data + seeker + 2);
					if (seeker <= lastseeker) {
						sciprintf("Warning: Script version is invalid.\n");
						free(s->classtable);
						return SCI_ERROR_INVALID_SCRIPT_VERSION;
					}
				}

				if (objtype == sci_obj_class) {
					int sugg_script;

					seeker -= SCRIPT_OBJECT_MAGIC_OFFSET; // Adjust position; script home is base +8 bytes

					classnr = getInt16(script->data + seeker + 4 + SCRIPT_SPECIES_OFFSET);
					if (classnr >= s->classtable_size) {

						if (classnr >= SCRIPT_MAX_CLASSTABLE_SIZE) {
							fprintf(stderr, "Invalid class number 0x%x in script.%d(0x%x), offset %04x\n",
							        classnr, scriptnr, scriptnr, seeker);
							return 1;
						}

						s->classtable = (Class*)sci_realloc(s->classtable, sizeof(Class) * (classnr + 1));
						// Clear after resize
						memset(&(s->classtable[s->classtable_size]), 0, sizeof(Class) * (1 + classnr - s->classtable_size));

						s->classtable_size = classnr + 1; // Adjust maximum number of entries
					}

					sugg_script = suggested_script(vocab996, classnr);

					// First, test whether the script hasn't been claimed, or if it's been claimed by the wrong script

					if (sugg_script == -1 || scriptnr == sugg_script /*|| !s->classtable[classnr].reg.segment*/)  {
						// Now set the home script of the class
						s->classtable[classnr].reg.offset = seeker + 4 - magic_offset;
						s->classtable[classnr].reg.segment = 0;
						s->classtable[classnr].script = scriptnr;
					}

					seeker += SCRIPT_OBJECT_MAGIC_OFFSET; // Re-adjust position
					seeker += getInt16(script->data + seeker + 2); // Move to next
				}

			} while (objtype != sci_obj_terminator && seeker <= script->size);

		}
	}
	s->resmgr->unlockResource(vocab996, 996, kResourceTypeVocab);
	vocab996 = NULL;
	return 0;
}

// Architectural stuff: Init/Unintialize engine
int script_init_engine(EngineState *s, sci_version_t version) {
	int result;

	s->max_version = SCI_VERSION(9, 999, 999);
	s->min_version = 0; //Set no real limits
	s->version = SCI_VERSION_DEFAULT_SCI0;
	s->kernel_opt_flags = 0;

	if (!version) {
		s->version_lock_flag = 0;
	} else {
		s->version = version;
		s->version_lock_flag = 1; // Lock version
	}

	script_detect_versions(s);

	if (s->version >= SCI_VERSION(1, 001, 000))
		result = create_class_table_sci11(s);
	else
		result = create_class_table_sci0(s);

	s->seg_manager = new SegManager(s->version >= SCI_VERSION(1, 001, 000));
	s->gc_countdown = GC_INTERVAL - 1;

	if (result) {
		sciprintf("Failed to initialize class table\n");
		return 1;
	}

	s->script_000_segment = script_get_segment(s, 0, SCRIPT_GET_LOCK);

	if (s->script_000_segment <= 0) {
		sciprintf("Failed to instantiate script.000\n");
		return 1;
	}

	s->script_000 = &(s->seg_manager->heap[s->script_000_segment]->data.script);

	s->sys_strings = s->seg_manager->allocateSysStrings(&s->sys_strings_segment);
	// Allocate static buffer for savegame and CWD directories
	SystemString *str = &s->sys_strings->strings[SYS_STRING_SAVEDIR];
	str->name = strdup("savedir");
	str->max_size = MAX_SAVE_DIR_SIZE;
	str->value = (char*)sci_malloc(MAX_SAVE_DIR_SIZE + 1);
	str->value[0] = 0; // Set to empty string

	s->save_dir_copy = make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
	s->save_dir_edit_offset = 0;

	s->r_acc = s->r_prev = NULL_REG;
	s->r_amp_rest = 0;

	s->execution_stack = NULL;    // Start without any execution stack
	s->execution_stack_base = -1; // No vm is running yet
	s->execution_stack_pos = -1;   // Start at execution stack position 0

	s->kernel_names = vocabulary_get_knames(s->resmgr, &s->kernel_names_nr);
	script_map_kernel(s);
	// Maps the kernel functions

	if (_init_vocabulary(s))
		return 1;

	if (s->selector_map.cantBeHere != -1)
		version_require_later_than(s, SCI_VERSION_FTU_INVERSE_CANBEHERE);

	s->restarting_flags = SCI_GAME_IS_NOT_RESTARTING;

	s->bp_list = NULL; // No breakpoints defined
	s->have_bp = 0;

	if (s->version >= SCI_VERSION_FTU_LOFS_ABSOLUTE &&
	        s->version < SCI_VERSION(1, 001, 000))
		s->seg_manager->setExportWidth(1);
	else
		s->seg_manager->setExportWidth(0);

	sciprintf("Engine initialized\n");

	if (_init_graphics_input(s))
		return 1;

	return 0;
}

void script_set_gamestate_save_dir(EngineState *s, const char *path) {
	SystemString *str = &s->sys_strings->strings[SYS_STRING_SAVEDIR];
	strncpy(str->value, path, str->max_size);
	str->value[str->max_size] = 0; // Make sure to terminate
}

void script_free_vm_memory(EngineState *s) {
	sciprintf("Freeing VM memory\n");
	s->save_dir_copy_buf = NULL;

	free(s->classtable);
	s->classtable = NULL;

	// Close all opened file handles
	s->_fileHandles.clear();
	s->_fileHandles.resize(5);
}

extern void free_kfunct_tables(EngineState *s);
// From kernel.c

void script_free_engine(EngineState *s) {
	script_free_vm_memory(s);

	sciprintf("Freeing state-dependant data\n");

	free_kfunct_tables(s);

	_free_vocabulary(s);
}

void script_free_breakpoints(EngineState *s) {
	Breakpoint *bp, *bp_next;

	// Free breakpoint list
	bp = s->bp_list;
	while (bp) {
		bp_next = bp->next;
		if (bp->type == BREAK_SELECTOR)
			free(bp->data.name);
		free(bp);
		bp = bp_next;
	}

	s->bp_list = NULL;
}

/*************************************************************/
/* Game instance stuff: Init/Unitialize state-dependant data */
/*************************************************************/

int game_init(EngineState *s) {
	// FIXME Use new VM instantiation code all over the place"
	reg_t game_obj; // Address of the game object
	dstack_t *stack;

	stack = s->seg_manager->allocateStack(VM_STACK_SIZE, &s->stack_segment);
	s->stack_base = stack->entries;
	s->stack_top = s->stack_base + VM_STACK_SIZE;

	if (!script_instantiate(s, 0)) {
		sciprintf("game_init(): Could not instantiate script 0\n");
		return 1;
	}

	s->parser_valid = 0; // Invalidate parser
	s->parser_event = NULL_REG; // Invalidate parser event

	s->synonyms = NULL;
	s->synonyms_nr = 0; // No synonyms

	// Initialize send_calls buffer

	if (!send_calls_allocated) {
		send_calls_allocated = 16;
		send_calls = (CallsStruct*)sci_calloc(sizeof(CallsStruct), send_calls_allocated);
	}

	if (s->gfx_state && _reset_graphics_input(s))
		return 1;

	s->successor = NULL; // No successor
	s->status_bar_text = NULL; // Status bar is blank
	s->status_bar_foreground = 0;
	s->status_bar_background = s->resmgr->_sciVersion >= SCI_VERSION_01_VGA ? 255 : 15;

	SystemString *str = &s->sys_strings->strings[SYS_STRING_PARSER_BASE];
	str->name = strdup("parser-base");
	str->max_size = MAX_PARSER_BASE;
	str->value = (char*)sci_malloc(MAX_PARSER_BASE + 1);
	str->value[0] = 0; // Set to empty string

	s->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);

	s->game_start_time = g_system->getMillis();
	s->last_wait_time = s->game_start_time;

	s->debug_mode = 0x0; // Disable all debugging
	s->onscreen_console = 0; // No onscreen console unless explicitly requested

	srand(g_system->getMillis()); // Initialize random number generator

//	script_dissect(0, s->_selectorNames);
	game_obj = script_lookup_export(s, 0, 0);
	// The first entry in the export table of script 0 points to the game object

	s->game_name = sci_strdup(obj_get_name(s, game_obj));

	if (!s->game_name) {
		sciprintf("Error: script.000, export 0 ("PREG") does not\n"
		          " yield an object with a name -> sanity check failed\n", PRINT_REG(game_obj));
		return 1;
	}

	sciprintf(" \"%s\" at "PREG"\n", s->game_name, PRINT_REG(game_obj));

	if (strlen((char *)s->game_name) >= MAX_GAMEDIR_SIZE) {
		s->game_name[MAX_GAMEDIR_SIZE - 1] = 0; // Fix length with brute force
		sciprintf(" Designation too long; was truncated to \"%s\"\n", s->game_name);
	}

	s->game_obj = game_obj;

	// Mark parse tree as unused
	s->parser_nodes[0].type = PARSE_TREE_NODE_LEAF;
	s->parser_nodes[0].content.value = 0;

	s->menubar = menubar_new(); // Create menu bar

	if (s->sfx_init_flags & SFX_STATE_FLAG_NOSOUND)
		game_init_sound(s, 0);

	return 0;
}

int game_exit(EngineState *s) {
	free(s->execution_stack);

	if (!s->successor) {
		sfx_exit(&s->sound);
		// Reinit because some other code depends on having a valid state
		game_init_sound(s, SFX_STATE_FLAG_NOSOUND);
	}

	delete s->seg_manager;

	free(s->synonyms);
	s->synonyms = NULL;
	s->synonyms_nr = 0;

	sciprintf("Freeing miscellaneous data...\n");

	// TODO Free parser segment here
	free(send_calls);
	send_calls_allocated = 0;

	// TODO Free scripts here

	menubar_free(s->menubar);

	_free_graphics_input(s);

	free(s->game_name);

	return 0;
}

} // End of namespace Sci

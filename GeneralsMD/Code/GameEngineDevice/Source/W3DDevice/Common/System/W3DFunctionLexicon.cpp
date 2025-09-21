/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DFunctionLexicon.cpp ///////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Function lexicon for w3d specific function pointers
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "GameClient/GameWindow.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/GameClient/W3DGUICallbacks.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DGadget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA 
///////////////////////////////////////////////////////////////////////////////////////////////////

// Game Window draw methods -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinDrawTable [] = 
{

	{ NAMEKEY_INVALID, "GameWinDefaultDraw",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameWinDefaultDraw) },
	{ NAMEKEY_INVALID, "W3DGameWinDefaultDraw",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGameWinDefaultDraw) },

	{ NAMEKEY_INVALID, "W3DGadgetPushButtonDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetPushButtonDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetPushButtonImageDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetPushButtonImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxDraw",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetCheckBoxDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxImageDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetCheckBoxImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonDraw",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetRadioButtonDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonImageDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetRadioButtonImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetTabControlDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlImageDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetTabControlImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxDraw",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetListBoxDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxImageDraw",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetListBoxImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxDraw",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetComboBoxDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxImageDraw",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetComboBoxImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetHorizontalSliderDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderImageDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetHorizontalSliderImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderDraw",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetVerticalSliderDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderImageDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetVerticalSliderImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarDraw",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetProgressBarDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarImageDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetProgressBarImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetStaticTextDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextImageDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetStaticTextImageDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetTextEntryDraw) },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryImageDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DGadgetTextEntryImageDraw) },

	{ NAMEKEY_INVALID, "W3DLeftHUDDraw",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DLeftHUDDraw) },
	{ NAMEKEY_INVALID, "W3DCameoMovieDraw",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCameoMovieDraw) },
	{ NAMEKEY_INVALID, "W3DRightHUDDraw",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DRightHUDDraw) },
	{ NAMEKEY_INVALID, "W3DPowerDraw",									reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DPowerDraw) },
	{ NAMEKEY_INVALID, "W3DMainMenuDraw",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMainMenuDraw) },
	{ NAMEKEY_INVALID, "W3DMainMenuFourDraw",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMainMenuFourDraw) },
	{ NAMEKEY_INVALID, "W3DMetalBarMenuDraw",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMetalBarMenuDraw) },
	{ NAMEKEY_INVALID, "W3DCreditsMenuDraw",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCreditsMenuDraw) },
	{ NAMEKEY_INVALID, "W3DClockDraw",									reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DClockDraw) },
	{ NAMEKEY_INVALID, "W3DMainMenuMapBorder",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMainMenuMapBorder) },
	{ NAMEKEY_INVALID, "W3DMainMenuButtonDropShadowDraw",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMainMenuButtonDropShadowDraw) },
	{ NAMEKEY_INVALID, "W3DMainMenuRandomTextDraw",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMainMenuRandomTextDraw) },
	{ NAMEKEY_INVALID, "W3DThinBorderDraw",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DThinBorderDraw) },
	{ NAMEKEY_INVALID, "W3DShellMenuSchemeDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DShellMenuSchemeDraw) },
	{ NAMEKEY_INVALID, "W3DCommandBarBackgroundDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCommandBarBackgroundDraw) },
	{ NAMEKEY_INVALID, "W3DCommandBarTopDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCommandBarTopDraw) },
	{ NAMEKEY_INVALID, "W3DCommandBarGenExpDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCommandBarGenExpDraw) },
	{ NAMEKEY_INVALID, "W3DCommandBarHelpPopupDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCommandBarHelpPopupDraw) },

	{ NAMEKEY_INVALID, "W3DCommandBarGridDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCommandBarGridDraw) },


	{ NAMEKEY_INVALID, "W3DCommandBarForegroundDraw",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DCommandBarForegroundDraw) },
	{ NAMEKEY_INVALID, "W3DNoDraw",											reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DNoDraw) },
	{ NAMEKEY_INVALID, "W3DDrawMapPreview",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DDrawMapPreview) },

	{ NAMEKEY_INVALID, nullptr,														nullptr },

};

// Game Window init methods -----------------------------------------------------------------------
static FunctionLexicon::TableEntry layoutInitTable [] = 
{

	{ NAMEKEY_INVALID, "W3DMainMenuInit",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(W3DMainMenuInit) },

	{ NAMEKEY_INVALID, nullptr,														nullptr },

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::W3DFunctionLexicon( void )
{

}  // end W3DFunctionLexicon

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::~W3DFunctionLexicon( void )
{

}  // end ~W3DFunctionLexicon

//-------------------------------------------------------------------------------------------------
/** Initialize the function table specific for our implementations of
	* the w3d device */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::init( void )
{

	// extend functionality
	FunctionLexicon::init();

	// load our own tables
	loadTable( gameWinDrawTable, TABLE_GAME_WIN_DEVICEDRAW );
	loadTable( layoutInitTable, TABLE_WIN_LAYOUT_DEVICEINIT );

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::reset( void )
{

	// Pay attention to the order of what happens in the base class as you reset

	// extend
	FunctionLexicon::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::update( void )
{

	// extend?
	FunctionLexicon::update();

}  // end update



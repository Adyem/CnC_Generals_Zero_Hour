/*
**	Command & Conquer Generals(tm)
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

// FILE: FunctionLexicon.cpp //////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Collection of function pointers to help us in managing
//						 and assign callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/FunctionLexicon.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/Gadget.h"

// Popup Ladder Select --------------------------------------------------------------------------
extern void PopupLadderSelectInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupLadderSelectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupLadderSelectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

extern WindowMsgHandledType PopupBuddyNotificationSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Buddy Overlay Right Click menu callbacks --------------------------------------------------------------
extern void RCGameDetailsMenuInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType RCGameDetailsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Beacon control bar callback --------------------------------------------------------------
extern WindowMsgHandledType BeaconWindowInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup Replay Save Menu ----------------------------------------------------------------------------------
extern void PopupReplayInit( WindowLayout *layout, void *userData );
extern void PopupReplayUpdate( WindowLayout *layout, void *userData );
extern void PopupReplayShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupReplaySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupReplayInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Extended MessageBox ----------------------------------------------------------------------------------
extern WindowMsgHandledType ExtendedMessageBoxSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// game window draw table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinDrawTable[] = 
{
	{ NAMEKEY_INVALID, "IMECandidateMainDraw",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(IMECandidateMainDraw) },
	{ NAMEKEY_INVALID, "IMECandidateTextAreaDraw",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(IMECandidateTextAreaDraw) },
	{ NAMEKEY_INVALID, nullptr,																nullptr }
};

// game window system table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinSystemTable[] = 
{


	{ NAMEKEY_INVALID, "PassSelectedButtonsToParentSystem",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PassSelectedButtonsToParentSystem) },
	{ NAMEKEY_INVALID, "PassMessagesToParentSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PassMessagesToParentSystem) },

	{ NAMEKEY_INVALID, "GameWinDefaultSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameWinDefaultSystem) },
	{ NAMEKEY_INVALID, "GadgetPushButtonSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetPushButtonSystem) },
	{ NAMEKEY_INVALID, "GadgetCheckBoxSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetCheckBoxSystem) },
	{ NAMEKEY_INVALID, "GadgetRadioButtonSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetRadioButtonSystem) },
	{ NAMEKEY_INVALID, "GadgetTabControlSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetTabControlSystem) },
	{ NAMEKEY_INVALID, "GadgetListBoxSystem",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetListBoxSystem) },
	{ NAMEKEY_INVALID, "GadgetComboBoxSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetComboBoxSystem) },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderSystem",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetHorizontalSliderSystem) },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetVerticalSliderSystem) },
	{ NAMEKEY_INVALID, "GadgetProgressBarSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetProgressBarSystem) },
	{ NAMEKEY_INVALID, "GadgetStaticTextSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetStaticTextSystem) },
	{ NAMEKEY_INVALID, "GadgetTextEntrySystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetTextEntrySystem) },
	{ NAMEKEY_INVALID, "MessageBoxSystem",									reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MessageBoxSystem) },
	{ NAMEKEY_INVALID, "QuitMessageBoxSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(QuitMessageBoxSystem) },

	{ NAMEKEY_INVALID, "ExtendedMessageBoxSystem",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ExtendedMessageBoxSystem) },

	{ NAMEKEY_INVALID, "MOTDSystem",										reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MOTDSystem) },
	{ NAMEKEY_INVALID, "MainMenuSystem",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MainMenuSystem) },
	{ NAMEKEY_INVALID, "OptionsMenuSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(OptionsMenuSystem) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SinglePlayerMenuSystem) },
	{ NAMEKEY_INVALID, "QuitMenuSystem",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(QuitMenuSystem) },
	{ NAMEKEY_INVALID, "MapSelectMenuSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "ReplayMenuSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayMenuSystem) },
	{ NAMEKEY_INVALID, "CreditsMenuSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(CreditsMenuSystem) },
	{ NAMEKEY_INVALID, "LanLobbyMenuSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanLobbyMenuSystem) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuSystem",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanGameOptionsMenuSystem) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanMapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuSystem", reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishGameOptionsMenuSystem) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuSystem",   reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishMapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "SaveLoadMenuSystem",            reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SaveLoadMenuSystem) },
	{ NAMEKEY_INVALID, "PopupCommunicatorSystem",       reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupCommunicatorSystem) },
	{ NAMEKEY_INVALID, "PopupBuddyNotificationSystem",  reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupBuddyNotificationSystem) },
	{ NAMEKEY_INVALID, "PopupReplaySystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupReplaySystem) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuSystem",     reinterpret_cast<FunctionLexicon::RawFunctionPointer>(KeyboardOptionsMenuSystem) },
	{ NAMEKEY_INVALID, "WOLLadderScreenSystem",			    reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLadderScreenSystem) },
	{ NAMEKEY_INVALID, "WOLLoginMenuSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLoginMenuSystem) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectSystem",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLocaleSelectSystem) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLobbyMenuSystem) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLGameSetupMenuSystem) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMapSelectMenuSystem) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlaySystem",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlaySystem) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuSystem",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlayRCMenuSystem) },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(RCGameDetailsMenuSystem) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlaySystem",reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameSpyPlayerInfoOverlaySystem) },
	{ NAMEKEY_INVALID, "WOLMessageWindowSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMessageWindowSystem) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQuickMatchMenuSystem) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuSystem",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLWelcomeMenuSystem) },
	{ NAMEKEY_INVALID, "WOLStatusMenuSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLStatusMenuSystem) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQMScoreScreenSystem) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenSystem",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLCustomScoreScreenSystem) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectSystem",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(NetworkDirectConnectSystem) },
	{ NAMEKEY_INVALID, "PopupHostGameSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupHostGameSystem) },
	{ NAMEKEY_INVALID, "PopupJoinGameSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupJoinGameSystem) },
	{ NAMEKEY_INVALID, "PopupLadderSelectSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupLadderSelectSystem) },
	{ NAMEKEY_INVALID, "InGamePopupMessageSystem",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(InGamePopupMessageSystem) },
	{ NAMEKEY_INVALID, "ControlBarSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ControlBarSystem) },
	{ NAMEKEY_INVALID, "ControlBarObserverSystem",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ControlBarObserverSystem) },
	{ NAMEKEY_INVALID, "IMECandidateWindowSystem",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(IMECandidateWindowSystem) },
	{ NAMEKEY_INVALID, "ReplayControlSystem",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayControlSystem) },
	{ NAMEKEY_INVALID, "InGameChatSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(InGameChatSystem) },
	{ NAMEKEY_INVALID, "DisconnectControlSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DisconnectControlSystem) },
	{ NAMEKEY_INVALID, "DiplomacySystem",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DiplomacySystem) },
	{ NAMEKEY_INVALID, "GeneralsExpPointsSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GeneralsExpPointsSystem) },
	{ NAMEKEY_INVALID, "DifficultySelectSystem",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DifficultySelectSystem) },
	
	{ NAMEKEY_INVALID, "IdleWorkerSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(IdleWorkerSystem) },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlSystem", reinterpret_cast<FunctionLexicon::RawFunctionPointer>(EstablishConnectionsControlSystem) },
	{ NAMEKEY_INVALID, "GameInfoWindowSystem",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameInfoWindowSystem) },
	{ NAMEKEY_INVALID, "ScoreScreenSystem",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ScoreScreenSystem) },
	{ NAMEKEY_INVALID, "DownloadMenuSystem",            reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DownloadMenuSystem) },

	{ NAMEKEY_INVALID, nullptr,																nullptr }

};

// game window input table ------------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinInputTable[] = 
{

	{ NAMEKEY_INVALID, "GameWinDefaultInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameWinDefaultInput) },
	{ NAMEKEY_INVALID, "GameWinBlockInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameWinBlockInput) },
	{ NAMEKEY_INVALID, "GadgetPushButtonInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetPushButtonInput) },
	{ NAMEKEY_INVALID, "GadgetCheckBoxInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetCheckBoxInput) },
	{ NAMEKEY_INVALID, "GadgetRadioButtonInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetRadioButtonInput) },
	{ NAMEKEY_INVALID, "GadgetTabControlInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetTabControlInput) },
	{ NAMEKEY_INVALID, "GadgetListBoxInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetListBoxInput) },
	{ NAMEKEY_INVALID, "GadgetListBoxMultiInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetListBoxMultiInput) },
	{ NAMEKEY_INVALID, "GadgetComboBoxInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetComboBoxInput) },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderInput",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetHorizontalSliderInput) },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderInput",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetVerticalSliderInput) },
	{ NAMEKEY_INVALID, "GadgetStaticTextInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetStaticTextInput) },
	{ NAMEKEY_INVALID, "GadgetTextEntryInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GadgetTextEntryInput) },

	{ NAMEKEY_INVALID, "MainMenuInput",									reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MainMenuInput) },
	{ NAMEKEY_INVALID, "MapSelectMenuInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MapSelectMenuInput) },
	{ NAMEKEY_INVALID, "OptionsMenuInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(OptionsMenuInput) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SinglePlayerMenuInput) },
	{ NAMEKEY_INVALID, "LanLobbyMenuInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanLobbyMenuInput) },
	{ NAMEKEY_INVALID, "ReplayMenuInput",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayMenuInput) },
	{ NAMEKEY_INVALID, "CreditsMenuInput",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(CreditsMenuInput) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInput",      reinterpret_cast<FunctionLexicon::RawFunctionPointer>(KeyboardOptionsMenuInput) },
	{ NAMEKEY_INVALID, "PopupCommunicatorInput",        reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupCommunicatorInput) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanGameOptionsMenuInput) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanMapSelectMenuInput) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInput",  reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishGameOptionsMenuInput) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInput",    reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishMapSelectMenuInput) },
	{ NAMEKEY_INVALID, "WOLLadderScreenInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLadderScreenInput) },
	{ NAMEKEY_INVALID, "WOLLoginMenuInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLoginMenuInput) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLocaleSelectInput) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLobbyMenuInput) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLGameSetupMenuInput) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMapSelectMenuInput) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlayInput) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInput",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameSpyPlayerInfoOverlayInput) },
	{ NAMEKEY_INVALID, "WOLMessageWindowInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMessageWindowInput) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQuickMatchMenuInput) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLWelcomeMenuInput) },
	{ NAMEKEY_INVALID, "WOLStatusMenuInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLStatusMenuInput) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQMScoreScreenInput) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInput",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLCustomScoreScreenInput) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInput",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(NetworkDirectConnectInput) },
	{ NAMEKEY_INVALID, "PopupHostGameInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupHostGameInput) },
	{ NAMEKEY_INVALID, "PopupJoinGameInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupJoinGameInput) },
	{ NAMEKEY_INVALID, "PopupLadderSelectInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupLadderSelectInput) },
	{ NAMEKEY_INVALID, "InGamePopupMessageInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(InGamePopupMessageInput) },
	{ NAMEKEY_INVALID, "ControlBarInput",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ControlBarInput) },
	{ NAMEKEY_INVALID, "ReplayControlInput",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayControlInput) },
	{ NAMEKEY_INVALID, "InGameChatInput",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(InGameChatInput) },
	{ NAMEKEY_INVALID, "DisconnectControlInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DisconnectControlInput) },
	{ NAMEKEY_INVALID, "DiplomacyInput",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DiplomacyInput) },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlInput", reinterpret_cast<FunctionLexicon::RawFunctionPointer>(EstablishConnectionsControlInput) },
	{ NAMEKEY_INVALID, "LeftHUDInput",									reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LeftHUDInput) },
	{ NAMEKEY_INVALID, "ScoreScreenInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ScoreScreenInput) },
	{ NAMEKEY_INVALID, "SaveLoadMenuInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SaveLoadMenuInput) },
	{ NAMEKEY_INVALID, "BeaconWindowInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(BeaconWindowInput) },
	{ NAMEKEY_INVALID, "DifficultySelectInput",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DifficultySelectInput) },
	{ NAMEKEY_INVALID, "PopupReplayInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupReplayInput) },
	{ NAMEKEY_INVALID, "GeneralsExpPointsInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GeneralsExpPointsInput)},

	{ NAMEKEY_INVALID, "DownloadMenuInput",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DownloadMenuInput) },

	{ NAMEKEY_INVALID, "IMECandidateWindowInput",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(IMECandidateWindowInput) },
	{ NAMEKEY_INVALID, nullptr,														nullptr }

};

// game window tooltip table ----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinTooltipTable[] = 
{


	{ NAMEKEY_INVALID, "GameWinDefaultTooltip",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameWinDefaultTooltip) },

	{ NAMEKEY_INVALID, nullptr,											nullptr }

};

// window layout init table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutInitTable[] = 
{

	{ NAMEKEY_INVALID, "MainMenuInit",									reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MainMenuInit) },
	{ NAMEKEY_INVALID, "OptionsMenuInit",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(OptionsMenuInit) },
	{ NAMEKEY_INVALID, "SaveLoadMenuInit",              reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SaveLoadMenuInit) },
	{ NAMEKEY_INVALID, "SaveLoadMenuFullScreenInit",    reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SaveLoadMenuFullScreenInit) },

	{ NAMEKEY_INVALID, "PopupCommunicatorInit",         reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupCommunicatorInit) },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInit",       reinterpret_cast<FunctionLexicon::RawFunctionPointer>(KeyboardOptionsMenuInit) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SinglePlayerMenuInit) },
	{ NAMEKEY_INVALID, "MapSelectMenuInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MapSelectMenuInit) },
	{ NAMEKEY_INVALID, "LanLobbyMenuInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanLobbyMenuInit) },
	{ NAMEKEY_INVALID, "ReplayMenuInit",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayMenuInit) },
	{ NAMEKEY_INVALID, "CreditsMenuInit",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(CreditsMenuInit) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInit",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanGameOptionsMenuInit) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanMapSelectMenuInit) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInit",   reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishGameOptionsMenuInit) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInit",     reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishMapSelectMenuInit) },
	{ NAMEKEY_INVALID, "WOLLadderScreenInit",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLadderScreenInit) },
	{ NAMEKEY_INVALID, "WOLLoginMenuInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLoginMenuInit) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInit",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLocaleSelectInit) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLobbyMenuInit) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLGameSetupMenuInit) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMapSelectMenuInit) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInit",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlayInit) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuInit",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlayRCMenuInit) },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(RCGameDetailsMenuInit) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInit",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameSpyPlayerInfoOverlayInit) },
	{ NAMEKEY_INVALID, "WOLMessageWindowInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMessageWindowInit) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQuickMatchMenuInit) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInit",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLWelcomeMenuInit) },
	{ NAMEKEY_INVALID, "WOLStatusMenuInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLStatusMenuInit) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQMScoreScreenInit) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInit",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLCustomScoreScreenInit) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInit",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(NetworkDirectConnectInit) },
	{ NAMEKEY_INVALID, "PopupHostGameInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupHostGameInit) },
	{ NAMEKEY_INVALID, "PopupJoinGameInit",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupJoinGameInit) },
	{ NAMEKEY_INVALID, "PopupLadderSelectInit",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupLadderSelectInit) },
	{ NAMEKEY_INVALID, "InGamePopupMessageInit",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(InGamePopupMessageInit) },
	{ NAMEKEY_INVALID, "GameInfoWindowInit",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameInfoWindowInit) },
	{ NAMEKEY_INVALID, "ScoreScreenInit",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ScoreScreenInit) },
	{ NAMEKEY_INVALID, "DownloadMenuInit",              reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DownloadMenuInit) },
	{ NAMEKEY_INVALID, "DifficultySelectInit",          reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DifficultySelectInit) },
	{ NAMEKEY_INVALID, "PopupReplayInit",							  reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupReplayInit) },

	{ NAMEKEY_INVALID, nullptr,														nullptr }  // keep this last

};

// window layout update table ---------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutUpdateTable[] = 
{

	{ NAMEKEY_INVALID, "MainMenuUpdate",								reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MainMenuUpdate) },
	{ NAMEKEY_INVALID, "OptionsMenuUpdate",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(OptionsMenuUpdate) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SinglePlayerMenuUpdate) },
	{ NAMEKEY_INVALID, "MapSelectMenuUpdate",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "LanLobbyMenuUpdate",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanLobbyMenuUpdate) },
	{ NAMEKEY_INVALID, "ReplayMenuUpdate",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayMenuUpdate) },
	{ NAMEKEY_INVALID, "SaveLoadMenuUpdate",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SaveLoadMenuUpdate) },
	
	{ NAMEKEY_INVALID, "CreditsMenuUpdate",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(CreditsMenuUpdate) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuUpdate",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanGameOptionsMenuUpdate) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanMapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuUpdate", reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishGameOptionsMenuUpdate) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuUpdate",   reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishMapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLLadderScreenUpdate",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLadderScreenUpdate) },
	{ NAMEKEY_INVALID, "WOLLoginMenuUpdate",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLoginMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectUpdate",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLocaleSelectUpdate) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuUpdate",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLobbyMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLGameSetupMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMapSelectMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayUpdate",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlayUpdate) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayUpdate",reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameSpyPlayerInfoOverlayUpdate) },
	{ NAMEKEY_INVALID, "WOLMessageWindowUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMessageWindowUpdate) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQuickMatchMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuUpdate",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLWelcomeMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLStatusMenuUpdate",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLStatusMenuUpdate) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenUpdate",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQMScoreScreenUpdate) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenUpdate",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLCustomScoreScreenUpdate) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectUpdate",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(NetworkDirectConnectUpdate) },
	{ NAMEKEY_INVALID, "ScoreScreenUpdate",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ScoreScreenUpdate) },
	{ NAMEKEY_INVALID, "DownloadMenuUpdate",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DownloadMenuUpdate) },
	{ NAMEKEY_INVALID, "PopupReplayUpdate",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupReplayUpdate) },
	{ NAMEKEY_INVALID, nullptr,														nullptr }  // keep this last

};

// window layout shutdown table -------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutShutdownTable[] = 
{

	{ NAMEKEY_INVALID, "MainMenuShutdown",							reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MainMenuShutdown) },
	{ NAMEKEY_INVALID, "OptionsMenuShutdown",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(OptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "SaveLoadMenuShutdown",          reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SaveLoadMenuShutdown) },
	{ NAMEKEY_INVALID, "PopupCommunicatorShutdown",     reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupCommunicatorShutdown) },      
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuShutdown",   reinterpret_cast<FunctionLexicon::RawFunctionPointer>(KeyboardOptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "SinglePlayerMenuShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SinglePlayerMenuShutdown) },
	{ NAMEKEY_INVALID, "MapSelectMenuShutdown",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(MapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "LanLobbyMenuShutdown",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanLobbyMenuShutdown) },
	{ NAMEKEY_INVALID, "ReplayMenuShutdown",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ReplayMenuShutdown) },
	{ NAMEKEY_INVALID, "CreditsMenuShutdown",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(CreditsMenuShutdown) },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuShutdown",		reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanGameOptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "LanMapSelectMenuShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(LanMapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuShutdown",reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishGameOptionsMenuShutdown) },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuShutdown", reinterpret_cast<FunctionLexicon::RawFunctionPointer>(SkirmishMapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLLadderScreenShutdown",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLadderScreenShutdown) },
	{ NAMEKEY_INVALID, "WOLLoginMenuShutdown",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLoginMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLLocaleSelectShutdown",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLocaleSelectShutdown) },
	{ NAMEKEY_INVALID, "WOLLobbyMenuShutdown",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLLobbyMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLGameSetupMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMapSelectMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayShutdown",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLBuddyOverlayShutdown) },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayShutdown",reinterpret_cast<FunctionLexicon::RawFunctionPointer>(GameSpyPlayerInfoOverlayShutdown) },
	{ NAMEKEY_INVALID, "WOLMessageWindowShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLMessageWindowShutdown) },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQuickMatchMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuShutdown",				reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLWelcomeMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLStatusMenuShutdown",					reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLStatusMenuShutdown) },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenShutdown",			reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLQMScoreScreenShutdown) },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenShutdown",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(WOLCustomScoreScreenShutdown) },
	{ NAMEKEY_INVALID, "NetworkDirectConnectShutdown",	reinterpret_cast<FunctionLexicon::RawFunctionPointer>(NetworkDirectConnectShutdown) },
	{ NAMEKEY_INVALID, "ScoreScreenShutdown",						reinterpret_cast<FunctionLexicon::RawFunctionPointer>(ScoreScreenShutdown) },
	{ NAMEKEY_INVALID, "DownloadMenuShutdown",          reinterpret_cast<FunctionLexicon::RawFunctionPointer>(DownloadMenuShutdown) },
	{ NAMEKEY_INVALID, "PopupReplayShutdown",	          reinterpret_cast<FunctionLexicon::RawFunctionPointer>(PopupReplayShutdown) },
	{ NAMEKEY_INVALID, nullptr,														nullptr }  // keep this last

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA 
///////////////////////////////////////////////////////////////////////////////////////////////////
FunctionLexicon *TheFunctionLexicon = nullptr;  ///< the function dictionary

//-------------------------------------------------------------------------------------------------
/** Since we have a convenient table to organize our callbacks anyway,
	* we'll just use this same storage space to load in any run time
	* components we might want to add to the table, such as generating
	* a key based off the name supplied in the table for faster access */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::loadTable( TableEntry *table, 
																 TableIndex tableIndex )
{

	// sanity
	if( table == nullptr )
		return;

	// loop through all entries
	TableEntry *entry = table;
	while( entry->name )
	{

		// assign key from name key based on name provided in table
		entry->key = TheNameKeyGenerator->nameToKey( AsciiString(entry->name) );

		// next table entry please
		entry++;

	}  // end while

	// assign table to the index specified
	m_tables[ tableIndex ] = table;

}  // end loadTable

//-------------------------------------------------------------------------------------------------
/** Search the provided table for a function matching the key */
//-------------------------------------------------------------------------------------------------
FunctionLexicon::RawFunctionPointer FunctionLexicon::keyToFunc( NameKeyType key, TableEntry *table )
{

        // sanity
        if( key == NAMEKEY_INVALID )
                return nullptr;

        // search table for key
        TableEntry *entry = table;
        while( entry && entry->key != NAMEKEY_INVALID )
        {

                if( entry->key == key )
                        return entry->func;
                entry++;

        }  // end if

        return nullptr;  // not found

}  // end keyToFunc

//-------------------------------------------------------------------------------------------------
/** Search tables for the function given this key, if the index parameter
	* is TABLE_ANY, then ALL tables will be searched.  Otherwise index refers
	* to only a single table index to be searched */
//-------------------------------------------------------------------------------------------------
FunctionLexicon::RawFunctionPointer FunctionLexicon::findFunction( NameKeyType key, TableIndex index )
{
        RawFunctionPointer func = nullptr;

        // sanity
        if( key == NAMEKEY_INVALID )
                return nullptr;

	// search ALL tables for function if the index paramater allows if
	if( index == TABLE_ANY )
	{

		Int i;
		for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
		{

			func = keyToFunc( key, m_tables[ i ] );
                        if( func )
                                break;  // exit for i

		}  // end for i

	}  // end if
	else
	{

		// do NOT search all tables, just the one specified by the parameter
                func = keyToFunc( key, m_tables[ index ] );

	}  // end else

	// return function, if found
        return func;

}  // end findFunction

#ifdef NOT_IN_USE
//-------------------------------------------------------------------------------------------------
/** Search for the function in the specified table */
//-------------------------------------------------------------------------------------------------
const char *FunctionLexicon::funcToName( RawFunctionPointer func, TableEntry *table )
{

        // sanity
        if( func == nullptr )
                return nullptr;

	// search the table
	TableEntry *entry = table;
	while( entry && entry->key != NAMEKEY_INVALID )
	{

		// is this it
		if( entry->func == func )
                        return entry->name;

		// not it, check next
		entry++;

	}  // end while

        return nullptr;  // not found

}  // end funcToName
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::FunctionLexicon( void )
{
	Int i;

	// empty the tables
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
		m_tables[ i ] = nullptr;

}  // end FunctionLexicon

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::~FunctionLexicon( void )
{

}  // end ~FunctionLexicon

//-------------------------------------------------------------------------------------------------
/** Initialize our dictionary of funtion pointers and symbols */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::init( void )
{

	// if you change this method, check the reset() and make sure it's OK

	// initialize the name key identifiers for the lookup table
	loadTable( gameWinDrawTable,						TABLE_GAME_WIN_DRAW );
	loadTable( gameWinSystemTable,					TABLE_GAME_WIN_SYSTEM );
	loadTable( gameWinInputTable,						TABLE_GAME_WIN_INPUT );
	loadTable( gameWinTooltipTable,					TABLE_GAME_WIN_TOOLTIP );

	loadTable( winLayoutInitTable,					TABLE_WIN_LAYOUT_INIT );
	loadTable( winLayoutUpdateTable,				TABLE_WIN_LAYOUT_UPDATE );
	loadTable( winLayoutShutdownTable,			TABLE_WIN_LAYOUT_SHUTDOWN );

	validate();

}  // end init

//-------------------------------------------------------------------------------------------------
/** reset */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::reset( void )
{

	//
	// make sure the ordering of what happens here with respect to derived classes resets is
	// all OK since we're cheating and using the init() method
	//

	// nothing dynamically loaded, just reinit the tables
	init();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */ 
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::update( void )
{

}  // end update

/*
// !NOTE! We can not have this function, see the header for
// more information as to why
//
//-------------------------------------------------------------------------------------------------
// translate a function pointer to its symbolic name
//-------------------------------------------------------------------------------------------------
char *FunctionLexicon::functionToName( void *func )
{
	
	// sanity
	if( func == nullptr )
		return nullptr;

	// search ALL the tables
	Int i;
	char *name = nullptr;
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		name = funcToName( func, m_tables[ i ] );
		if( name )
			return name;

	}  // end for i

	return nullptr;  // not found

}  // end functionToName
*/

//-------------------------------------------------------------------------------------------------
/** Scan the tables and make sure that each function address is unique.
	* We want to do this to prevent accidental entries of two identical
	* functions and because the compiler will optimize identical functions
	* to the same address (typically in empty functions with no body and the
	* same parameters) which we MUST keep separate for when we add code to
	* them */
//-------------------------------------------------------------------------------------------------
Bool FunctionLexicon::validate( void )
{
	Bool valid = TRUE;
	Int i, j;
	TableEntry *sourceEntry, *lookAtEntry;

	// scan all talbes
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		// scan through this table
		sourceEntry = m_tables[ i ];
		while( sourceEntry && sourceEntry->key != NAMEKEY_INVALID )
		{
		
			//
			// scan all tables looking for the function in sourceEntry, do not bother
			// of source entry is nullptr (a valid entry in the table, but not a function)
			//
			if( sourceEntry->func )
			{

				// scan all tables
				for( j = 0; j < MAX_FUNCTION_TABLES; j++ )
				{

					// scan all entries in this table
					lookAtEntry = m_tables[ j ];
					while( lookAtEntry && lookAtEntry->key != NAMEKEY_INVALID )
					{

						//
						// check for match, do not match the entry source with itself
						//
						if( sourceEntry != lookAtEntry )
							if( sourceEntry->func == lookAtEntry->func )
							{

								DEBUG_LOG(( "WARNING! Function lexicon entries match same address! '%s' and '%s'\n", 
														sourceEntry->name, lookAtEntry->name ));
								valid = FALSE;

							}  // end if

						// next entry in this target table
						lookAtEntry++;

					}  // end while

				}  // end for j

			}  // end if

			// next source entry
			sourceEntry++;

		}  // end while	

	}  // end for i

	// return the valid state of our tables
	return valid;

}  // end validate

//============================================================================
// FunctionLexicon::gameWinDrawFunc
//============================================================================

GameWinDrawFunc FunctionLexicon::gameWinDrawFunc( NameKeyType key, TableIndex index )
{ 
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		GameWinDrawFunc func;

            func = reinterpret_cast<GameWinDrawFunc>( findFunction( key, TABLE_GAME_WIN_DEVICEDRAW ) );
		if ( func == nullptr )
		{
                    func = reinterpret_cast<GameWinDrawFunc>( findFunction( key, TABLE_GAME_WIN_DRAW ) );
		}
		return func;
	}
	// search the specified table
    return reinterpret_cast<GameWinDrawFunc>( findFunction( key, index ) );
}

WindowLayoutInitFunc FunctionLexicon::winLayoutInitFunc( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		WindowLayoutInitFunc func;

            func = reinterpret_cast<WindowLayoutInitFunc>( findFunction( key, TABLE_WIN_LAYOUT_DEVICEINIT ) );
		if ( func == nullptr )
		{
                    func = reinterpret_cast<WindowLayoutInitFunc>( findFunction( key, TABLE_WIN_LAYOUT_INIT ) );
		}
		return func;
	}
	// search the specified table
    return reinterpret_cast<WindowLayoutInitFunc>( findFunction( key, index ) );
}

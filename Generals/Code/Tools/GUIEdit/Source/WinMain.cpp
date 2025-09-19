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

// FILE: WinMain.cpp //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  GUIEdit.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       GUI Edit and window layout entry point
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/GameEngine.h"
#include "GameClient/GameWindowManager.h"
#include "Win32Device/GameClient/Win32Mouse.h"
#include "SFMLPlatform/SfmlKeyboardBridge.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "Resource.h"
#include "Lib/BaseType.h"
#include "GUIEdit.h"
#include "EditWindow.h"
#include "DialogProc.h"
#include "LayoutScheme.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////
static char *szWindowClass = "GUIEdit";

// PUBLIC DATA ////////////////////////////////////////////////////////////////
HINSTANCE ApplicationHInstance;				///< main application instance
HWND ApplicationHWnd;							///< main application HWnd
Win32Mouse *TheWin32Mouse = NULL;	///< for Win32 mouse
char *gAppPrefix = "ge_"; /// So GuiEdit can have a different debug log file name if we need it

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

namespace
{

sf::Keyboard::Key TranslateVirtualKey( WPARAM key )
{
        if( key >= 'A' && key <= 'Z' )
        {
                return static_cast< sf::Keyboard::Key >( sf::Keyboard::A + (key - 'A') );
        }

        if( key >= '0' && key <= '9' )
        {
                return static_cast< sf::Keyboard::Key >( sf::Keyboard::Num0 + (key - '0') );
        }

        switch( key )
        {
                case VK_ESCAPE:    return sf::Keyboard::Escape;
                case VK_LCONTROL:  return sf::Keyboard::LControl;
                case VK_RCONTROL:  return sf::Keyboard::RControl;
                case VK_LSHIFT:    return sf::Keyboard::LShift;
                case VK_RSHIFT:    return sf::Keyboard::RShift;
                case VK_LMENU:     return sf::Keyboard::LAlt;
                case VK_RMENU:     return sf::Keyboard::RAlt;
                case VK_MENU:      return sf::Keyboard::Menu;
                case VK_OEM_4:     return sf::Keyboard::LBracket;
                case VK_OEM_6:     return sf::Keyboard::RBracket;
                case VK_OEM_1:     return sf::Keyboard::Semicolon;
                case VK_OEM_7:     return sf::Keyboard::Quote;
                case VK_OEM_COMMA: return sf::Keyboard::Comma;
                case VK_OEM_PERIOD:return sf::Keyboard::Period;
                case VK_OEM_2:     return sf::Keyboard::Slash;
                case VK_OEM_5:     return sf::Keyboard::Backslash;
                case VK_OEM_3:     return sf::Keyboard::Tilde;
                case VK_OEM_PLUS:  return sf::Keyboard::Equal;
                case VK_OEM_MINUS: return sf::Keyboard::Hyphen;
                case VK_SPACE:     return sf::Keyboard::Space;
                case VK_RETURN:    return sf::Keyboard::Enter;
                case VK_BACK:      return sf::Keyboard::Backspace;
                case VK_TAB:       return sf::Keyboard::Tab;
                case VK_PRIOR:     return sf::Keyboard::PageUp;
                case VK_NEXT:      return sf::Keyboard::PageDown;
                case VK_END:       return sf::Keyboard::End;
                case VK_HOME:      return sf::Keyboard::Home;
                case VK_INSERT:    return sf::Keyboard::Insert;
                case VK_DELETE:    return sf::Keyboard::Delete;
                case VK_ADD:       return sf::Keyboard::Add;
                case VK_SUBTRACT:  return sf::Keyboard::Subtract;
                case VK_MULTIPLY:  return sf::Keyboard::Multiply;
                case VK_DIVIDE:    return sf::Keyboard::Divide;
                case VK_LEFT:      return sf::Keyboard::Left;
                case VK_RIGHT:     return sf::Keyboard::Right;
                case VK_UP:        return sf::Keyboard::Up;
                case VK_DOWN:      return sf::Keyboard::Down;
                case VK_NUMPAD0:   return sf::Keyboard::Numpad0;
                case VK_NUMPAD1:   return sf::Keyboard::Numpad1;
                case VK_NUMPAD2:   return sf::Keyboard::Numpad2;
                case VK_NUMPAD3:   return sf::Keyboard::Numpad3;
                case VK_NUMPAD4:   return sf::Keyboard::Numpad4;
                case VK_NUMPAD5:   return sf::Keyboard::Numpad5;
                case VK_NUMPAD6:   return sf::Keyboard::Numpad6;
                case VK_NUMPAD7:   return sf::Keyboard::Numpad7;
                case VK_NUMPAD8:   return sf::Keyboard::Numpad8;
                case VK_NUMPAD9:   return sf::Keyboard::Numpad9;
                case VK_F1:        return sf::Keyboard::F1;
                case VK_F2:        return sf::Keyboard::F2;
                case VK_F3:        return sf::Keyboard::F3;
                case VK_F4:        return sf::Keyboard::F4;
                case VK_F5:        return sf::Keyboard::F5;
                case VK_F6:        return sf::Keyboard::F6;
                case VK_F7:        return sf::Keyboard::F7;
                case VK_F8:        return sf::Keyboard::F8;
                case VK_F9:        return sf::Keyboard::F9;
                case VK_F10:       return sf::Keyboard::F10;
                case VK_F11:       return sf::Keyboard::F11;
                case VK_F12:       return sf::Keyboard::F12;
                case VK_PAUSE:     return sf::Keyboard::Pause;
                default:
                        break;
        }

        return sf::Keyboard::Unknown;
}

void DispatchSfmlKeyboardEvent( UINT message, WPARAM wParam, LPARAM lParam )
{
        switch( message )
        {
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYUP:
                        break;
                default:
                        return;
        }

        sfml_platform::SfmlKeyboardBridge *keyboard = sfml_platform::GetActiveKeyboardBridge();
        if( keyboard == NULL )
        {
                return;
        }

        const sf::Keyboard::Key code = TranslateVirtualKey( wParam );
        if( code == sf::Keyboard::Unknown )
        {
                return;
        }

        sf::Event event = {};
        event.type = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) ? sf::Event::KeyPressed : sf::Event::KeyReleased;
        event.key.code = code;
        event.key.alt = (GetAsyncKeyState( VK_MENU ) & 0x8000) != 0;
        event.key.control = (GetAsyncKeyState( VK_CONTROL ) & 0x8000) != 0;
        event.key.shift = (GetAsyncKeyState( VK_SHIFT ) & 0x8000) != 0;
#if defined(VK_LWIN) && defined(VK_RWIN)
        event.key.system = ((GetAsyncKeyState( VK_LWIN ) | GetAsyncKeyState( VK_RWIN )) & 0x8000) != 0;
#else
        event.key.system = FALSE;
#endif

        keyboard->handleEvent( event );
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutCallback(HWND, UINT, WPARAM, LPARAM);

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// initInstance ===============================================================
//
//   FUNCTION: initInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
//=============================================================================
static BOOL initInstance( HINSTANCE hInstance, int nCmdShow )
{

	// store application instance
	ApplicationHInstance = hInstance;

	// create app window and keep handle
	ApplicationHWnd = CreateWindowEx( 0,											// extended style
																		szWindowClass,					// window class name
																		"GUIEdit",							// window name
																		WS_OVERLAPPEDWINDOW,		// window styles
																		0,											// x position
																		0,											// y position
																		GetSystemMetrics( SM_CXSCREEN ), // width
																		GetSystemMetrics( SM_CYSCREEN ),  // height
																		NULL,										// parent
																		NULL,										// menu
																		ApplicationHInstance,		// instance
																		NULL );									// creation data

	if( ApplicationHWnd == NULL )
		return FALSE;

	// display the window
	ShowWindow( ApplicationHWnd, SW_MAXIMIZE );
	UpdateWindow( ApplicationHWnd );

	return TRUE;

}  // end initInstance

// registerClass ==============================================================
//
//  FUNCTION: registerClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
//=============================================================================
static ATOM registerClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= (WNDPROC)WndProc;
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= hInstance;
	wcex.hIcon					= LoadIcon(hInstance, (LPCTSTR)GUIEDIT_LARGE_ICON);
	wcex.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject( BLACK_BRUSH );
	wcex.lpszMenuName		=	(LPCSTR)GUIEDIT_MENU;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm				= LoadIcon(wcex.hInstance, (LPCTSTR)GUIEDIT_SMALL_ICON);

	return RegisterClassEx( &wcex );

}  // registerClass

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// WinMain ====================================================================
/** Entry point for application */
//=============================================================================
Int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	Int returnValue;
	HACCEL hAccelTable;
	Bool quit = FALSE;

	/// @todo remove this force set of working directory later
	Char buffer[ _MAX_PATH ];
	GetModuleFileName( NULL, buffer, sizeof( buffer ) );
	Char *pEnd = buffer + strlen( buffer );
	while( pEnd != buffer ) 
	{
		if( *pEnd == '\\' ) 
		{
			*pEnd = 0;
			break;
		}
		pEnd--;
	}
	::SetCurrentDirectory(buffer);

	// start the log
	DEBUG_INIT(DEBUG_FLAGS_DEFAULT);
	initMemoryManager();

	// register a class for our window with the OS
	registerClass( hInstance );

	// Perform application initialization:
	if( !initInstance( hInstance, nCmdShow ) ) 
		return FALSE;

	// load accellerator table
	hAccelTable = LoadAccelerators( hInstance, (LPCTSTR)GUIEDIT_ACCELERATORS );

	// initialize the common controls
	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof( controls );
	controls.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx( &controls );

	// initialize GUIEdit data
	TheEditor = new GUIEdit;
	if( TheEditor == NULL )
		return FALSE;
	TheEditor->init();	

	//
	// see if we have any messages to process, a NULL window handle tells the
	// OS to look at the main window associated with the calling thread, us!
	//
	while( quit == FALSE )
	{

		// is there is message ready for us?
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{

			// process ALL messages waiting
			while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{

				// get the message
				returnValue = GetMessage( &msg, NULL, 0, 0 );

				// check for quitting
				if( returnValue == 0 )
					quit = TRUE;

				// translate accelerator messages
				if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
				{

					// translate and dispatch the message
					TranslateMessage( &msg );
					DispatchMessage( &msg );

				}  // end if

			}  // end while

		}  // end if
		else
		{

			// udpate our universe
			TheEditor->update();
			Sleep(1);

		}  // end else
						
	}  // end while

	// shutdown GUIEdit data
	delete TheEditor;
	TheEditor = NULL;

	// close the log
	shutdownMemoryManager();
	DEBUG_SHUTDOWN();

	return msg.wParam;

}  // end WinMain

// WndProc ====================================================================
//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//=============================================================================
LRESULT CALLBACK WndProc( HWND hWnd, UINT message,
                                                                                                        WPARAM wParam, LPARAM lParam )
{

        DispatchSfmlKeyboardEvent( message, wParam, lParam );

        switch( message )
        {

		// ------------------------------------------------------------------------
		case WM_MOUSEMOVE:
		{

			// keep focus in our app
			if( GetFocus() != hWnd )
				SetFocus( hWnd );
			return 0;

		}  // end move mouse

		// ------------------------------------------------------------------------
		case WM_COMMAND:
		{
			Int controlID = LOWORD( wParam );
//			Int nofifyCode = HIWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

			switch( controlID )
			{

				// --------------------------------------------------------------------
				case MENU_EXIT:
					TheEditor->menuExit();
					break;

				//---------------------------------------------------------------------
				case MENU_NEW:
					TheEditor->menuNew();
					break;

				// --------------------------------------------------------------------
				case MENU_OPEN:
					TheEditor->menuOpen();
					break;

				// --------------------------------------------------------------------
				case MENU_SAVE:
					TheEditor->menuSave();
					break;

				// --------------------------------------------------------------------
				case MENU_SAVEAS:
					TheEditor->menuSaveAs();
					break;

				// --------------------------------------------------------------------
				case MENU_COPY:
					TheEditor->menuCopy();
					break;

				// --------------------------------------------------------------------
				case MENU_CUT:
					TheEditor->menuCut();
					break;

				// --------------------------------------------------------------------
				case MENU_PASTE:
					TheEditor->menuPaste();
					break;

				// --------------------------------------------------------------------
				case MENU_CALLBACKS:
					DialogBox( TheEditor->getInstance(), (LPCTSTR)CALLBACK_EDITOR_DIALOG,
										 TheEditor->getWindowHandle(), CallbackEditorDialogProc );
					break;

				// --------------------------------------------------------------------
				case MENU_GRID_SETTINGS:
					DialogBox( TheEditor->getInstance(), (LPCTSTR)GRID_SETTINGS_DIALOG,
										 TheEditor->getWindowHandle(), GridSettingsDialogProc );
					break;

				// --------------------------------------------------------------------
				case MENU_SHOW_HIDDEN_OUTLINES:
					TheEditor->setShowHiddenOutlines( !TheEditor->getShowHiddenOutlines() );
					break;

				// --------------------------------------------------------------------
				case MENU_SHOW_SEE_THRU_OUTLINES:
					TheEditor->setShowSeeThruOutlines( !TheEditor->getShowSeeThruOutlines() );
					break;
														
				// --------------------------------------------------------------------
				case MENU_TEST_MODE:

					// switch to test mode or edit mode
					if( TheEditor)
					{
						EditMode mode = TheEditor->getMode();

						if( mode != MODE_TEST_RUN )
							TheEditor->setMode( MODE_TEST_RUN );
						else
							TheEditor->setMode( MODE_EDIT );

					}  // end if
					break;				
					
				// --------------------------------------------------------------------
				case MENU_BACKGROUND_COLOR:
					
					if( TheEditWindow )
					{
						RGBColorReal color = TheEditWindow->getBackgroundColor();
						RGBColorInt *newColor;
						POINT mouse;

						// get mouse point
						GetCursorPos( &mouse );

						// select a new color
						newColor = SelectColor( (Int)(255.0f * color.red),
																		(Int)(255.0f * color.green),
																		(Int)(255.0f * color.blue),
																		(Int)(255.0f * color.alpha),
																		mouse.x, mouse.y );
						if( newColor )
						{
							
							color.red   = (Real)newColor->red / 255.0f;
							color.green = (Real)newColor->green / 255.0f;
							color.blue  = (Real)newColor->blue / 255.0f;
							color.alpha = (Real)newColor->alpha / 255.0f;
							TheEditWindow->setBackgroundColor( color );

						}  // end if

					}  //  end if

					break;

				// --------------------------------------------------------------------
				case MENU_SCHEME:
				{

					// open the default scheme information
					if( TheDefaultScheme )
						TheDefaultScheme->openDialog();
					break;

				}  // end scheme

				// --------------------------------------------------------------------
				case MENU_ABOUT:
				{

					DialogBox( ApplicationHInstance, (LPCTSTR)ABOUT_DIALOG, 
										 hWnd, (DLGPROC)AboutCallback );
					break;

				}  // end about


				// --------------------------------------------------------------------
				default:
				{

					return DefWindowProc( hWnd, message, wParam, lParam );

				}  // end default

			}  // end switch( controlID )

			return 0;

		}  // end command

		// ------------------------------------------------------------------------
		case WM_CLOSE:
		{

			// close gets initiated from the 'x' in the top right of the window
			TheEditor->menuExit();
			return 0;

		}  // end close

		// ------------------------------------------------------------------------
		case WM_KEYDOWN:
		{
			Int virtualKey = wParam;
//			Int keyData = lParam;	
			Bool controlDown = BitTest( GetKeyState( VK_CONTROL ), 0x1000 );

			switch( virtualKey )
			{

				// --------------------------------------------------------------------
				case VK_ESCAPE:
				{

					// unselect all windows
					if( TheEditor )
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
							TheEditor->setMode( MODE_EDIT );
						else
							TheEditor->clearSelections();
					break;

				}  // end escape

				// --------------------------------------------------------------------
				case VK_DELETE:
				{

					// delete all selected windows
					if( TheEditor )
						TheEditor->deleteSelected();
					break;

				}  // end delete

				// --------------------------------------------------------------------
				case VK_LEFT:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{
					
							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);
							
							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.x -= TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.x--;	
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}  // end Left
				// --------------------------------------------------------------------
				case VK_RIGHT:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{
					
							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);
							
							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;
							
							temp = TheEditWindow->getDragMoveDest();
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.x += TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.x++;	
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}  // end RIGHT
				// --------------------------------------------------------------------
				case VK_UP:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{
					
							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);
							
							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();
							
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.y -= TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.y--;	
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}  // end Up
				// --------------------------------------------------------------------
				case VK_DOWN:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{
					
							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);
							
							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.y += TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.y++;	
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}  // end Down
				
				// --------------------------------------------------------------------
				case VK_RETURN:
				{
					if( TheEditor && TheEditWindow && TheEditor->getMode() == MODE_KEYBOARD_MOVE )
					{
						ICoord2D tempOrigin, tempDest;
						tempDest = TheEditWindow->getDragMoveDest();
						tempOrigin = TheEditWindow->getDragMoveOrigin();

						// move the windows
						TheEditor->dragMoveSelectedWindows( &tempOrigin, &tempDest );

						// go back to normal mode
						TheEditor->setMode( MODE_EDIT );
					}
					break;
				}// end Enter

			}  // end switch( virtualKey )

			return 0;

		}  // end key down

		// ------------------------------------------------------------------------
		case WM_SIZE:
		{
			Int width  = LOWORD( lParam );
			Int height = HIWORD( lParam );

			// resize status bar
			if( TheEditor )
			{
				HWND statusBar = TheEditor->getStatusBarWindowHandle();

				if( statusBar )
				{
					RECT rect;
					Int barX, barY;
					Int barWidth, barHeight;

					// keep status window height the same
					GetWindowRect( statusBar, &rect );
					barWidth = width;
					barHeight = rect.bottom - rect.top;

					barX = 0;
					barY = height - barHeight;
					MoveWindow( statusBar, barX, barY, barWidth, barHeight, TRUE );

				}  // end if

			}  // end if

			return 0;

		}  // end size

		// ------------------------------------------------------------------------
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		}  // end paint

		// ------------------------------------------------------------------------
		case WM_DESTROY:
		{

			PostQuitMessage(0);
			break;

		}  // end destroy

		// ------------------------------------------------------------------------
		default:
		{

			return DefWindowProc(hWnd, message, wParam, lParam);

		}  // end default

	}  // end switch( message )

	return DefWindowProc( hWnd, message, wParam, lParam );

}  // end WndProc

// AboutCallback ==============================================================
/** Mesage handler for about box. */
//=============================================================================
LRESULT CALLBACK AboutCallback( HWND hDlg, UINT message, 
																WPARAM wParam, LPARAM lParam )
{

	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;

}  // end AboutCallback


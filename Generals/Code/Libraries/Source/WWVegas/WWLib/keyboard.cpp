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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/KEYBOARD.CPP                                       $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/15/99 10:15a                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   WWKeyboardClass::Buff_Get -- Lowlevel function to get a key from key buffer               *
 *   WWKeyboardClass::Check -- Checks to see if a key is in the buffer                         *
 *   WWKeyboardClass::Clear -- Clears the keyboard buffer.                                     *
 *   WWKeyboardClass::Down -- Checks to see if the specified key is being held down.           *
 *   WWKeyboardClass::Fetch_Element -- Extract the next element in the keyboard buffer.        *
 *   WWKeyboardClass::Fill_Buffer_From_Syste -- Extract and process any queued windows messages*
 *   WWKeyboardClass::Get -- Logic to get a metakey from the buffer                            *
 *   WWKeyboardClass::Get_Mouse_X -- Returns the mouses current x position in pixels           *
 *   WWKeyboardClass::Get_Mouse_XY -- Returns the mouses x,y position via reference vars       *
 *   WWKeyboardClass::Get_Mouse_Y -- returns the mouses current y position in pixels           *
 *   WWKeyboardClass::Is_Buffer_Empty -- Checks to see if the keyboard buffer is empty.        *
 *   WWKeyboardClass::Is_Buffer_Full -- Determines if the keyboard buffer is full.             *
 *   WWKeyboardClass::Is_Mouse_Key -- Checks to see if specified key refers to the mouse.      *
 *   WWKeyboardClass::Process_Sfml_Event -- Handles SFML events for the keyboard queue        *
 *   WWKeyboardClass::Peek_Element -- Fetches the next element in the keyboard buffer.         *
 *   WWKeyboardClass::Put -- Logic to insert a key into the keybuffer]                         *
 *   WWKeyboardClass::Put_Element -- Put a keyboard data element into the buffer.              *
 *   WWKeyboardClass::Put_Key_Message -- Translates and inserts wParam into Keyboard Buffer    *
 *   WWKeyboardClass::To_ASCII -- Convert the key value into an ASCII representation.          *
 *   WWKeyboardClass::Available_Buffer_Room -- Fetch the quantity of free elements in the keybo*
 *   WWKeyboardClass::Put_Mouse_Message -- Stores a mouse type message into the keyboard buffer*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"_xmouse.h"
#include "keyboard.h"
#include "compat/win_compat.h"
//#include	"mono.h"
#include	"sfml_message_pump.h"

#if WWLIB_HAS_SFML
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>
#endif

namespace
{

#if WWLIB_HAS_SFML
unsigned short Translate_Sfml_Key(sf::Keyboard::Key key)
{
	using sf::Keyboard;

	switch (key) {
	case Keyboard::A: return VK_A;
	case Keyboard::B: return VK_B;
	case Keyboard::C: return VK_C;
	case Keyboard::D: return VK_D;
	case Keyboard::E: return VK_E;
	case Keyboard::F: return VK_F;
	case Keyboard::G: return VK_G;
	case Keyboard::H: return VK_H;
	case Keyboard::I: return VK_I;
	case Keyboard::J: return VK_J;
	case Keyboard::K: return VK_K;
	case Keyboard::L: return VK_L;
	case Keyboard::M: return VK_M;
	case Keyboard::N: return VK_N;
	case Keyboard::O: return VK_O;
	case Keyboard::P: return VK_P;
	case Keyboard::Q: return VK_Q;
	case Keyboard::R: return VK_R;
	case Keyboard::S: return VK_S;
	case Keyboard::T: return VK_T;
	case Keyboard::U: return VK_U;
	case Keyboard::V: return VK_V;
	case Keyboard::W: return VK_W;
	case Keyboard::X: return VK_X;
	case Keyboard::Y: return VK_Y;
	case Keyboard::Z: return VK_Z;
	case Keyboard::Num0: return VK_0;
	case Keyboard::Num1: return VK_1;
	case Keyboard::Num2: return VK_2;
	case Keyboard::Num3: return VK_3;
	case Keyboard::Num4: return VK_4;
	case Keyboard::Num5: return VK_5;
	case Keyboard::Num6: return VK_6;
	case Keyboard::Num7: return VK_7;
	case Keyboard::Num8: return VK_8;
	case Keyboard::Num9: return VK_9;
	case Keyboard::Escape: return VK_ESCAPE;
	case Keyboard::LControl:
	case Keyboard::RControl: return VK_CONTROL;
	case Keyboard::LShift:
	case Keyboard::RShift: return VK_SHIFT;
	case Keyboard::LAlt:
	case Keyboard::RAlt: return VK_MENU;
	case Keyboard::Menu: return VK_MENU;
	case Keyboard::LSystem: return VK_NONE_5B;
	case Keyboard::RSystem: return VK_NONE_5C;
	case Keyboard::Space: return VK_SPACE;
	case Keyboard::Enter: return VK_RETURN;
	case Keyboard::Backspace: return VK_BACK;
	case Keyboard::Tab: return VK_TAB;
	case Keyboard::PageUp: return VK_PRIOR;
	case Keyboard::PageDown: return VK_NEXT;
	case Keyboard::End: return VK_END;
	case Keyboard::Home: return VK_HOME;
	case Keyboard::Insert: return VK_INSERT;
	case Keyboard::Delete: return VK_DELETE;
	case Keyboard::Left: return VK_LEFT;
	case Keyboard::Right: return VK_RIGHT;
	case Keyboard::Up: return VK_UP;
	case Keyboard::Down: return VK_DOWN;
	case Keyboard::Add: return VK_ADD;
	case Keyboard::Subtract: return VK_SUBTRACT;
	case Keyboard::Multiply: return VK_MULTIPLY;
	case Keyboard::Divide: return VK_DIVIDE;
	case Keyboard::Numpad0: return VK_NUMPAD0;
	case Keyboard::Numpad1: return VK_NUMPAD1;
	case Keyboard::Numpad2: return VK_NUMPAD2;
	case Keyboard::Numpad3: return VK_NUMPAD3;
	case Keyboard::Numpad4: return VK_NUMPAD4;
	case Keyboard::Numpad5: return VK_NUMPAD5;
	case Keyboard::Numpad6: return VK_NUMPAD6;
	case Keyboard::Numpad7: return VK_NUMPAD7;
	case Keyboard::Numpad8: return VK_NUMPAD8;
	case Keyboard::Numpad9: return VK_NUMPAD9;
	case Keyboard::F1: return VK_F1;
	case Keyboard::F2: return VK_F2;
	case Keyboard::F3: return VK_F3;
	case Keyboard::F4: return VK_F4;
	case Keyboard::F5: return VK_F5;
	case Keyboard::F6: return VK_F6;
	case Keyboard::F7: return VK_F7;
	case Keyboard::F8: return VK_F8;
	case Keyboard::F9: return VK_F9;
	case Keyboard::F10: return VK_F10;
	case Keyboard::F11: return VK_F11;
	case Keyboard::F12: return VK_F12;
	case Keyboard::Pause: return VK_SCROLL;
	case Keyboard::Semicolon: return VK_NONE_BA;
	case Keyboard::Equal: return VK_NONE_BB;
	case Keyboard::Comma: return VK_NONE_BC;
	case Keyboard::Hyphen: return VK_NONE_BD;
	case Keyboard::Period: return VK_NONE_BE;
	case Keyboard::Slash: return VK_NONE_BF;
	case Keyboard::Backslash: return VK_NONE_DC;
	case Keyboard::Tilde: return VK_NONE_C0;
	case Keyboard::LBracket: return VK_NONE_DB;
	case Keyboard::RBracket: return VK_NONE_DD;
	case Keyboard::Quote: return VK_NONE_DE;
	default: break;
	}

	return VK_NONE;
}

#else
unsigned short Translate_Sfml_Key(sf::Keyboard::Key)
{
	return VK_NONE;
}

#endif

#if WWLIB_HAS_SFML
unsigned short Translate_Sfml_Mouse_Button(sf::Mouse::Button button)
{
	using sf::Mouse;

	switch (button) {
	case Mouse::Left: return VK_LBUTTON;
	case Mouse::Right: return VK_RBUTTON;
	case Mouse::Middle: return VK_MBUTTON;
	default: break;
	}

	return VK_NONE;
}

#else
unsigned short Translate_Sfml_Mouse_Button(sf::Mouse::Button)
{
	return VK_NONE;
}

#endif

} // anonymous namespace

namespace
{


} // anonymous namespace

#define	ARRAY_SIZE(x)		int(sizeof(x)/sizeof(x[0]))

void Stop_Execution (void)
{
//	__asm nop			// Is this line needed?
}


/***********************************************************************************************
 * WWKeyboardClass::WWKeyBoardClass -- Construction for Westwood Keyboard Class                *
 *                                                                                             *
 * INPUT:		none							                                                        *
 *                                                                                             *
 * OUTPUT:     none							                                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
WWKeyboardClass::WWKeyboardClass(void) :
	MouseQX(0),
	MouseQY(0),
	Head(0),
	Tail(0),
	MessagePump(NULL),
	MessagePumpListenerId(0)
{
	memset(KeyState, '\0', sizeof(KeyState));
}

WWKeyboardClass::~WWKeyboardClass(void)
{
	Detach_From_Message_Pump();
}

void WWKeyboardClass::Attach_To_Message_Pump(WWLib::SfmlMessagePump &pump)
{
	if (MessagePump == &pump && MessagePumpListenerId != 0) {
		return;
	}

	Detach_From_Message_Pump();

	MessagePump = &pump;
	MessagePumpListenerId = pump.AddEventListener([this](const sf::Event &event) {
		Process_Sfml_Event(event);
	});
}

void WWKeyboardClass::Detach_From_Message_Pump(void)
{
	if (MessagePump != NULL && MessagePumpListenerId != 0) {
		MessagePump->RemoveEventListener(MessagePumpListenerId);
	}

	MessagePump = NULL;
	MessagePumpListenerId = 0;
}


/***********************************************************************************************
 * WWKeyboardClass::Buff_Get -- Lowlevel function to get a key from key buffer                 *
 *                                                                                             *
 * INPUT:		none                                                        						  *
 *                                                                                             *
 * OUTPUT:     int		- the key value that was pulled from buffer (includes bits)				  *                                                                                *
 *                                                                                             *
 * WARNINGS:   If the key was a mouse event MouseQX and MouseQY will be updated                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Buff_Get(void)
{
	while (!Check()) {}										// wait for key in buffer

	unsigned short temp = Fetch_Element();
	if (Is_Mouse_Key(temp)) {
		MouseQX = Fetch_Element();
		MouseQY = Fetch_Element();
	}
	return(temp);
}


/***********************************************************************************************
 * WWKeyboardClass::Is_Mouse_Key -- Checks to see if specified key refers to the mouse.        *
 *                                                                                             *
 *    This checks the specified key code to see if it refers to the mouse buttons.             *
 *                                                                                             *
 * INPUT:   key   -- The key to check.                                                         *
 *                                                                                             *
 * OUTPUT:  bool; Is the key a mouse button key?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Is_Mouse_Key(unsigned short key)
{
	key &= 0xFF;
	return (key == VK_LBUTTON || key == VK_MBUTTON || key == VK_RBUTTON);
}


/***********************************************************************************************
 * WWKeyboardClass::Check -- Checks to see if a key is in the buffer                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *   09/24/1996 JLB : Converted to new style keyboard system.                                  *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Check(void) const
{
	((WWKeyboardClass *)this)->Fill_Buffer_From_System();
	if (Is_Buffer_Empty()) return(false);
	return(Peek_Element());
}


/***********************************************************************************************
 * WWKeyboardClass::Get -- Logic to get a metakey from the buffer                              *
 *                                                                                             *
 * INPUT:		none                                                        						  *
 *                                                                                             *
 * OUTPUT:     int		- the meta key taken from the buffer.											  *
 *                                                                                             *
 * WARNINGS:	This routine will not return until a keypress is received							  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Get(void)
{
	while (!Check()) {}								// wait for key in buffer
	return (Buff_Get());
}


/***********************************************************************************************
 * WWKeyboardClass::Put -- Logic to insert a key into the keybuffer]                           *
 *                                                                                             *
 * INPUT:		int	 	- the key to insert into the buffer          								  *
 *                                                                                             *
 * OUTPUT:     bool		- true if key is sucessfuly inserted.							              *
 *                                                                                             *
 * WARNINGS:   none							                                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put(unsigned short key)
{
	if (!Is_Buffer_Full()) {
		Put_Element(key);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Put_Key_Message -- Translates and inserts wParam into Keyboard Buffer      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put_Key_Message(unsigned short vk_key, bool release)
{
	/*
	** Get the status of all of the different keyboard modifiers.  Note, only pay attention
	** to numlock and caps lock if we are dealing with a key that is affected by them.  Note
	** that we do not want to set the shift, ctrl and alt bits for Mouse keypresses as this
	** would be incompatible with the dos version.
	*/
        if (!Is_Mouse_Key(vk_key)) {
#if defined(_WIN32)
                if (((GetKeyState(VK_SHIFT) & 0x8000) != 0) ||
                        ((GetKeyState(VK_CAPITAL) & 0x0008) != 0) ||
                        ((GetKeyState(VK_NUMLOCK) & 0x0008) != 0)) {
                        vk_key |= WWKEY_SHIFT_BIT;
                }
                if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                        vk_key |= WWKEY_CTRL_BIT;
                }
                if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
                        vk_key |= WWKEY_ALT_BIT;
                }
#else
                if ((KeyState[VK_SHIFT] & 0x80) != 0 ||
                    (KeyState[VK_CAPITAL] & 0x01) != 0 ||
                    (KeyState[VK_NUMLOCK] & 0x01) != 0) {
                        vk_key |= WWKEY_SHIFT_BIT;
                }
                if ((KeyState[VK_CONTROL] & 0x80) != 0) {
                        vk_key |= WWKEY_CTRL_BIT;
                }
                if ((KeyState[VK_MENU] & 0x80) != 0) {
                        vk_key |= WWKEY_ALT_BIT;
                }
#endif
        }

	if (release) {
		vk_key |= WWKEY_RLS_BIT;
	}

	/*
	** Finally use the put command to enter the key into the keyboard
	** system.
	*/
	return(Put(vk_key));
}


/***********************************************************************************************
 * WWKeyboardClass::Put_Mouse_Message -- Stores a mouse type message into the keyboard buffer. *
 *                                                                                             *
 *    This routine will store the mouse type event into the keyboard buffer. It also checks    *
 *    to ensure that there is enough room in the buffer so that partial mouse events won't     *
 *    be recorded.                                                                             *
 *                                                                                             *
 * INPUT:   vk_key   -- The mouse key message itself.                                          *
 *                                                                                             *
 *          x,y      -- The mouse coordinates at the time of the event.                        *
 *                                                                                             *
 *          release  -- Is this a mouse button release?                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the event stored sucessfully into the keyboard buffer?                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put_Mouse_Message(unsigned short vk_key, int x, int y, bool release)
{
	if (Available_Buffer_Room() >= 3 && Is_Mouse_Key(vk_key)) {
		Put_Key_Message(vk_key, release);
		Put((unsigned short)x);
		Put((unsigned short)y);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::To_ASCII -- Convert the key value into an ASCII representation.            *
 *                                                                                             *
 *    This routine will convert the key code specified into an ASCII value. This takes into    *
 *    consideration the language and keyboard mapping of the host Windows system.              *
 *                                                                                             *
 * INPUT:   key   -- The key code to convert into ASCII.                                       *
 *                                                                                             *
 * OUTPUT:  Returns with the key converted into ASCII. If the key has no ASCII equivalent,     *
 *          then '\0' is returned.                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char WWKeyboardClass::To_ASCII(unsigned short key)
{
	/*
	**	Released keys never translate into an ASCII value.
	*/
	if (key & WWKEY_RLS_BIT) {
		return('\0');
	}

	/*
	**	Set the KeyState buffer to reflect the shift bits stored in the key value.
	*/
	if (key & WWKEY_SHIFT_BIT) {
		KeyState[VK_SHIFT] = 0x80;
	}
	if (key & WWKEY_CTRL_BIT) {
		KeyState[VK_CONTROL] = 0x80;
	}
	if (key & WWKEY_ALT_BIT) {
		KeyState[VK_MENU] = 0x80;
	}

	/*
	**	Ask windows to translate the key into an ASCII equivalent.
	*/
	char buffer[10];
	int result;
//	int result = 1;
	int scancode;
//	int scancode = 0;

#if defined(_WIN32)
	scancode = MapVirtualKey(key & 0xFF, 0);
	result = ToAscii((UINT)(key & 0xFF), (UINT)scancode, (PBYTE)KeyState, (LPWORD)buffer, (UINT)0);
#else
	scancode = static_cast<int>(key & 0xFF);
	buffer[0] = static_cast<char>(key & 0xFF);
	result = 1;
	(void)scancode;
#endif

	/*
	**	Restore the KeyState buffer back to pristine condition.
	*/
	if (key & WWKEY_SHIFT_BIT) {
		KeyState[VK_SHIFT] = 0;
	}
	if (key & WWKEY_CTRL_BIT) {
		KeyState[VK_CONTROL] = 0;
	}
	if (key & WWKEY_ALT_BIT) {
		KeyState[VK_MENU] = 0;
	}

	/*
	**	If Windows could not perform the translation as expected, then
	**	return with a null ASCII value.
	*/
	if (result != 1) {
		return('\0');
	}

	return(buffer[0]);
}


/***********************************************************************************************
 * WWKeyboardClass::Down -- Checks to see if the specified key is being held down.             *
 *                                                                                             *
 *    This routine will examine the key specified to see if it is currently being held down.   *
 *                                                                                             *
 * INPUT:   key   -- The key to check.                                                         *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified key currently being held down?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Down(unsigned short key)
{
#if defined(_WIN32)
	return(GetAsyncKeyState(key & 0xFF) != 0);
#else
	return((KeyState[key & 0xFF] & 0x80) != 0);
#endif
}


//extern "C" {
//	void __cdecl Stop_Execution (void);
//}


/***********************************************************************************************
 * WWKeyboardClass::Fetch_Element -- Extract the next element in the keyboard buffer.          *
 *                                                                                             *
 *    This routine will extract the next pending element in the keyboard queue. If there is    *
 *    no element available, then NULL is returned.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the element extracted from the queue. An empty queue is signified     *
 *          by a 0 return value.                                                               *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Fetch_Element(void)
{
	unsigned short val = 0;
	if (Head != Tail) {
		val = Buffer[Head];

		Head = (Head + 1) % ARRAY_SIZE(Buffer);
	}
	return(val);
}


/***********************************************************************************************
 * WWKeyboardClass::Peek_Element -- Fetches the next element in the keyboard buffer.           *
 *                                                                                             *
 *    This routine will examine and return with the next element in the keyboard buffer but    *
 *    it will not alter or remove that element. Use this routine to see what is pending in     *
 *    the keyboard queue.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the next element in the keyboard queue. If the keyboard buffer is     *
 *          empty, then 0 is returned.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Peek_Element(void) const
{
	if (!Is_Buffer_Empty()) {
		return(Buffer[Head]);
	}
	return(0);
}


/***********************************************************************************************
 * WWKeyboardClass::Put_Element -- Put a keyboard data element into the buffer.                *
 *                                                                                             *
 *    This will put one keyboard data element into the keyboard buffer. Typically, this data   *
 *    is a key code, but it might be mouse coordinates.                                        *
 *                                                                                             *
 * INPUT:   val   -- The data element to add to the keyboard buffer.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the keyboard element added successfully? A failure would indicate that   *
 *                the keyboard buffer is full.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put_Element(unsigned short val)
{
	if (!Is_Buffer_Full()) {
		int temp = (Tail+1) % ARRAY_SIZE(Buffer);
		Buffer[Tail] = val;
		Tail = temp;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Is_Buffer_Full -- Determines if the keyboard buffer is full.               *
 *                                                                                             *
 *    This routine will examine the keyboard buffer to determine if it is completely           *
 *    full of queued keyboard events.                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the keyboard buffer completely full?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Is_Buffer_Full(void) const
{
	if ((Tail + 1) % ARRAY_SIZE(Buffer) == Head) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Is_Buffer_Empty -- Checks to see if the keyboard buffer is empty.          *
 *                                                                                             *
 *    This routine will examine the keyboard buffer to see if it contains no events at all.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the keyboard buffer currently without any pending events queued?          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Is_Buffer_Empty(void) const
{
	if (Head == Tail) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Fill_Buffer_From_Syste -- Extract and process any queued windows messages. *
 *                                                                                             *
 *    This routine will extract and process any windows messages in the windows message        *
 *    queue. It is presumed that the normal message handler will call the keyboard             *
 *    message processing function.                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWKeyboardClass::Fill_Buffer_From_System(void)
{
	if (!Is_Buffer_Full() && MessagePump != NULL) {
		MessagePump->PumpEvents();
	}
}


/***********************************************************************************************
 * WWKeyboardClass::Clear -- Clears the keyboard buffer.                                       *
 *                                                                                             *
 *    This routine will clear the keyboard buffer of all pending keyboard events.              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWKeyboardClass::Clear(void)
{
	/*
	**	Extract any windows pending keyboard message events and then clear out the keyboard
	**	buffer.
	*/
	Fill_Buffer_From_System();
	Head = Tail;

	/*
	**	Perform a second clear to handle the rare case of the keyboard buffer being full and there
	**	still remains keyboard related events in the windows message queue.
	*/
	Fill_Buffer_From_System();
	Head = Tail;
}
/***********************************************************************************************
 * WWKeyboardClass::Process_Sfml_Event -- Handles sf::Event input for the keyboard buffer       *
 *                                                                                             *
 *    This routine examines the SFML event supplied and, if it corresponds to keyboard or       *
 *    mouse input that WWLib tracks, stores the translated result into the keyboard queue.      *
 *                                                                                             *
 * INPUT:   event -- The SFML event to translate.                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/03/2025 JH : Created to replace Win32 message handling.                                 *
 ***********************************************************************************************/
void WWKeyboardClass::Process_Sfml_Event(const sf::Event &event)
{
	switch (event.type) {
	case sf::Event::KeyPressed:
	case sf::Event::KeyReleased:
	{
		unsigned short vk_key = Translate_Sfml_Key(static_cast<sf::Keyboard::Key>(event.key.code));
		if (vk_key == VK_NONE) {
			break;
		}

		if (event.type == sf::Event::KeyPressed && vk_key == VK_SCROLL) {
			Stop_Execution();
		} else {
			Put_Key_Message(vk_key, event.type == sf::Event::KeyReleased);
		}
		break;
	}
	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
	{
		unsigned short vk_key = Translate_Sfml_Mouse_Button(static_cast<sf::Mouse::Button>(event.mouseButton.button));
		if (vk_key == VK_NONE) {
			break;
		}

		int x = event.mouseButton.x;
		int y = event.mouseButton.y;

		if (MessagePump != NULL) {
			sf::Vector2i window_position = MessagePump->GetWindow().getPosition();
			x += window_position.x;
			y += window_position.y;
		}

		if (MouseCursor != NULL) MouseCursor->Convert_Coordinate(x, y);

		Put_Mouse_Message(vk_key, x, y, event.type == sf::Event::MouseButtonReleased);
		break;
	}
	default:
		break;
	}
}





/***********************************************************************************************
 * WWKeyboardClass::Available_Buffer_Room -- Fetch the quantity of free elements in the keyboa *
 *                                                                                             *
 *    This examines the keyboard buffer queue and determine how many elements are available    *
 *    for use before the buffer becomes full. Typical use of this would be when inserting      *
 *    mouse events that require more than one element. Such an event must detect when there    *
 *    would be insufficient room in the buffer and bail accordingly.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of elements that may be stored in to the keyboard buffer   *
 *          before it becomes full and cannot accept any more elements.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWKeyboardClass::Available_Buffer_Room(void) const
{
	int avail = 0;
	if (Head == Tail) {
		avail = ARRAY_SIZE(Buffer);
	}
	if (Head < Tail) {
		avail = Tail - Head;
	}
	if (Head > Tail) {
		avail = (Tail + ARRAY_SIZE(Buffer)) - Head;
	}
	return(avail);
}

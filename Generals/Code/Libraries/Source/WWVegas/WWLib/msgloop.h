/*
**      Command & Conquer Generals(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/msgloop.h                                          $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef MSGLOOP_H
#define MSGLOOP_H

#include "sfml_message_pump.h"

namespace WWLib {

void Windows_Message_Handler();

SfmlMessagePump::DialogHookId Add_Modeless_Dialog(SfmlMessagePump::DialogHook hook);
void Remove_Modeless_Dialog(SfmlMessagePump::DialogHookId id);

SfmlMessagePump::AcceleratorId Add_Accelerator(const AcceleratorKey &key, SfmlMessagePump::AcceleratorCallback callback);
void Remove_Accelerator(SfmlMessagePump::AcceleratorId id);

SfmlMessagePump::InterceptorId Push_Message_Interceptor(SfmlMessagePump::Interceptor interceptor);
void Remove_Message_Interceptor(SfmlMessagePump::InterceptorId id);

} // namespace WWLib

#endif // MSGLOOP_H

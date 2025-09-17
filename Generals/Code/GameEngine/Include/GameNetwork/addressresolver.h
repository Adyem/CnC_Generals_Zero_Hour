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

////////////////////////////////////////////////////////////////////////////////
//                                                                                                                             /
/
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
/
//                                                                                                                             /
/
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __ADDRESSRESOLVER_H
#define __ADDRESSRESOLVER_H

#include "GameNetwork/NetworkDefs.h"

class AsciiString;
struct sockaddr;
struct addrinfo;

struct ResolvedNetAddress
{
        ResolvedNetAddress();

        enum { MAX_STORAGE_SIZE = 128 };

        union
        {
                UnsignedChar m_raw[MAX_STORAGE_SIZE];
                double m_alignment; // ensures at least 8-byte alignment
        } m_storage;

        Int m_length;
        Int m_family;

        const sockaddr *getSockaddr() const;
        sockaddr *getSockaddr();
};

struct ResolverRequest
{
        ResolverRequest();

        const char *m_host;
        const char *m_service;
        Int m_family;
        Int m_sockType;
        Int m_protocol;
        Int m_flags;
};

Bool ResolveFirstUsableAddress(const ResolverRequest &request, ResolvedNetAddress &outAddress, Int *outErrorCode = nullptr);

Bool ResolveFirstUsableAddress(const AsciiString &host, UnsignedShort port, Int sockType, Int protocol, Int flags, ResolvedNetAddress &outAddress, Int *outErrorCode = nullptr);

Bool ResolveAddressList(const ResolverRequest &request, addrinfo **outInfo, Int *outErrorCode = nullptr);

#endif

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

#include "PreRTS.h"     // This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/addressresolver.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "Common/Debug.h"

#ifndef WSA_NOT_ENOUGH_MEMORY
#define WSA_NOT_ENOUGH_MEMORY WSAENOBUFS
#endif

namespace
{
        Int TranslateAddrInfoErrorToWSA(Int error)
        {
                switch (error)
                {
#ifdef EAI_AGAIN
                case EAI_AGAIN:
                        return WSATRY_AGAIN;
#endif
#ifdef EAI_BADFLAGS
                case EAI_BADFLAGS:
                        return WSAEINVAL;
#endif
#ifdef EAI_FAIL
                case EAI_FAIL:
                        return WSANO_RECOVERY;
#endif
#ifdef EAI_FAMILY
                case EAI_FAMILY:
                        return WSAEAFNOSUPPORT;
#endif
#ifdef EAI_MEMORY
                case EAI_MEMORY:
                        return WSA_NOT_ENOUGH_MEMORY;
#endif
#ifdef EAI_NONAME
                case EAI_NONAME:
                        return WSAHOST_NOT_FOUND;
#endif
#ifdef EAI_NODATA
                case EAI_NODATA:
                        return WSANO_DATA;
#endif
#ifdef EAI_SERVICE
                case EAI_SERVICE:
                        return WSATYPE_NOT_FOUND;
#endif
#ifdef EAI_SOCKTYPE
                case EAI_SOCKTYPE:
                        return WSAESOCKTNOSUPPORT;
#endif
#ifdef EAI_ADDRFAMILY
                case EAI_ADDRFAMILY:
                        return WSAEAFNOSUPPORT;
#endif
                default:
                        return WSANO_RECOVERY;
                }
        }

        Bool CopyFirstUsableAddress(addrinfo *info, ResolvedNetAddress &outAddress)
        {
                const addrinfo *ipv6Candidate = nullptr;
                for (addrinfo *current = info; current != nullptr; current = current->ai_next)
                {
                        if ((current->ai_family != AF_INET) && (current->ai_family != AF_INET6))
                        {
                                continue;
                        }

                        if (current->ai_addrlen <= 0)
                        {
                                continue;
                        }

                        if (current->ai_addrlen > ResolvedNetAddress::MAX_STORAGE_SIZE)
                        {
                                DEBUG_LOG(("CopyFirstUsableAddress - address length %d exceeds storage size %d\n", current->ai_addrlen, ResolvedNetAddress::MAX_STORAGE_SIZE));
                                continue;
                        }

                        if (current->ai_family == AF_INET)
                        {
                                memcpy(outAddress.m_storage.m_raw, current->ai_addr, current->ai_addrlen);
                                outAddress.m_length = current->ai_addrlen;
                                outAddress.m_family = current->ai_family;
                                return TRUE;
                        }

                        if (ipv6Candidate == nullptr)
                        {
                                ipv6Candidate = current;
                        }
                }

                if (ipv6Candidate != nullptr)
                {
                        memcpy(outAddress.m_storage.m_raw, ipv6Candidate->ai_addr, ipv6Candidate->ai_addrlen);
                        outAddress.m_length = ipv6Candidate->ai_addrlen;
                        outAddress.m_family = ipv6Candidate->ai_family;
                        return TRUE;
                }

                return FALSE;
        }

        void LogResolutionFailure(const ResolverRequest &request, Int errorCode)
        {
#ifdef DEBUG_LOGGING
                const char *hostString = (request.m_host != nullptr) ? request.m_host : "<null>";
                const char *serviceString = (request.m_service != nullptr) ? request.m_service : "<null>";
                const char *errorString = gai_strerrorA(errorCode);
                DEBUG_LOG(("ResolveFirstUsableAddress - failed to resolve host '%s' service '%s' (error %d: %s)\n", hostString, serviceString, errorCode, errorString));
#else
                (void)request;
                (void)errorCode;
#endif
        }

        void LogNoUsableAddress(const ResolverRequest &request)
        {
#ifdef DEBUG_LOGGING
                const char *hostString = (request.m_host != nullptr) ? request.m_host : "<null>";
                const char *serviceString = (request.m_service != nullptr) ? request.m_service : "<null>";
                DEBUG_LOG(("ResolveFirstUsableAddress - no usable IPv4/IPv6 address for host '%s' service '%s'\n", hostString, serviceString));
#else
                (void)request;
#endif
        }
}

ResolvedNetAddress::ResolvedNetAddress()
{
        memset(m_storage.m_raw, 0, sizeof(m_storage.m_raw));
        m_length = 0;
        m_family = AF_UNSPEC;
}

const sockaddr *ResolvedNetAddress::getSockaddr() const
{
        return reinterpret_cast<const sockaddr *>(m_storage.m_raw);
}

sockaddr *ResolvedNetAddress::getSockaddr()
{
        return reinterpret_cast<sockaddr *>(m_storage.m_raw);
}

ResolverRequest::ResolverRequest()
{
        m_host = nullptr;
        m_service = nullptr;
        m_family = AF_UNSPEC;
        m_sockType = 0;
        m_protocol = 0;
        m_flags = 0;
}

Bool ResolveFirstUsableAddress(const ResolverRequest &request, ResolvedNetAddress &outAddress, Int *outErrorCode)
{
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = request.m_family;
        hints.ai_socktype = request.m_sockType;
        hints.ai_protocol = request.m_protocol;
        hints.ai_flags = request.m_flags;

        addrinfo *info = nullptr;
        const char *host = request.m_host;
        if ((host != nullptr) && (host[0] == '\0'))
        {
                host = nullptr;
        }

        int resolveError = getaddrinfo(host, request.m_service, &hints, &info);
        if (resolveError != 0)
        {
                Int translatedError = TranslateAddrInfoErrorToWSA(resolveError);
                WSASetLastError(translatedError);
                if (outErrorCode != nullptr)
                {
                        *outErrorCode = translatedError;
                }

                LogResolutionFailure(request, resolveError);
                return FALSE;
        }

        Bool success = CopyFirstUsableAddress(info, outAddress);
        freeaddrinfo(info);

        if (!success)
        {
                WSASetLastError(WSANO_DATA);
                if (outErrorCode != nullptr)
                {
                        *outErrorCode = WSANO_DATA;
                }

                LogNoUsableAddress(request);
                return FALSE;
        }

        if (outErrorCode != nullptr)
        {
                *outErrorCode = 0;
        }
#ifdef DEBUG_LOGGING
        if (outAddress.m_family == AF_INET6)
        {
                char addressString[INET6_ADDRSTRLEN + 8];
                memset(addressString, 0, sizeof(addressString));
                DWORD addressLength = sizeof(addressString);
                sockaddr_storage storage;
                memset(&storage, 0, sizeof(storage));
                memcpy(&storage, outAddress.getSockaddr(), outAddress.m_length);
                if (WSAAddressToStringA(reinterpret_cast<LPSOCKADDR>(&storage), outAddress.m_length, nullptr, addressString, &addressLength) == 0)
                {
                        DEBUG_LOG(("ResolveFirstUsableAddress - using IPv6 address %s\n", addressString));
                }
                else
                {
                        Int formatError = WSAGetLastError();
                        DEBUG_LOG(("ResolveFirstUsableAddress - IPv6 formatting failed with error %d\n", formatError));
                }
        }
#endif
        WSASetLastError(0);

        return TRUE;
}

Bool ResolveFirstUsableAddress(const AsciiString &host, UnsignedShort port, Int sockType, Int protocol, Int flags, ResolvedNetAddress &outAddress, Int *outErrorCode)
{
        ResolverRequest request;
        request.m_host = (host.getLength() > 0) ? host.str() : nullptr;
        request.m_family = AF_UNSPEC;
        request.m_sockType = sockType;
        request.m_protocol = protocol;
        request.m_flags = flags;

        char serviceBuffer[6];
        if (port != 0)
        {
                _snprintf(serviceBuffer, sizeof(serviceBuffer), "%hu", port);
                serviceBuffer[sizeof(serviceBuffer) - 1] = '\0';
                request.m_service = serviceBuffer;
        }
        else
        {
                request.m_service = nullptr;
        }

        return ResolveFirstUsableAddress(request, outAddress, outErrorCode);
}

Bool ResolveAddressList(const ResolverRequest &request, addrinfo **outInfo, Int *outErrorCode)
{
        if (outInfo == nullptr)
        {
                WSASetLastError(WSAEFAULT);
                if (outErrorCode != nullptr)
                {
                        *outErrorCode = WSAEFAULT;
                }
                return FALSE;
        }

        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = request.m_family;
        hints.ai_socktype = request.m_sockType;
        hints.ai_protocol = request.m_protocol;
        hints.ai_flags = request.m_flags;

        const char *host = request.m_host;
        if ((host != nullptr) && (host[0] == '\0'))
        {
                host = nullptr;
        }

        int resolveError = getaddrinfo(host, request.m_service, &hints, outInfo);
        if (resolveError != 0)
        {
                Int translatedError = TranslateAddrInfoErrorToWSA(resolveError);
                WSASetLastError(translatedError);
                if (outErrorCode != nullptr)
                {
                        *outErrorCode = translatedError;
                }

                LogResolutionFailure(request, resolveError);
                *outInfo = nullptr;
                return FALSE;
        }

        if (outErrorCode != nullptr)
        {
                *outErrorCode = 0;
        }
        WSASetLastError(0);

        return TRUE;
}

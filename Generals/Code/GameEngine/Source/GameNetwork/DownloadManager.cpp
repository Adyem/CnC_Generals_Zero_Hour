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
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
//                                                                                                                             /
////////////////////////////////////////////////////////////////////////////////

// FILE: DownloadManager.cpp //////////////////////////////////////////////////////
// Generals download manager code
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"     // This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GameText.h"
#include "GameNetwork/DownloadManager.h"

DownloadManager *TheDownloadManager = nullptr;

DownloadManager::DownloadManager()
        : m_wasError(FALSE),
          m_sawEnd(FALSE),
          m_errorString(UnicodeString::TheEmptyString),
          m_statusString(UnicodeString::TheEmptyString)
{
}

DownloadManager::~DownloadManager() = default;

void DownloadManager::init(void)
{
        reset();
}

void DownloadManager::reset(void)
{
        m_wasError = FALSE;
        m_sawEnd = FALSE;
        m_errorString = UnicodeString::TheEmptyString;
        m_statusString = UnicodeString::TheEmptyString;
        m_queuedDownloads.clear();
}

HRESULT DownloadManager::update(void)
{
        return S_OK;
}

HRESULT DownloadManager::downloadFile(AsciiString, AsciiString, AsciiString, AsciiString, AsciiString localfile, AsciiString, Bool)
{
        m_wasError = TRUE;
        m_sawEnd = FALSE;
        if (localfile.isNotEmpty()) {
                m_statusString = TheGameText ? TheGameText->fetch("FTP:StatusIdle") : UnicodeString::TheEmptyString;
        }
        return E_FAIL;
}

AsciiString DownloadManager::getLastLocalFile(void)
{
        return AsciiString::TheEmptyString;
}

HRESULT DownloadManager::OnError(Int)
{
        m_wasError = TRUE;
        return E_FAIL;
}

HRESULT DownloadManager::OnEnd()
{
        m_sawEnd = TRUE;
        m_wasError = FALSE;
        return S_OK;
}

HRESULT DownloadManager::OnQueryResume()
{
        return S_OK;
}

HRESULT DownloadManager::OnProgressUpdate(Int, Int, Int, Int)
{
        return S_OK;
}

HRESULT DownloadManager::OnStatusUpdate(Int)
{
        return S_OK;
}

void DownloadManager::queueFileForDownload(AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume)
{
        QueuedDownload q;
        q.server = server;
        q.userName = username;
        q.password = password;
        q.file = file;
        q.localFile = localfile;
        q.regKey = regkey;
        q.tryResume = tryResume;
        m_queuedDownloads.push_back(q);
}

HRESULT DownloadManager::downloadNextQueuedFile(void)
{
        if (m_queuedDownloads.empty()) {
                return S_OK;
        }
        m_queuedDownloads.pop_front();
        m_wasError = TRUE;
        m_sawEnd = FALSE;
        return E_FAIL;
}

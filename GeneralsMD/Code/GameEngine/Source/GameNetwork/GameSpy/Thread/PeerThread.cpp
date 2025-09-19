/*
**      Command & Conquer Generals(tm) - Zero Hour
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
//
//  Neutral GameSpy peer worker stub
//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"

#include "GameNetwork/GameSpy/PeerThread.h"

#include <queue>

namespace
{
        class NullPeerQueue final : public GameSpyPeerMessageQueueInterface
        {
        public:
                NullPeerQueue()
                        : m_serialAuth(SERIAL_AUTHFAILED),
                          m_threadStarted(FALSE)
                {
                }

                virtual void startThread(void) override
                {
                        m_serialAuth = SERIAL_AUTHFAILED;
                        m_threadStarted = TRUE;
                }

                virtual void endThread(void) override
                {
                        m_serialAuth = SERIAL_AUTHFAILED;
                        m_threadStarted = FALSE;
                        m_responses = ResponseQueue();
                }

                virtual Bool isThreadRunning(void) override
                {
                        return m_threadStarted;
                }

                virtual Bool isConnected(void) override
                {
                        return FALSE;
                }

                virtual Bool isConnecting(void) override
                {
                        return FALSE;
                }

                virtual void addRequest(const PeerRequest &request) override
                {
                        if (request.peerRequestType == PeerRequest::PEERREQUEST_LOGIN)
                        {
                                PeerResponse response;
                                response.peerResponseType = PeerResponse::PEERRESPONSE_DISCONNECT;
                                response.discon.reason = DISCONNECT_COULDNOTCONNECT;
                                queueResponse(response);
                                m_serialAuth = SERIAL_AUTHFAILED;
                        }
                }

                virtual Bool getRequest(PeerRequest &) override
                {
                        return FALSE;
                }

                virtual void addResponse(const PeerResponse &response) override
                {
                        queueResponse(response);
                }

                virtual Bool getResponse(PeerResponse &response) override
                {
                        if (m_responses.empty())
                        {
                                return FALSE;
                        }

                        response = m_responses.front();
                        m_responses.pop();
                        return TRUE;
                }

                virtual SerialAuthResult getSerialAuthResult(void) override
                {
                        return m_serialAuth;
                }

        private:
                using ResponseQueue = std::queue<PeerResponse>;

                void queueResponse(const PeerResponse &response)
                {
                        m_responses.push(response);
                }

                SerialAuthResult m_serialAuth;
                Bool m_threadStarted;
                ResponseQueue m_responses;
        };
}

GameSpyPeerMessageQueueInterface *TheGameSpyPeerMessageQueue = nullptr;

GameSpyPeerMessageQueueInterface *GameSpyPeerMessageQueueInterface::createNewMessageQueue(void)
{
        return NEW NullPeerQueue();
}

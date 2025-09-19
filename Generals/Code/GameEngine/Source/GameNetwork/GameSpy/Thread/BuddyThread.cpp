/*
**      Command & Conquer Generals(tm)
**
**      Neutral GameSpy buddy worker stub.
*/

#include "PreRTS.h"

#include "GameNetwork/GameSpy/BuddyThread.h"

#include <queue>

namespace
{
        class NullBuddyQueue final : public GameSpyBuddyMessageQueueInterface
        {
        public:
                NullBuddyQueue()
                        : m_threadStarted(FALSE),
                          m_profileID(0)
                {
                }

                virtual void startThread(void) override
                {
                        m_threadStarted = TRUE;
                        m_profileID = 0;
                        clearQueues();
                }

                virtual void endThread(void) override
                {
                        m_threadStarted = FALSE;
                        clearQueues();
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

                virtual void addRequest(const BuddyRequest &request) override
                {
                        if (request.buddyRequestType == BuddyRequest::BUDDYREQUEST_LOGIN)
                        {
                                BuddyResponse response;
                                response.buddyResponseType = BuddyResponse::BUDDYRESPONSE_DISCONNECT;
                                response.result = 0;
                                response.arg.error.errorCode = 0;
                                response.arg.error.errorString[0] = '\0';
                                response.arg.error.fatal = 0;
                                queueResponse(response);
                        }
                }

                virtual Bool getRequest(BuddyRequest &) override
                {
                        return FALSE;
                }

                virtual void addResponse(const BuddyResponse &response) override
                {
                        m_responses.push(response);
                }

                virtual Bool getResponse(BuddyResponse &response) override
                {
                        if (m_responses.empty())
                        {
                                return FALSE;
                        }

                        response = m_responses.front();
                        m_responses.pop();
                        return TRUE;
                }

                virtual GPProfile getLocalProfileID(void) override
                {
                        return m_profileID;
                }

        private:
                using ResponseQueue = std::queue<BuddyResponse>;

                void clearQueues()
                {
                        m_responses = ResponseQueue();
                }

                void queueResponse(const BuddyResponse &response)
                {
                        m_responses.push(response);
                }

                Bool m_threadStarted;
                GPProfile m_profileID;
                ResponseQueue m_responses;
        };
}

GameSpyBuddyMessageQueueInterface *TheGameSpyBuddyMessageQueue = nullptr;

GameSpyBuddyMessageQueueInterface *GameSpyBuddyMessageQueueInterface::createNewMessageQueue(void)
{
        return NEW NullBuddyQueue();
}


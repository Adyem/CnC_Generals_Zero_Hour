/*
**      Command & Conquer Generals(tm) - Zero Hour
**
**      Neutral GameSpy game results worker stub.
*/

#include "PreRTS.h"

#include "GameNetwork/GameSpy/GameResultsThread.h"

#include <queue>

namespace
{
        class NullGameResultsQueue final : public GameResultsInterface
        {
        public:
                NullGameResultsQueue()
                        : m_threadStarted(FALSE)
                {
                }

                virtual void init(void) override {}
                virtual void reset(void) override {}
                virtual void update(void) override {}

                virtual void startThreads(void) override
                {
                        m_threadStarted = TRUE;
                        clearQueues();
                }

                virtual void endThreads(void) override
                {
                        m_threadStarted = FALSE;
                        clearQueues();
                }

                virtual Bool areThreadsRunning(void) override
                {
                        return m_threadStarted;
                }

                virtual void addRequest(const GameResultsRequest &request) override
                {
                        GameResultsResponse response;
                        response.hostname = request.hostname;
                        response.port = request.port;
                        response.sentOk = FALSE;
                        queueResponse(response);
                }

                virtual Bool getRequest(GameResultsRequest &) override
                {
                        return FALSE;
                }

                virtual void addResponse(const GameResultsResponse &response) override
                {
                        m_responses.push(response);
                }

                virtual Bool getResponse(GameResultsResponse &response) override
                {
                        if (m_responses.empty())
                        {
                                return FALSE;
                        }

                        response = m_responses.front();
                        m_responses.pop();
                        return TRUE;
                }

                virtual Bool areGameResultsBeingSent(void) override
                {
                        return !m_responses.empty();
                }

        private:
                using ResponseQueue = std::queue<GameResultsResponse>;

                void clearQueues()
                {
                        m_responses = ResponseQueue();
                }

                void queueResponse(const GameResultsResponse &response)
                {
                        m_responses.push(response);
                }

                Bool m_threadStarted;
                ResponseQueue m_responses;
        };
}

GameResultsInterface *TheGameResultsQueue = nullptr;

GameResultsInterface *GameResultsInterface::createNewGameResultsInterface(void)
{
        return NEW NullGameResultsQueue();
}


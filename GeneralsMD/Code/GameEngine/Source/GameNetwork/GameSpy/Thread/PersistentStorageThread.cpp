/*
**      Command & Conquer Generals(tm) - Zero Hour
**
**      Neutral GameSpy persistent storage worker stub.
*/

#include "PreRTS.h"

#include "GameNetwork/GameSpy/PersistentStorageThread.h"

#include <map>
#include <queue>
#include <algorithm>

namespace
{
        template <typename MapType>
        void MergePerGeneralMap(MapType &destination, const MapType &source)
        {
                        for (auto const &entry : source)
                        {
                                destination[entry.first] += entry.second;
                        }
        }

        class NullPersistentStorageQueue final : public GameSpyPSMessageQueueInterface
        {
        public:
                NullPersistentStorageQueue()
                        : m_threadStarted(FALSE)
                {
                }

                virtual void startThread(void) override
                {
                        m_threadStarted = TRUE;
                        clearQueues();
                }

                virtual void endThread(void) override
                {
                        m_threadStarted = FALSE;
                        clearQueues();
                        m_trackedStats.clear();
                }

                virtual Bool isThreadRunning(void) override
                {
                        return m_threadStarted;
                }

                virtual void addRequest(const PSRequest &request) override
                {
                        PSResponse response;
                        response.responseType = PSResponse::PSRESPONSE_COULDNOTCONNECT;
                        response.player = request.player;
                        response.preorder = FALSE;
                        queueResponse(response);
                }

                virtual Bool getRequest(PSRequest &) override
                {
                        return FALSE;
                }

                virtual void addResponse(const PSResponse &response) override
                {
                        m_responses.push(response);
                }

                virtual Bool getResponse(PSResponse &response) override
                {
                        if (m_responses.empty())
                        {
                                return FALSE;
                        }

                        response = m_responses.front();
                        m_responses.pop();
                        return TRUE;
                }

                virtual void trackPlayerStats(PSPlayerStats stats) override
                {
                        m_trackedStats[stats.id] = stats;
                }

                virtual PSPlayerStats findPlayerStatsByID(Int id) override
                {
                        auto it = m_trackedStats.find(id);
                        if (it != m_trackedStats.end())
                        {
                                return it->second;
                        }

                        PSPlayerStats emptyStats;
                        emptyStats.id = id;
                        return emptyStats;
                }

        private:
                using ResponseQueue = std::queue<PSResponse>;

                void clearQueues()
                {
                        m_responses = ResponseQueue();
                }

                void queueResponse(const PSResponse &response)
                {
                        m_responses.push(response);
                }

                Bool m_threadStarted;
                std::map<Int, PSPlayerStats> m_trackedStats;
                ResponseQueue m_responses;
        };
}

PSPlayerStats::PSPlayerStats()
{
        reset();
}

PSPlayerStats::PSPlayerStats(const PSPlayerStats &other) = default;

void PSPlayerStats::reset(void)
{
        id = 0;
        wins.clear();
        losses.clear();
        games.clear();
        duration.clear();
        unitsKilled.clear();
        unitsLost.clear();
        unitsBuilt.clear();
        buildingsKilled.clear();
        buildingsLost.clear();
        buildingsBuilt.clear();
        earnings.clear();
        techCaptured.clear();
        discons.clear();
        desyncs.clear();
        surrenders.clear();
        gamesOf2p.clear();
        gamesOf3p.clear();
        gamesOf4p.clear();
        gamesOf5p.clear();
        gamesOf6p.clear();
        gamesOf7p.clear();
        gamesOf8p.clear();
        customGames.clear();
        QMGames.clear();
        locale = 0;
        gamesAsRandom = 0;
        options.clear();
        systemSpec.clear();
        lastFPS = 0.0f;
        lastGeneral = -1;
        gamesInRowWithLastGeneral = 0;
        challengeMedals = 0;
        battleHonors = 0;
        QMwinsInARow = 0;
        maxQMwinsInARow = 0;
        winsInARow = 0;
        maxWinsInARow = 0;
        lossesInARow = 0;
        maxLossesInARow = 0;
        disconsInARow = 0;
        maxDisconsInARow = 0;
        desyncsInARow = 0;
        maxDesyncsInARow = 0;
        builtParticleCannon = 0;
        builtNuke = 0;
        builtSCUD = 0;
        lastLadderPort = 0;
        lastLadderHost.clear();
}

void PSPlayerStats::incorporate(const PSPlayerStats &other)
{
        MergePerGeneralMap(wins, other.wins);
        MergePerGeneralMap(losses, other.losses);
        MergePerGeneralMap(games, other.games);
        MergePerGeneralMap(duration, other.duration);
        MergePerGeneralMap(unitsKilled, other.unitsKilled);
        MergePerGeneralMap(unitsLost, other.unitsLost);
        MergePerGeneralMap(unitsBuilt, other.unitsBuilt);
        MergePerGeneralMap(buildingsKilled, other.buildingsKilled);
        MergePerGeneralMap(buildingsLost, other.buildingsLost);
        MergePerGeneralMap(buildingsBuilt, other.buildingsBuilt);
        MergePerGeneralMap(earnings, other.earnings);
        MergePerGeneralMap(techCaptured, other.techCaptured);
        MergePerGeneralMap(discons, other.discons);
        MergePerGeneralMap(desyncs, other.desyncs);
        MergePerGeneralMap(surrenders, other.surrenders);
        MergePerGeneralMap(gamesOf2p, other.gamesOf2p);
        MergePerGeneralMap(gamesOf3p, other.gamesOf3p);
        MergePerGeneralMap(gamesOf4p, other.gamesOf4p);
        MergePerGeneralMap(gamesOf5p, other.gamesOf5p);
        MergePerGeneralMap(gamesOf6p, other.gamesOf6p);
        MergePerGeneralMap(gamesOf7p, other.gamesOf7p);
        MergePerGeneralMap(gamesOf8p, other.gamesOf8p);
        MergePerGeneralMap(customGames, other.customGames);
        MergePerGeneralMap(QMGames, other.QMGames);

        locale = other.locale;
        gamesAsRandom += other.gamesAsRandom;
        if (!other.options.empty())
        {
                options = other.options;
        }
        if (!other.systemSpec.empty())
        {
                systemSpec = other.systemSpec;
        }
        lastFPS = other.lastFPS;
        lastGeneral = other.lastGeneral;
        gamesInRowWithLastGeneral = other.gamesInRowWithLastGeneral;
        challengeMedals += other.challengeMedals;
        battleHonors += other.battleHonors;
        QMwinsInARow = std::max(QMwinsInARow, other.QMwinsInARow);
        maxQMwinsInARow = std::max(maxQMwinsInARow, other.maxQMwinsInARow);
        winsInARow = std::max(winsInARow, other.winsInARow);
        maxWinsInARow = std::max(maxWinsInARow, other.maxWinsInARow);
        lossesInARow = std::max(lossesInARow, other.lossesInARow);
        maxLossesInARow = std::max(maxLossesInARow, other.maxLossesInARow);
        disconsInARow = std::max(disconsInARow, other.disconsInARow);
        maxDisconsInARow = std::max(maxDisconsInARow, other.maxDisconsInARow);
        desyncsInARow = std::max(desyncsInARow, other.desyncsInARow);
        maxDesyncsInARow = std::max(maxDesyncsInARow, other.maxDesyncsInARow);
        builtParticleCannon += other.builtParticleCannon;
        builtNuke += other.builtNuke;
        builtSCUD += other.builtSCUD;
        lastLadderPort = other.lastLadderPort;
        if (!other.lastLadderHost.empty())
        {
                lastLadderHost = other.lastLadderHost;
        }
}

PSRequest::PSRequest()
{
        player.reset();
        requestType = PSREQUEST_READPLAYERSTATS;
        addDiscon = FALSE;
        addDesync = FALSE;
        lastHouse = -1;
}

GameSpyPSMessageQueueInterface *TheGameSpyPSMessageQueue = nullptr;

GameSpyPSMessageQueueInterface *GameSpyPSMessageQueueInterface::createNewMessageQueue(void)
{
        return NEW NullPersistentStorageQueue();
}

std::string GameSpyPSMessageQueueInterface::formatPlayerKVPairs(PSPlayerStats)
{
        return std::string();
}

PSPlayerStats GameSpyPSMessageQueueInterface::parsePlayerKVPairs(std::string)
{
        return PSPlayerStats();
}


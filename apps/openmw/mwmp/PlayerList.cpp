#include <components/openmw-mp/TimedLog.hpp>
#include <apps/openmw/mwclass/creature.hpp>

#include "../mwbase/environment.hpp"

#include "../mwclass/npc.hpp"

#include "../mwmechanics/creaturestats.hpp"

#include "../mwworld/cellstore.hpp"
#include "../mwworld/player.hpp"
#include "../mwworld/worldimp.hpp"

#include "RecordConvertPlayer.hpp"

#include "PlayerList.hpp"
#include "Main.hpp"
#include "DedicatedPlayer.hpp"
#include "CellController.hpp"
#include "GUIController.hpp"
#include "RefNumCompat.hpp"


using namespace mwmp;

std::map <RakNet::RakNetGUID, DedicatedPlayer *> PlayerList::playerList;

void PlayerList::update(float dt)
{
    for (auto &playerEntry : playerList)
    {
        DedicatedPlayer *player = playerEntry.second;
        if (player == nullptr) continue;

        player->update(dt);
    }
}

DedicatedPlayer *PlayerList::newPlayer(RakNet::RakNetGUID guid)
{
    LOG_APPEND(TimedLog::LOG_INFO, "- Creating new DedicatedPlayer with guid %s", guid.ToString());

    playerList[guid] = new DedicatedPlayer(guid);

    LOG_APPEND(TimedLog::LOG_INFO, "- There are now %i DedicatedPlayers", playerList.size());

    return playerList[guid];
}

void PlayerList::deletePlayer(RakNet::RakNetGUID guid)
{
    if (playerList[guid]->reference)
        playerList[guid]->deleteReference();

    delete playerList[guid];
    playerList.erase(guid);
}

void PlayerList::cleanUp()
{
    for (auto &playerEntry : playerList)
        delete playerEntry.second;
}

DedicatedPlayer *PlayerList::getPlayer(RakNet::RakNetGUID guid)
{
    return playerList[guid];
}

DedicatedPlayer *PlayerList::getPlayer(const MWWorld::Ptr &ptr)
{
    for (auto &playerEntry : playerList)
    {
        if (playerEntry.second == nullptr || playerEntry.second->getPtr().mRef == nullptr)
            continue;
        
        const ESM::RefId& refId = ptr.getCellRef().getRefId();
        
        if (playerEntry.second->getPtr().getCellRef().getRefId() == refId)
            return playerEntry.second;
    }

    return nullptr;
}

/*
    0.51 replaced the integer actor id with ESM::RefNum, which every reference already
    carries, so the lookup is now against the cell ref rather than against creature stats.
*/
DedicatedPlayer* PlayerList::getPlayer(ESM::RefNum actorRefNum)
{
    if (!actorRefNum.isSet())
        return nullptr;

    for (auto& playerEntry : playerList)
    {
        if (playerEntry.second == nullptr || playerEntry.second->getPtr().mRef == nullptr)
            continue;

        MWWorld::Ptr playerPtr = playerEntry.second->getPtr();

        if (playerPtr.getCellRef().getRefNum() == actorRefNum)
            return playerEntry.second;
    }

    return nullptr;
}

std::vector<RakNet::RakNetGUID> PlayerList::getPlayersInCell(const mwmp::records::Cell& cell)
{
    std::vector<RakNet::RakNetGUID> playersInCell;

    for (auto& playerEntry : playerList)
    {
        if (playerEntry.first != RakNet::UNASSIGNED_CRABNET_GUID)
        {
            if (Main::get().getCellController()->isSameCell(cell, playerEntry.second->cell))
            {
                playersInCell.push_back(playerEntry.first);
            }
        }
    }

    return playersInCell;
}

std::vector<RakNet::RakNetGUID> PlayerList::getPlayersInCell(const ESM::Cell& cell)
{
    return getPlayersInCell(mwmp::RecordConvert::toMirror(cell));
}

std::vector<RakNet::RakNetGUID> PlayerList::getPlayersInCell(const MWWorld::Cell& cell)
{
    return getPlayersInCell(mwmp::RecordConvert::toMirror(cell));
}

bool PlayerList::isDedicatedPlayer(const MWWorld::Ptr &ptr)
{
    if (ptr.mRef == nullptr)
        return false;

    /*
        Start of tes3mp change (major)

        Players always have 0 as their refNum and mpNum -- but ask RefNumCompat what the
        refNum is rather than reading mIndex directly.

        0.51's CellRef::getOrAssignRefNum gives every reference a RefNum so WorldModel's
        Ptr registry has a key for it, including the ones tes3mp creates for remote
        players. Read raw, mIndex is then non-zero and this returned false for every
        DedicatedPlayer that ever existed -- silently disabling all 35 call sites, which
        between them cover combat, hit handling, knockdown and actor processing.

        RefNumCompat::toWire reports a generated RefNum as 0, which is what this test has
        always meant by "no refNum".
    */
    if (mwmp::RefNumCompat::toWire(ptr.getCellRef()) != 0 || ptr.getCellRef().getMpNum() != 0)
        return false;
    /*
        End of tes3mp change (major)
    */

    return (getPlayer(ptr) != nullptr);
}

void PlayerList::enableMarkers(const mwmp::records::Cell& cell)
{
    for (auto &playerEntry : playerList)
    {
        if (playerEntry.second == nullptr || playerEntry.second->getPtr().mRef == nullptr)
            continue;

        if (Main::get().getCellController()->isSameCell(cell, playerEntry.second->cell))
        {
            playerEntry.second->enableMarker();
        }
    }
}

void PlayerList::enableMarkers(const ESM::Cell& cell)
{
    enableMarkers(mwmp::RecordConvert::toMirror(cell));
}

void PlayerList::enableMarkers(const MWWorld::Cell& cell)
{
    enableMarkers(mwmp::RecordConvert::toMirror(cell));
}

/*
    Go through all DedicatedPlayers checking if their hit attempt actor matches this one
    and clear it if it does

    This resets the combat target for a DedicatedPlayer's followers in Actors::update()

    0.51 spells the identity as ESM::RefNum and the cleared value as a default-constructed
    (unset) one, where 0.8.1 used an int and -1.
*/
void PlayerList::clearHitAttemptActor(ESM::RefNum actorRefNum)
{
    if (!actorRefNum.isSet())
        return;

    for (auto &playerEntry : playerList)
    {
        if (playerEntry.second == nullptr || playerEntry.second->getPtr().mRef == nullptr)
            continue;

        MWMechanics::CreatureStats &playerCreatureStats = playerEntry.second->getPtr().getClass().getCreatureStats(playerEntry.second->getPtr());

        if (playerCreatureStats.getHitAttemptActor() == actorRefNum)
            playerCreatureStats.setHitAttemptActor({});
    }
}

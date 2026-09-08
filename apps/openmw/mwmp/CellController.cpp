#include <components/detournavigator/navigator.hpp>
#include <components/esm3/cellid.hpp>
#include <components/openmw-mp/TimedLog.hpp>
#include <components/openmw-mp/Utils.hpp>

#include "../mwbase/environment.hpp"

#include "../mwworld/containerstore.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/worldimp.hpp"

#include <components/esm/exteriorcelllocation.hpp>

#include "../mwworld/worldmodel.hpp"
#include "../mwworld/scene.hpp"

#include "CellController.hpp"
#include "RecordConvertPlayer.hpp"
#include "Main.hpp"
#include "LocalActor.hpp"
#include "LocalPlayer.hpp"
#include "RefNumCompat.hpp"
using namespace mwmp;

std::map<std::string, mwmp::Cell *> CellController::cellsInitialized;
std::map<std::string, std::string> CellController::localActorsToCells;
std::map<std::string, std::string> CellController::dedicatedActorsToCells;
std::map<std::string, unsigned int> CellController::queuedDeathStates;

mwmp::CellController::CellController()
{

}

CellController::~CellController()
{

}

void CellController::updateLocal(bool forceUpdate)
{
    MWBase::World* world = MWBase::Environment::get().getWorld();

    // Loop through Cells, deleting inactive ones and updating LocalActors in active ones
    for (auto it = cellsInitialized.begin(); it != cellsInitialized.end();)
    {
        mwmp::Cell *mpCell = it->second;

        if (mpCell->getCellStore() == nullptr || mpCell->getCellStore()->getCell() == nullptr || !isActiveWorldCell(*mpCell->getCellStore()->getCell()))
        {
            mpCell->uninitializeLocalActors();
            mpCell->uninitializeDedicatedActors();
            delete it->second;
            cellsInitialized.erase(it++);
        }
        else
        {
            mpCell->updateLocal(forceUpdate);
            ++it;
        }
    }

    // If there are cellsInitialized remaining, loop through them and initialize new LocalActors for eligible ones
    // 
    //
    // Note: This cannot be combined with the above loop because initializing LocalActors in a Cell before they are
    //       deleted from their previous one can make their records stay deleted
    if (cellsInitialized.size() > 0)
    {
        for (auto& cell : cellsInitialized)
        {
            mwmp::Cell* mpCell = cell.second;
            if (mpCell->shouldInitializeActors == true)
            {
                mpCell->shouldInitializeActors = false;
                mpCell->initializeLocalActors();
            }
        }
    }
    /*
        Start of tes3mp change (major)

        0.8.1 disabled the DetourNavigator here when no cells were initialized, as a
        performance hint.

        0.51 replaced Navigator::setUpdatesEnabled(bool) with a scoped update guard --
        updates are batched under makeUpdateGuard() rather than globally switched off, and
        there is no longer a way to express "stop updating". Nothing is lost in
        correctness: with no initialized cells the navigator has nothing to update anyway.

        Dropped deliberately rather than approximated.
    */
    /*
        End of tes3mp change (major)
    */
}

void CellController::updateDedicated(float dt)
{
    for (const auto &cell : cellsInitialized)
        cell.second->updateDedicated(dt);
}

void CellController::initializeCell(const mwmp::records::Cell& cell)
{
    std::string mapIndex = cell.getShortDescription();

    // If this key doesn't exist, create it
    if (cellsInitialized.count(mapIndex) == 0)
    {
        LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Initializing mwmp::Cell %s", cell.getShortDescription().c_str());

        MWWorld::CellStore *cellStore = getCellStore(cell);

        if (!cellStore) return;

        mwmp::Cell *mpCell = new mwmp::Cell(cellStore);
        cellsInitialized[mapIndex] = mpCell;

        LOG_APPEND(TimedLog::LOG_VERBOSE, "- Successfully initialized mwmp::Cell %s", cell.getShortDescription().c_str());
    }
}

void CellController::uninitializeCell(const mwmp::records::Cell& cell)
{
    std::string mapIndex = cell.getShortDescription();

    // If this key exists, erase the key-value pair from the map
    if (cellsInitialized.count(mapIndex) > 0)
    {
        mwmp::Cell* mpCell = cellsInitialized.at(mapIndex);
        mpCell->uninitializeLocalActors();
        mpCell->uninitializeDedicatedActors();
        delete cellsInitialized.at(mapIndex);
        cellsInitialized.erase(mapIndex);
    }
}

void CellController::uninitializeCells()
{
    if (cellsInitialized.size() > 0)
    {
        for (auto it = cellsInitialized.cbegin(); it != cellsInitialized.cend(); it++)
        {
            mwmp::Cell* mpCell = it->second;
            mpCell->uninitializeLocalActors();
            mpCell->uninitializeDedicatedActors();
            delete it->second;
        }

        cellsInitialized.clear();
    }
}

void CellController::readPositions(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readPositions(actorList);
}

void CellController::readAnimFlags(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readAnimFlags(actorList);
}

void CellController::readAnimPlay(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readAnimPlay(actorList);
}

void CellController::readStatsDynamic(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readStatsDynamic(actorList);
}

void CellController::readDeath(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readDeath(actorList);
}

void CellController::readEquipment(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readEquipment(actorList);
}

void CellController::readSpeech(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readSpeech(actorList);
}

void CellController::readSpellsActive(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readSpellsActive(actorList);
}

void CellController::readAi(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readAi(actorList);
}

void CellController::readAttack(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readAttack(actorList);
}

void CellController::readCast(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readCast(actorList);
}

void CellController::readCellChange(ActorList& actorList)
{
    std::string mapIndex = actorList.cell.getShortDescription();

    initializeCell(actorList.cell);

    // If this now exists, send it the data
    if (cellsInitialized.count(mapIndex) > 0)
        cellsInitialized[mapIndex]->readCellChange(actorList);
}

bool CellController::hasQueuedDeathState(MWWorld::Ptr ptr)
{
    std::string actorIndex = generateMapIndex(ptr);

    return queuedDeathStates.count(actorIndex) > 0;
}

unsigned int CellController::getQueuedDeathState(MWWorld::Ptr ptr)
{
    std::string actorIndex = generateMapIndex(ptr);

    return queuedDeathStates[actorIndex];
}

void CellController::clearQueuedDeathState(MWWorld::Ptr ptr)
{
    std::string actorIndex = generateMapIndex(ptr);

    queuedDeathStates.erase(actorIndex);
}

void CellController::setQueuedDeathState(MWWorld::Ptr ptr, unsigned int deathState)
{
    std::string actorIndex = generateMapIndex(ptr);

    queuedDeathStates[actorIndex] = deathState;
}

void CellController::setLocalActorRecord(std::string actorIndex, std::string cellIndex)
{
    localActorsToCells[actorIndex] = cellIndex;
}

void CellController::removeLocalActorRecord(std::string actorIndex)
{
    localActorsToCells.erase(actorIndex);
}

bool CellController::isLocalActor(MWWorld::Ptr ptr)
{
    if (ptr.mRef == nullptr)
        return false;

    std::string actorIndex = generateMapIndex(ptr);

    return localActorsToCells.count(actorIndex) > 0;
}

bool CellController::isLocalActor(int refNum, int mpNum)
{
    std::string actorIndex = generateMapIndex(refNum, mpNum);

    return localActorsToCells.count(actorIndex) > 0;
}

LocalActor *CellController::getLocalActor(MWWorld::Ptr ptr)
{
    std::string actorIndex = generateMapIndex(ptr);
    std::string cellIndex = localActorsToCells.at(actorIndex);

    return cellsInitialized.at(cellIndex)->getLocalActor(actorIndex);
}

LocalActor *CellController::getLocalActor(int refNum, int mpNum)
{
    std::string actorIndex = generateMapIndex(refNum, mpNum);
    std::string cellIndex = localActorsToCells.at(actorIndex);

    return cellsInitialized.at(cellIndex)->getLocalActor(actorIndex);
}

void CellController::setDedicatedActorRecord(std::string actorIndex, std::string cellIndex)
{
    dedicatedActorsToCells[actorIndex] = cellIndex;
}

void CellController::removeDedicatedActorRecord(std::string actorIndex)
{
    dedicatedActorsToCells.erase(actorIndex);
}

bool CellController::isDedicatedActor(MWWorld::Ptr ptr)
{
    if (ptr.mRef == nullptr)
        return false;

    std::string actorIndex = generateMapIndex(ptr);

    return dedicatedActorsToCells.count(actorIndex) > 0;
}

bool CellController::isDedicatedActor(int refNum, int mpNum)
{
    std::string actorIndex = generateMapIndex(refNum, mpNum);

    return dedicatedActorsToCells.count(actorIndex) > 0;
}

DedicatedActor *CellController::getDedicatedActor(MWWorld::Ptr ptr)
{
    std::string actorIndex = generateMapIndex(ptr);
    std::string cellIndex = dedicatedActorsToCells.at(actorIndex);

    return cellsInitialized.at(cellIndex)->getDedicatedActor(actorIndex);
}

DedicatedActor *CellController::getDedicatedActor(int refNum, int mpNum)
{
    std::string actorIndex = generateMapIndex(refNum, mpNum);
    std::string cellIndex = dedicatedActorsToCells.at(actorIndex);

    return cellsInitialized.at(cellIndex)->getDedicatedActor(actorIndex);
}

std::string CellController::generateMapIndex(int refNum, int mpNum)
{
    std::string mapIndex = "";
    mapIndex = Utils::toString(refNum) + "-" + Utils::toString(mpNum);
    return mapIndex;
}

std::string CellController::generateMapIndex(MWWorld::Ptr ptr)
{
    return generateMapIndex(mwmp::RefNumCompat::toWire(ptr.getCellRef()), ptr.getCellRef().getMpNum());
}

std::string CellController::generateMapIndex(BaseActor baseActor)
{
    return generateMapIndex(baseActor.refNum, baseActor.mpNum);
}

bool CellController::hasLocalAuthority(const mwmp::records::Cell& cell)
{
    if (isInitializedCell(cell) && isActiveWorldCell(cell))
        return getCell(cell)->hasLocalAuthority();

    return false;
}

bool CellController::isInitializedCell(const std::string& cellDescription)
{
    return (cellsInitialized.count(cellDescription) > 0);
}

bool CellController::isInitializedCell(const mwmp::records::Cell& cell)
{
    return isInitializedCell(cell.getShortDescription());
}

/*
    Start of tes3mp change (major)

    0.8.1 put this loop behind a World::isCellActive() hook on MWBase::World. It never
    needed to live there: the body is entirely mwmp logic -- it compares cells with
    CellController's own isSameCell -- and the only reason for the hook was that
    Scene::getActiveCells() was not reachable from outside MWWorld. 0.51 exposes the
    scene through Environment::getWorldScene(), so the hook is gone and the logic is
    here, which is one fewer engine interface to re-apply on the next port.
*/
bool CellController::isActiveWorldCell(const mwmp::records::Cell& cell)
{
    const MWWorld::Scene::CellStoreCollection& activeCells
        = MWBase::Environment::get().getWorldScene()->getActiveCells();

    for (const MWWorld::CellStore* activeCell : activeCells)
    {
        if (isSameCell(cell, *activeCell->getCell()))
            return true;
    }

    return false;
}
/*
    End of tes3mp change (major)
*/

Cell *CellController::getCell(const mwmp::records::Cell& cell)
{
    return cellsInitialized.at(cell.getShortDescription());
}

MWWorld::CellStore *CellController::getCellStore(const mwmp::records::Cell& cell)
{
    MWWorld::CellStore *cellStore;

    /*
        0.51 moved cell lookup off MWBase::World and onto WorldModel, and exteriors are
        now addressed by ExteriorCellLocation -- grid position plus a worldspace, so that
        ESM4 worldspaces can coexist with Morrowind's. The protocol only ever describes
        cells in the Morrowind worldspace, so that is what is named here.
    */
    MWWorld::WorldModel* worldModel = MWBase::Environment::get().getWorldModel();

    try
    {
        if (cell.isExterior())
            cellStore = &worldModel->getExterior(
                ESM::ExteriorCellLocation(cell.mData.mX, cell.mData.mY, ESM::Cell::sDefaultWorldspaceId));
        else
            cellStore = &worldModel->getInterior(cell.mName);
    }
    catch (std::exception&)
    {
        cellStore = nullptr;
    }

    return cellStore;
}

bool CellController::isSameCell(const mwmp::records::Cell& cell, const mwmp::records::Cell& otherCell)
{
    if (&cell == nullptr || &otherCell == nullptr) return false;

    bool isCellExterior = false;
    bool isOtherCellExterior = false;

    try
    {
        isCellExterior = cell.isExterior();
        isOtherCellExterior = otherCell.isExterior();
    }
    catch (std::exception& e)
    {
        LOG_MESSAGE_SIMPLE(TimedLog::LOG_ERROR, "Failed cell comparison");
        return false;
    }

    if (isCellExterior && isOtherCellExterior)
    {
        if (cell.mData.mX == otherCell.mData.mX && cell.mData.mY == otherCell.mData.mY)
            return true;
    }
    else if (Misc::StringUtils::ciEqual(cell.mName, otherCell.mName))
        return true;

    return false;
}

int CellController::getCellSize() const
{
    return 8192;
}

/*
    Start of tes3mp change (major)

    Converting overloads: engine code still holds ESM::Cell, while the protocol layer holds
    the mirror. Convert once here rather than at every call site.
*/
namespace
{
    mwmp::records::Cell toMirror(const ESM::Cell& cell)
    {
        return mwmp::RecordConvert::toMirror(cell);
    }

    // 0.51's unified cell view; CellStore::getCell() hands this out now.
    mwmp::records::Cell toMirror(const MWWorld::Cell& cell)
    {
        return mwmp::RecordConvert::toMirror(cell);
    }
}

void CellController::initializeCell(const ESM::Cell& cell) { initializeCell(toMirror(cell)); }
void CellController::uninitializeCell(const ESM::Cell& cell) { uninitializeCell(toMirror(cell)); }
bool CellController::hasLocalAuthority(const ESM::Cell& cell) { return hasLocalAuthority(toMirror(cell)); }
bool CellController::isInitializedCell(const ESM::Cell& cell) { return isInitializedCell(toMirror(cell)); }
bool CellController::isActiveWorldCell(const ESM::Cell& cell) { return isActiveWorldCell(toMirror(cell)); }
Cell* CellController::getCell(const ESM::Cell& cell) { return getCell(toMirror(cell)); }
MWWorld::CellStore* CellController::getCellStore(const ESM::Cell& cell) { return getCellStore(toMirror(cell)); }
bool CellController::isSameCell(const ESM::Cell& cell, const ESM::Cell& otherCell)
{
    return isSameCell(toMirror(cell), toMirror(otherCell));
}
/*
    End of tes3mp change (major)
*/

bool CellController::isSameCell(const mwmp::records::Cell& cell, const ESM::Cell& otherCell)
{
    return isSameCell(cell, toMirror(otherCell));
}

bool CellController::isSameCell(const ESM::Cell& cell, const mwmp::records::Cell& otherCell)
{
    return isSameCell(toMirror(cell), otherCell);
}

void CellController::initializeCell(const MWWorld::Cell& cell) { initializeCell(toMirror(cell)); }
void CellController::uninitializeCell(const MWWorld::Cell& cell) { uninitializeCell(toMirror(cell)); }
bool CellController::hasLocalAuthority(const MWWorld::Cell& cell) { return hasLocalAuthority(toMirror(cell)); }
bool CellController::isInitializedCell(const MWWorld::Cell& cell) { return isInitializedCell(toMirror(cell)); }
bool CellController::isActiveWorldCell(const MWWorld::Cell& cell) { return isActiveWorldCell(toMirror(cell)); }
Cell* CellController::getCell(const MWWorld::Cell& cell) { return getCell(toMirror(cell)); }
MWWorld::CellStore* CellController::getCellStore(const MWWorld::Cell& cell) { return getCellStore(toMirror(cell)); }

bool CellController::isSameCell(const MWWorld::Cell& cell, const MWWorld::Cell& otherCell)
{
    return isSameCell(toMirror(cell), toMirror(otherCell));
}

bool CellController::isSameCell(const mwmp::records::Cell& cell, const MWWorld::Cell& otherCell)
{
    return isSameCell(cell, toMirror(otherCell));
}

bool CellController::isSameCell(const MWWorld::Cell& cell, const mwmp::records::Cell& otherCell)
{
    return isSameCell(toMirror(cell), otherCell);
}

#ifndef OPENMW_CELLCONTROLLER_HPP
#define OPENMW_CELLCONTROLLER_HPP

#include <components/openmw-mp/Base/records/PlayerState.hpp>
#include "Cell.hpp"
#include "ActorList.hpp"
#include "LocalActor.hpp"
#include "DedicatedActor.hpp"
#include "../mwworld/cellstore.hpp"

namespace mwmp
{
    class CellController
    {
    public:

        CellController();
        virtual ~CellController();

        void updateLocal(bool forceUpdate);
        void updateDedicated(float dt);

        void initializeCell(const mwmp::records::Cell& cell);
        void uninitializeCell(const mwmp::records::Cell& cell);
        void uninitializeCells();

        void readPositions(mwmp::ActorList& actorList);
        void readAnimFlags(mwmp::ActorList& actorList);
        void readAnimPlay(mwmp::ActorList& actorList);
        void readStatsDynamic(mwmp::ActorList& actorList);
        void readDeath(mwmp::ActorList& actorList);
        void readEquipment(mwmp::ActorList& actorList);
        void readSpeech(mwmp::ActorList& actorList);
        void readSpellsActive(mwmp::ActorList& actorList);
        void readAi(mwmp::ActorList& actorList);
        void readAttack(mwmp::ActorList& actorList);
        void readCast(mwmp::ActorList& actorList);
        void readCellChange(mwmp::ActorList& actorList);

        bool hasQueuedDeathState(MWWorld::Ptr ptr);
        unsigned int getQueuedDeathState(MWWorld::Ptr ptr);
        void clearQueuedDeathState(MWWorld::Ptr ptr);
        void setQueuedDeathState(MWWorld::Ptr ptr, unsigned int deathState);

        void setLocalActorRecord(std::string actorIndex, std::string cellIndex);
        void removeLocalActorRecord(std::string actorIndex);
        
        bool isLocalActor(MWWorld::Ptr ptr);
        bool isLocalActor(int refNum, int mpNum);
        virtual LocalActor *getLocalActor(MWWorld::Ptr ptr);
        virtual LocalActor *getLocalActor(int refNum, int mpNum);

        void setDedicatedActorRecord(std::string actorIndex, std::string cellIndex);
        void removeDedicatedActorRecord(std::string actorIndex);
        
        bool isDedicatedActor(MWWorld::Ptr ptr);
        bool isDedicatedActor(int refNum, int mpNum);
        virtual DedicatedActor *getDedicatedActor(MWWorld::Ptr ptr);
        virtual DedicatedActor *getDedicatedActor(int refNum, int mpNum);

        std::string generateMapIndex(int refNumindex, int mpNum);
        std::string generateMapIndex(MWWorld::Ptr ptr);
        std::string generateMapIndex(mwmp::BaseActor baseActor);

        bool hasLocalAuthority(const mwmp::records::Cell& cell);
        bool isInitializedCell(const std::string& cellDescription);
        bool isInitializedCell(const mwmp::records::Cell& cell);
        bool isActiveWorldCell(const mwmp::records::Cell& cell);
        virtual Cell *getCell(const mwmp::records::Cell& cell);

        virtual MWWorld::CellStore *getCellStore(const mwmp::records::Cell& cell);

        bool isSameCell(const mwmp::records::Cell& cell, const mwmp::records::Cell& otherCell);

        /*
            Start of tes3mp change (major)

            These take the protocol's mirror Cell, because that is what arrives from the
            server and what BasePlayer/BaseActor hold. Engine code still calls them with an
            ESM::Cell, so converting overloads are provided rather than making every caller
            convert.
        */
        void initializeCell(const ESM::Cell& cell);
        void uninitializeCell(const ESM::Cell& cell);
        bool hasLocalAuthority(const ESM::Cell& cell);
        bool isInitializedCell(const ESM::Cell& cell);
        bool isActiveWorldCell(const ESM::Cell& cell);
        Cell* getCell(const ESM::Cell& cell);
        MWWorld::CellStore* getCellStore(const ESM::Cell& cell);
        bool isSameCell(const ESM::Cell& cell, const ESM::Cell& otherCell);
        // Mixed pairs occur where engine code compares a player's wire cell with a live one
        bool isSameCell(const mwmp::records::Cell& cell, const ESM::Cell& otherCell);
        bool isSameCell(const ESM::Cell& cell, const mwmp::records::Cell& otherCell);
        /*
            End of tes3mp change (major)
        */

        int getCellSize() const;

    private:
        static std::map<std::string, mwmp::Cell *> cellsInitialized;
        static std::map<std::string, std::string> localActorsToCells;
        static std::map<std::string, std::string> dedicatedActorsToCells;
        static std::map<std::string, unsigned int> queuedDeathStates;
    };
}

#endif //OPENMW_CELLCONTROLLER_HPP

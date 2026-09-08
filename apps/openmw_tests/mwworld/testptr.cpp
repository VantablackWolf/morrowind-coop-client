#include "apps/openmw/mwclass/npc.hpp"
#include "apps/openmw/mwworld/esmstore.hpp"
#include "apps/openmw/mwworld/livecellref.hpp"
#include "apps/openmw/mwworld/ptr.hpp"
#include "apps/openmw/mwworld/worldmodel.hpp"

#include <components/esm3/loadnpc.hpp>
#include <components/esm3/readerscache.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace MWWorld
{
    namespace
    {
        using namespace testing;

        TEST(MWWorldPtrTest, toStringShouldReturnHumanReadableTextRepresentationOfPtrWithNullRef)
        {
            Ptr ptr;
            EXPECT_EQ(ptr.toString(), "null object");
        }

        TEST(MWWorldPtrTest, toStringShouldReturnHumanReadableTextRepresentationOfPtrWithDeletedRef)
        {
            MWClass::Npc::registerSelf();
            ESM::NPC npc;
            npc.blank();
            npc.mId = ESM::RefId::stringRefId("Player");
            ESMStore store;
            store.insert(npc);
            ESM::CellRef cellRef;
            cellRef.blank();
            cellRef.mRefID = npc.mId;
            cellRef.mRefNum = ESM::RefNum{ .mIndex = 0x2a, .mContentFile = 0xd };
            LiveCellRef<ESM::NPC> liveCellRef(cellRef, &npc);
            liveCellRef.mData.setDeletedByContentFile(true);
            Ptr ptr(&liveCellRef);
            /*
                Start of tes3mp change (minor)

                Ptr::toString() also reports mpNum, the multiplayer half of an object's
                identity. A RefNum alone does not identify an object created at runtime by
                the server, so every TES3MP log line that names a Ptr needs it too.
            */
            EXPECT_THAT(ptr.toString(), StrCaseEq("deleted object0xd00002a (NPC, \"player\", mpNum 0)"));
            /*
                End of tes3mp change (minor)
            */
        }

        TEST(MWWorldPtrTest, toStringShouldReturnHumanReadableTextRepresentationOfPtr)
        {
            MWClass::Npc::registerSelf();
            ESM::NPC npc;
            npc.blank();
            npc.mId = ESM::RefId::stringRefId("Player");
            ESMStore store;
            store.insert(npc);
            ESM::CellRef cellRef;
            cellRef.blank();
            cellRef.mRefID = npc.mId;
            cellRef.mRefNum = ESM::RefNum{ .mIndex = 0x2a, .mContentFile = 0xd };
            LiveCellRef<ESM::NPC> liveCellRef(cellRef, &npc);
            Ptr ptr(&liveCellRef);
            /*
                Start of tes3mp change (minor)

                Ptr::toString() also reports mpNum, the multiplayer half of an object's
                identity. A RefNum alone does not identify an object created at runtime by
                the server, so every TES3MP log line that names a Ptr needs it too.
            */
            EXPECT_THAT(ptr.toString(), StrCaseEq("object0xd00002a (NPC, \"player\", mpNum 0)"));
            /*
                End of tes3mp change (minor)
            */
        }

        TEST(MWWorldPtrTest, underlyingLiveCellRefShouldBeDeregisteredOnDestruction)
        {
            MWClass::Npc::registerSelf();
            ESM::NPC npc;
            npc.blank();
            npc.mId = ESM::RefId::stringRefId("Player");
            ESMStore store;
            store.insert(npc);
            ESM::ReadersCache readersCache;
            WorldModel worldModel(store, readersCache);
            ESM::CellRef cellRef;
            cellRef.blank();
            cellRef.mRefID = npc.mId;
            cellRef.mRefNum = ESM::FormId{ .mIndex = 0x2a, .mContentFile = 0xd };
            {
                LiveCellRef<ESM::NPC> liveCellRef(cellRef, &npc);
                Ptr ptr(&liveCellRef);
                worldModel.registerPtr(ptr);
                ASSERT_EQ(worldModel.getPtr(cellRef.mRefNum), ptr);
            }
            EXPECT_EQ(worldModel.getPtr(cellRef.mRefNum), Ptr());
        }
    }
}

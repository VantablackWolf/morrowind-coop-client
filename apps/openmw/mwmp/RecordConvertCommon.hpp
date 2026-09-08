#ifndef OPENMW_MWMP_RECORDCONVERTCOMMON_H
#define OPENMW_MWMP_RECORDCONVERTCOMMON_H

/*
    Conversions for the shared sub-structs in
    components/openmw-mp/Base/records/Common.hpp.

    Client-side only. The dedicated server never includes this.
*/

#include <cstdint>

#include <components/esm/attr.hpp>
#include <components/esm3/aipackage.hpp>
#include <components/esm3/effectlist.hpp>
#include <components/esm3/loadarmo.hpp>
#include <components/esm3/loadcont.hpp>
#include <components/esm3/loadmgef.hpp>
#include <components/esm3/loadnpc.hpp>
#include <components/esm3/loadskil.hpp>

#include <components/openmw-mp/Base/records/Common.hpp>

#include "RefIdCompat.hpp"

namespace mwmp
{
    namespace RecordConvert
    {
        /*
            Effects.

            0.47 carries magic effect / skill / attribute as small integers with
            -1 meaning "not applicable". 0.51 wants RefIds with EmptyRefId for
            the same thing. OpenMW's indexToRefId()/refIdToIndex() already honour
            that convention in both directions, so no -1 special case is needed.
        */
        inline void toEngine(const records::ENAMstruct& from, ESM::ENAMstruct& to)
        {
            to.mEffectID = ESM::MagicEffect::indexToRefId(from.mEffectID);
            to.mSkill = ESM::Skill::indexToRefId(from.mSkill);
            to.mAttribute = ESM::Attribute::indexToRefId(from.mAttribute);
            to.mRange = from.mRange;
            to.mArea = from.mArea;
            to.mDuration = from.mDuration;
            to.mMagnMin = from.mMagnMin;
            to.mMagnMax = from.mMagnMax;
        }

        inline void fromEngine(const ESM::ENAMstruct& from, records::ENAMstruct& to)
        {
            to.mEffectID = static_cast<short>(ESM::MagicEffect::refIdToIndex(from.mEffectID));
            to.mSkill = static_cast<signed char>(ESM::Skill::refIdToIndex(from.mSkill));
            to.mAttribute = static_cast<signed char>(ESM::Attribute::refIdToIndex(from.mAttribute));
            to.mRange = from.mRange;
            to.mArea = from.mArea;
            to.mDuration = from.mDuration;
            to.mMagnMin = from.mMagnMin;
            to.mMagnMax = from.mMagnMax;
        }

        inline void toEngine(const records::EffectList& from, ESM::EffectList& to)
        {
            to.mList.clear();
            to.mList.reserve(from.mList.size());
            std::uint32_t index = 0;
            for (const auto& effect : from.mList)
            {
                // mIndex is engine bookkeeping, not carried on the wire, so it
                // is assigned positionally to match load order.
                ESM::IndexedENAMstruct indexed;
                toEngine(effect, indexed.mData);
                indexed.mIndex = index++;
                to.mList.push_back(indexed);
            }
        }

        inline void fromEngine(const ESM::EffectList& from, records::EffectList& to)
        {
            to.mList.clear();
            to.mList.reserve(from.mList.size());
            for (const auto& indexed : from.mList)
            {
                records::ENAMstruct effect;
                fromEngine(indexed.mData, effect);
                to.mList.push_back(effect);
            }
        }

        // Inventory.
        inline void toEngine(const records::InventoryList& from, ESM::InventoryList& to)
        {
            to.mList.clear();
            to.mList.reserve(from.mList.size());
            for (const auto& item : from.mList)
            {
                ESM::ContItem converted;
                converted.mCount = item.mCount;
                // Contents may name records this client has not created yet.
                converted.mItem = RefIdCompat::fromWireCreate(item.mItem);
                to.mList.push_back(converted);
            }
        }

        inline void fromEngine(const ESM::InventoryList& from, records::InventoryList& to)
        {
            to.mList.clear();
            to.mList.reserve(from.mList.size());
            for (const auto& item : from.mList)
            {
                records::ContItem converted;
                converted.mCount = item.mCount;
                converted.mItem = RefIdCompat::toWire(item.mItem);
                to.mList.push_back(converted);
            }
        }

        // Body parts.
        inline void toEngine(const records::PartReferenceList& from, ESM::PartReferenceList& to)
        {
            to.mParts.clear();
            to.mParts.reserve(from.mParts.size());
            for (const auto& part : from.mParts)
            {
                ESM::PartReference converted;
                converted.mPart = part.mPart;
                converted.mMale = RefIdCompat::fromWireCreate(part.mMale);
                converted.mFemale = RefIdCompat::fromWireCreate(part.mFemale);
                to.mParts.push_back(converted);
            }
        }

        inline void fromEngine(const ESM::PartReferenceList& from, records::PartReferenceList& to)
        {
            to.mParts.clear();
            to.mParts.reserve(from.mParts.size());
            for (const auto& part : from.mParts)
            {
                records::PartReference converted;
                converted.mPart = part.mPart;
                converted.mMale = RefIdCompat::toWire(part.mMale);
                converted.mFemale = RefIdCompat::toWire(part.mFemale);
                to.mParts.push_back(converted);
            }
        }

        // AI data -- member-wise copy, no type changes on the serialized subset.
        inline void toEngine(const records::AIData& from, ESM::AIData& to)
        {
            to.mFight = from.mFight;
            to.mFlee = from.mFlee;
            to.mAlarm = from.mAlarm;
            to.mServices = from.mServices;
        }

        inline void fromEngine(const ESM::AIData& from, records::AIData& to)
        {
            to.mFight = from.mFight;
            to.mFlee = from.mFlee;
            to.mAlarm = from.mAlarm;
            to.mServices = from.mServices;
        }
    }
}

#endif

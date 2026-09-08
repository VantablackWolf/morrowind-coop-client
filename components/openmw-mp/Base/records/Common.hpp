#ifndef OPENMW_MP_RECORDS_COMMON_H
#define OPENMW_MP_RECORDS_COMMON_H

/*
    Shared sub-structs mirrored for the TES3MP protocol layer.

    Field types here are copied from OpenMW 0.47 -- the version that defines the
    current TES3MP wire encoding -- NOT from 0.51. That is what keeps existing
    0.8.1 servers and clients interoperable. Where 0.51 changed a type, the
    conversion happens in apps/openmw/mwmp/RecordConvert.hpp and nowhere else.

    Only fields TES3MP actually serializes are present.
*/

#include <cstdint>
#include <string>
#include <vector>

namespace mwmp
{
    namespace records
    {
        /*
            Mirrors ESM::ENAMstruct.

            This one is NOT a plain string<->RefId case. In 0.47 these three
            fields are small integers; in 0.51 they became ESM::RefId:

                short mEffectID       -> ESM::RefId (magic effect)
                signed char mSkill    -> ESM::RefId (skill)
                signed char mAttribute-> ESM::RefId (attribute)

            The wire keeps the integers. Conversion goes through OpenMW's own
            ESM::MagicEffect / ESM::Skill / ESM::Attribute indexToRefId() and
            refIdToIndex() helpers, which already map -1 <-> EmptyRefId, so the
            0.47 "N/A is -1" convention survives untouched. Verified empirically:
            indexToRefId(-1).empty() is true, refIdToIndex(EmptyRefId) is -1.
        */
        struct ENAMstruct
        {
            short mEffectID;
            signed char mSkill;
            signed char mAttribute;
            int mRange;
            int mArea;
            int mDuration;
            int mMagnMin;
            int mMagnMax;
        };

        /*
            Mirrors ESM::EffectList.

            0.51 changed the element type from ENAMstruct to IndexedENAMstruct,
            which wraps the effect in { ENAMstruct mData; uint32_t mIndex; }.
            The index is engine bookkeeping and is not on the wire, so the
            converter assigns it positionally.
        */
        struct EffectList
        {
            std::vector<ENAMstruct> mList;
        };

        // Mirrors ESM::ContItem. mItem is ESM::RefId in 0.51.
        struct ContItem
        {
            int mCount;
            std::string mItem;
        };

        // Mirrors ESM::InventoryList.
        struct InventoryList
        {
            std::vector<ContItem> mList;
        };

        // Mirrors ESM::PartReference. Both id fields are ESM::RefId in 0.51.
        struct PartReference
        {
            unsigned char mPart;
            std::string mMale;
            std::string mFemale;
        };

        // Mirrors ESM::PartReferenceList.
        struct PartReferenceList
        {
            std::vector<PartReference> mParts;
        };

        /*
            Mirrors the serialized subset of ESM::AIData. Layout is unchanged
            between 0.47 and 0.51 apart from padding fields that TES3MP never
            sent, so no conversion is needed beyond a member-wise copy.
        */
        struct AIData
        {
            unsigned char mFight;
            unsigned char mFlee;
            unsigned char mAlarm;
            int mServices;
        };

        /*
            Mirrors the serialized subset of ESM::NPDTstruct52.
            mLevel is short; the three stats are unsigned short.
        */
        struct NPDTstruct
        {
            short mLevel;
            unsigned short mHealth;
            unsigned short mMana;
            unsigned short mFatigue;
        };
    }
}

#endif

#ifndef OPENMW_MWMP_RECORDCONVERT_H
#define OPENMW_MWMP_RECORDCONVERT_H

/*
    Client-side conversion between the protocol's plain mirror records
    (components/openmw-mp/Base/records/) and the engine's ESM3 records.

    SAMPLE -- this is the shape proposed for all 25 record types.

    This is the ONLY place refIds cross between wire form and engine form on
    the client. The dedicated server never includes this header, and never
    sees ESM::RefId at all.
*/

#include <components/esm3/loadarmo.hpp>
#include <components/openmw-mp/Base/records/Armor.hpp>

#include "RefIdCompat.hpp"

namespace mwmp
{
    namespace RecordConvert
    {
        inline void toEngine(const records::PartReference& from, ESM::PartReference& to)
        {
            to.mPart = from.mPart;
            // Body part ids arrive from server scripts and may name records this
            // client has not created yet, so these must intern rather than look up.
            to.mMale = RefIdCompat::fromWireCreate(from.mMale);
            to.mFemale = RefIdCompat::fromWireCreate(from.mFemale);
        }

        inline void toEngine(const records::Armor& from, ESM::Armor& to)
        {
            // POD block copies straight across; note mData.mEnchant is the
            // enchantment point value, NOT the enchantment record id.
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mData.mArmor = from.mData.mArmor;

            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;

            // Custom records are created on the fly, so the record this id names
            // may not exist yet -- fromWireExisting() would silently yield an
            // empty RefId here and the record would be dropped.
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mEnchant = RefIdCompat::fromWireCreate(from.mEnchant);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);

            to.mParts.mParts.clear();
            to.mParts.mParts.reserve(from.mParts.mParts.size());
            for (const auto& part : from.mParts.mParts)
            {
                ESM::PartReference converted;
                toEngine(part, converted);
                to.mParts.mParts.push_back(converted);
            }
        }

        inline void fromEngine(const ESM::Armor& from, records::Armor& to)
        {
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mData.mArmor = from.mData.mArmor;

            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;

            to.mId = RefIdCompat::toWire(from.mId);
            to.mEnchant = RefIdCompat::toWire(from.mEnchant);
            to.mScript = RefIdCompat::toWire(from.mScript);

            to.mParts.mParts.clear();
            to.mParts.mParts.reserve(from.mParts.mParts.size());
            for (const auto& part : from.mParts.mParts)
            {
                records::PartReference converted;
                converted.mPart = part.mPart;
                converted.mMale = RefIdCompat::toWire(part.mMale);
                converted.mFemale = RefIdCompat::toWire(part.mFemale);
                to.mParts.mParts.push_back(converted);
            }
        }
    }
}

#endif

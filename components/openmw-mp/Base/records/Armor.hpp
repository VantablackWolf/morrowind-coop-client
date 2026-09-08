#ifndef OPENMW_MP_RECORDS_ARMOR_H
#define OPENMW_MP_RECORDS_ARMOR_H

/*
    Plain mirror of ESM::Armor for the TES3MP protocol layer.

    SAMPLE -- this is the shape proposed for all 25 record types.

    Why this exists: OpenMW 0.49 changed record ids from std::string to
    ESM::RefId. The TES3MP wire format encodes them as plain strings, and the
    dedicated server (apps/openmw-mp) previously pulled in components/esm just
    to hold these structs. Mirroring them here means:

      - the server stops depending on the engine's ESM headers entirely, so a
        future OpenMW bump is a client-only problem
      - PacketRecordDynamic.cpp needs NO changes: RW(recordData.mId, send, true)
        still sees a std::string
      - the wire format is preserved by construction, because these field types
        are copied from OpenMW 0.47 -- the version that defines the current
        encoding -- not from 0.51

    Only the fields TES3MP actually serializes are mirrored (14 for Armor; 215
    across all 25 types). Fields the protocol never touches are deliberately
    absent rather than carried dead.
*/

#include <cstdint>
#include <string>
#include <vector>

namespace mwmp
{
    namespace records
    {
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

        struct Armor
        {
            // Mirrors ESM::Armor::AODTstruct. Pure POD, unchanged 0.47 -> 0.51.
            //
            // Note mEnchant here is the enchantment POINT VALUE (int), which is
            // a different field from Armor::mEnchant below (the enchantment
            // record id). They are easy to confuse and the compiler will not
            // catch a swap, since one is int and the other was std::string.
            struct AODTstruct
            {
                int mType;
                float mWeight;
                int mValue;
                int mHealth;
                int mEnchant;
                int mArmor;
            };

            AODTstruct mData;

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;

            // ESM::RefId in 0.51 -- converted at the engine boundary only.
            std::string mEnchant;
            std::string mScript;

            PartReferenceList mParts;
        };
    }
}

#endif

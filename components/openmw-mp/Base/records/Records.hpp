#ifndef OPENMW_MP_RECORDS_RECORDS_H
#define OPENMW_MP_RECORDS_RECORDS_H

/*
    Plain mirrors of the 25 ESM record types TES3MP synchronises.

    GENERATED from the OpenMW 0.47 headers -- the version that defines the
    current TES3MP wire encoding -- restricted to the 215 fields that
    PacketRecordDynamic.cpp actually serializes. Fields the protocol never
    sends are deliberately absent rather than carried dead.

    Consequences of mirroring rather than including components/esm3:
      - apps/openmw-mp (the dedicated server) no longer depends on the engine's
        ESM headers at all, so a future OpenMW bump is a client-only problem
      - PacketRecordDynamic.cpp is unchanged: every RW() call still sees the
        same type it saw in 0.8.1, so the wire format cannot drift
      - all engine-side type changes (std::string -> ESM::RefId, int -> RefId
        for effect/skill/attribute indices) are confined to
        apps/openmw/mwmp/RecordConvert*.hpp

    Do not "fix" a field type here to match 0.51. These types define the
    protocol; changing one is a wire-format break.
*/

#include <cstdint>
#include <string>
#include <vector>

#include "Common.hpp"

namespace mwmp
{
    namespace records
    {
        struct Spell
        {
            struct SPDTstruct
            {
                int mType;
                int mCost;
                int mFlags;
            };

            std::string mId;
            std::string mName;
            SPDTstruct mData;
            records::EffectList mEffects;
        };

        struct Potion
        {
            struct ALDTstruct
            {
                float mWeight;
                int mValue;
                int mAutoCalc;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            ALDTstruct mData;
            std::string mScript;
            records::EffectList mEffects;
        };

        struct Enchantment
        {
            struct ENDTstruct
            {
                int mType;
                int mCost;
                int mCharge;
                int mFlags;
            };

            std::string mId;
            ENDTstruct mData;
            records::EffectList mEffects;
        };

        struct Armor
        {
            struct AODTstruct
            {
                int mType;
                float mWeight;
                int mValue;
                int mHealth;
                int mArmor;
                int mEnchant;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            AODTstruct mData;
            std::string mEnchant;
            std::string mScript;
            records::PartReferenceList mParts;
        };

        struct Book
        {
            struct BKDTstruct
            {
                float mWeight;
                int mValue;
                int mIsScroll;
                int mSkillId;
                int mEnchant;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            std::string mText;
            BKDTstruct mData;
            std::string mEnchant;
            std::string mScript;
        };

        struct Clothing
        {
            struct CTDTstruct
            {
                int mType;
                float mWeight;
                unsigned short mValue;
                unsigned short mEnchant;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            CTDTstruct mData;
            std::string mEnchant;
            std::string mScript;
            records::PartReferenceList mParts;
        };

        struct Miscellaneous
        {
            struct MCDTstruct
            {
                float mWeight;
                int mValue;
                int mIsKey;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            MCDTstruct mData;
            std::string mScript;
        };

        struct Weapon
        {
            struct WPDTstruct
            {
                short mType;
                float mWeight;
                int mValue;
                unsigned short mHealth;
                float mSpeed;
                float mReach;
                unsigned char mChop[2];
                unsigned char mSlash[2];
                unsigned char mThrust[2];
                int mFlags;
                unsigned short mEnchant;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            WPDTstruct mData;
            std::string mEnchant;
            std::string mScript;
        };

        struct Activator
        {
            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mScript;
        };

        struct Apparatus
        {
            struct AADTstruct
            {
                int mType;
                float mWeight;
                int mValue;
                float mQuality;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            AADTstruct mData;
            std::string mScript;
        };

        struct BodyPart
        {
            struct BYDTstruct
            {
                unsigned char mType;
                unsigned char mPart;
                unsigned char mVampire;
                unsigned char mFlags;
            };

            std::string mId;
            std::string mModel;
            std::string mRace;
            BYDTstruct mData;
        };

        struct Cell
        {
            std::string mName;
        };

        struct Container
        {
            std::string mId;
            std::string mName;
            std::string mModel;
            float mWeight;
            int mFlags;
            std::string mScript;
            records::InventoryList mInventory;
        };

        struct Creature
        {
            struct NPDTstruct
            {
                int mType;
                int mLevel;
                int mHealth;
                int mMana;
                int mFatigue;
                int mSoul;
                int mAttack[6];
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            float mScale;
            int mBloodType;
            NPDTstruct mData;
            records::AIData mAiData;
            unsigned char mFlags;
            std::string mScript;
            records::InventoryList mInventory;
        };

        struct Door
        {
            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mOpenSound;
            std::string mCloseSound;
            std::string mScript;
        };

        struct GameSetting
        {
            std::string mId;
            // NOTE: ESM::GameSetting::mValue (an ESM::Variant) is deliberately
            // absent. The wire carries mwmp::GameSettingRecord::variable
            // instead, and the Variant is built in RecordConvert.
        };

        struct Ingredient
        {
            struct IRDTstruct
            {
                float mWeight;
                int mValue;
                int mEffectID[4];
                int mAttributes[4];
                int mSkills[4];
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            IRDTstruct mData;
            std::string mScript;
        };

        struct Light
        {
            struct LHDTstruct
            {
                float mWeight;
                int mValue;
                int mTime;
                int mRadius;
                unsigned int mColor;
                int mFlags;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            std::string mSound;
            LHDTstruct mData;
            std::string mScript;
        };

        struct Lockpick
        {
            struct Data
            {
                float mWeight;
                int mValue;
                float mQuality;
                int mUses;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            Data mData;
            std::string mScript;
        };

        struct NPC
        {
            std::string mId;
            std::string mName;
            unsigned char mFlags;
            std::string mRace;
            std::string mModel;
            std::string mHair;
            std::string mHead;
            std::string mClass;
            std::string mFaction;
            std::string mScript;
            records::NPDTstruct mNpdt;
            records::AIData mAiData;
            unsigned char mNpdtType;
            records::InventoryList mInventory;
        };

        struct Probe
        {
            struct Data
            {
                float mWeight;
                int mValue;
                float mQuality;
                int mUses;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            Data mData;
            std::string mScript;
        };

        struct Repair
        {
            struct Data
            {
                float mWeight;
                int mValue;
                float mQuality;
                int mUses;
            };

            std::string mId;
            std::string mName;
            std::string mModel;
            std::string mIcon;
            Data mData;
            std::string mScript;
        };

        struct Script
        {
            std::string mId;
            std::string mScriptText;
        };

        struct Static
        {
            std::string mId;
            std::string mModel;
        };

        struct Sound
        {
            struct SOUNstruct
            {
                unsigned char mVolume;
                unsigned char mMinRange;
                unsigned char mMaxRange;
            };

            std::string mId;
            std::string mSound;
            SOUNstruct mData;
        };
    }
}

#endif

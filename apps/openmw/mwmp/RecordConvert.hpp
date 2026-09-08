#ifndef OPENMW_MWMP_RECORDCONVERT_H
#define OPENMW_MWMP_RECORDCONVERT_H

/*
    Conversion between the protocol's plain mirror records
    (components/openmw-mp/Base/records/Records.hpp) and the engine's ESM3
    records.

    GENERATED alongside the mirrors. This is the ONLY place refIds change form
    on the client; the dedicated server never includes this header and never
    sees ESM::RefId.

    Two rules the generator follows, both load-bearing:

      - fromWireCreate() is used for every incoming refId, never
        fromWireExisting(). TES3MP creates records on the fly, so an incoming
        id routinely names a record this client has not built yet; looking it
        up instead of interning it would silently yield an empty RefId.

      - effect / skill / attribute indices go through OpenMW's own
        indexToRefId() / refIdToIndex(), matching how components/esm3 converts
        them internally, including the -1 <-> EmptyRefId convention.
*/

#include <algorithm>
#include <iterator>

#include <components/esm3/loadacti.hpp>
#include <components/esm3/loadalch.hpp>
#include <components/esm3/loadappa.hpp>
#include <components/esm3/loadarmo.hpp>
#include <components/esm3/loadbody.hpp>
#include <components/esm3/loadbook.hpp>
#include <components/esm3/loadcell.hpp>
#include <components/esm3/loadclot.hpp>
#include <components/esm3/loadcont.hpp>
#include <components/esm3/loadcrea.hpp>
#include <components/esm3/loaddoor.hpp>
#include <components/esm3/loadench.hpp>
#include <components/esm3/loadgmst.hpp>
#include <components/esm3/loadingr.hpp>
#include <components/esm3/loadligh.hpp>
#include <components/esm3/loadlock.hpp>
#include <components/esm3/loadmisc.hpp>
#include <components/esm3/loadnpc.hpp>
#include <components/esm3/loadprob.hpp>
#include <components/esm3/loadrepa.hpp>
#include <components/esm3/loadscpt.hpp>
#include <components/esm3/loadsoun.hpp>
#include <components/esm3/loadspel.hpp>
#include <components/esm3/loadstat.hpp>
#include <components/esm3/loadweap.hpp>

#include <components/openmw-mp/Base/records/Records.hpp>

#include "RecordConvertCommon.hpp"
#include "RecordConvertPlayer.hpp"
#include "RefIdCompat.hpp"

namespace mwmp
{
    namespace RecordConvert
    {
        inline void toEngine(const records::Spell& from, ESM::Spell& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mData.mType = from.mData.mType;
            to.mData.mCost = from.mData.mCost;
            to.mData.mFlags = from.mData.mFlags;
            toEngine(from.mEffects, to.mEffects);
        }

        inline void fromEngine(const ESM::Spell& from, records::Spell& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mData.mType = from.mData.mType;
            to.mData.mCost = from.mData.mCost;
            to.mData.mFlags = from.mData.mFlags;
            fromEngine(from.mEffects, to.mEffects);
        }

        inline void toEngine(const records::Potion& from, ESM::Potion& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mFlags = from.mData.mAutoCalc; // renamed in 0.51, same ALDT slot
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
            toEngine(from.mEffects, to.mEffects);
        }

        inline void fromEngine(const ESM::Potion& from, records::Potion& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mAutoCalc = from.mData.mFlags; // renamed in 0.51, same ALDT slot
            to.mScript = RefIdCompat::toWire(from.mScript);
            fromEngine(from.mEffects, to.mEffects);
        }

        inline void toEngine(const records::Enchantment& from, ESM::Enchantment& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mData.mType = from.mData.mType;
            to.mData.mCost = from.mData.mCost;
            to.mData.mCharge = from.mData.mCharge;
            to.mData.mFlags = from.mData.mFlags;
            toEngine(from.mEffects, to.mEffects);
        }

        inline void fromEngine(const ESM::Enchantment& from, records::Enchantment& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mData.mType = from.mData.mType;
            to.mData.mCost = from.mData.mCost;
            to.mData.mCharge = from.mData.mCharge;
            to.mData.mFlags = from.mData.mFlags;
            fromEngine(from.mEffects, to.mEffects);
        }

        inline void toEngine(const records::Armor& from, ESM::Armor& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mArmor = from.mData.mArmor;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::fromWireCreate(from.mEnchant);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
            toEngine(from.mParts, to.mParts);
        }

        inline void fromEngine(const ESM::Armor& from, records::Armor& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mArmor = from.mData.mArmor;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::toWire(from.mEnchant);
            to.mScript = RefIdCompat::toWire(from.mScript);
            fromEngine(from.mParts, to.mParts);
        }

        inline void toEngine(const records::Book& from, ESM::Book& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mText = from.mText;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mIsScroll = from.mData.mIsScroll;
            to.mData.mSkillId = from.mData.mSkillId;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::fromWireCreate(from.mEnchant);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Book& from, records::Book& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mText = from.mText;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mIsScroll = from.mData.mIsScroll;
            to.mData.mSkillId = from.mData.mSkillId;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::toWire(from.mEnchant);
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Clothing& from, ESM::Clothing& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::fromWireCreate(from.mEnchant);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
            toEngine(from.mParts, to.mParts);
        }

        inline void fromEngine(const ESM::Clothing& from, records::Clothing& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::toWire(from.mEnchant);
            to.mScript = RefIdCompat::toWire(from.mScript);
            fromEngine(from.mParts, to.mParts);
        }

        inline void toEngine(const records::Miscellaneous& from, ESM::Miscellaneous& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mFlags = from.mData.mIsKey; // renamed in 0.51, same MCDT slot
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Miscellaneous& from, records::Miscellaneous& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mIsKey = from.mData.mFlags; // renamed in 0.51, same MCDT slot
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Weapon& from, ESM::Weapon& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mSpeed = from.mData.mSpeed;
            to.mData.mReach = from.mData.mReach;
            std::copy(std::begin(from.mData.mChop), std::end(from.mData.mChop), std::begin(to.mData.mChop));
            std::copy(std::begin(from.mData.mSlash), std::end(from.mData.mSlash), std::begin(to.mData.mSlash));
            std::copy(std::begin(from.mData.mThrust), std::end(from.mData.mThrust), std::begin(to.mData.mThrust));
            to.mData.mFlags = from.mData.mFlags;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::fromWireCreate(from.mEnchant);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Weapon& from, records::Weapon& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mSpeed = from.mData.mSpeed;
            to.mData.mReach = from.mData.mReach;
            std::copy(std::begin(from.mData.mChop), std::end(from.mData.mChop), std::begin(to.mData.mChop));
            std::copy(std::begin(from.mData.mSlash), std::end(from.mData.mSlash), std::begin(to.mData.mSlash));
            std::copy(std::begin(from.mData.mThrust), std::end(from.mData.mThrust), std::begin(to.mData.mThrust));
            to.mData.mFlags = from.mData.mFlags;
            to.mData.mEnchant = from.mData.mEnchant;
            to.mEnchant = RefIdCompat::toWire(from.mEnchant);
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Activator& from, ESM::Activator& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Activator& from, records::Activator& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Apparatus& from, ESM::Apparatus& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Apparatus& from, records::Apparatus& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mType = from.mData.mType;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::BodyPart& from, ESM::BodyPart& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mModel = from.mModel;
            to.mRace = RefIdCompat::fromWireCreate(from.mRace);
            to.mData.mType = from.mData.mType;
            to.mData.mPart = from.mData.mPart;
            to.mData.mVampire = from.mData.mVampire;
            to.mData.mFlags = from.mData.mFlags;
        }

        inline void fromEngine(const ESM::BodyPart& from, records::BodyPart& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mModel = from.mModel;
            to.mRace = RefIdCompat::toWire(from.mRace);
            to.mData.mType = from.mData.mType;
            to.mData.mPart = from.mData.mPart;
            to.mData.mVampire = from.mData.mVampire;
            to.mData.mFlags = from.mData.mFlags;
        }

        inline void toEngine(const records::Container& from, ESM::Container& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mWeight = from.mWeight;
            to.mFlags = from.mFlags;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
            toEngine(from.mInventory, to.mInventory);
        }

        inline void fromEngine(const ESM::Container& from, records::Container& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mWeight = from.mWeight;
            to.mFlags = from.mFlags;
            to.mScript = RefIdCompat::toWire(from.mScript);
            fromEngine(from.mInventory, to.mInventory);
        }

        inline void toEngine(const records::Creature& from, ESM::Creature& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mScale = from.mScale;
            to.mBloodType = from.mBloodType;
            to.mData.mType = from.mData.mType;
            to.mData.mLevel = from.mData.mLevel;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mMana = from.mData.mMana;
            to.mData.mFatigue = from.mData.mFatigue;
            to.mData.mSoul = from.mData.mSoul;
            std::copy(std::begin(from.mData.mAttack), std::end(from.mData.mAttack), std::begin(to.mData.mAttack));
            toEngine(from.mAiData, to.mAiData);
            to.mFlags = from.mFlags;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
            toEngine(from.mInventory, to.mInventory);
        }

        inline void fromEngine(const ESM::Creature& from, records::Creature& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mScale = from.mScale;
            to.mBloodType = from.mBloodType;
            to.mData.mType = from.mData.mType;
            to.mData.mLevel = from.mData.mLevel;
            to.mData.mHealth = from.mData.mHealth;
            to.mData.mMana = from.mData.mMana;
            to.mData.mFatigue = from.mData.mFatigue;
            to.mData.mSoul = from.mData.mSoul;
            std::copy(std::begin(from.mData.mAttack), std::end(from.mData.mAttack), std::begin(to.mData.mAttack));
            fromEngine(from.mAiData, to.mAiData);
            to.mFlags = from.mFlags;
            to.mScript = RefIdCompat::toWire(from.mScript);
            fromEngine(from.mInventory, to.mInventory);
        }

        inline void toEngine(const records::Door& from, ESM::Door& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mOpenSound = RefIdCompat::fromWireCreate(from.mOpenSound);
            to.mCloseSound = RefIdCompat::fromWireCreate(from.mCloseSound);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Door& from, records::Door& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mOpenSound = RefIdCompat::toWire(from.mOpenSound);
            to.mCloseSound = RefIdCompat::toWire(from.mCloseSound);
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        /*
            GameSetting carries only its id here. ESM::GameSetting::mValue is an
            ESM::Variant which never crosses the wire -- PacketRecordDynamic
            sends mwmp::GameSettingRecord::variable instead, and the caller
            builds the Variant from it after this conversion.
        */
        inline void toEngine(const records::GameSetting& from, ESM::GameSetting& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
        }

        inline void fromEngine(const ESM::GameSetting& from, records::GameSetting& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
        }

        inline void toEngine(const records::Ingredient& from, ESM::Ingredient& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            for (int i = 0; i < 4; ++i)
                to.mData.mEffectID[i] = ESM::MagicEffect::indexToRefId(from.mData.mEffectID[i]);
            for (int i = 0; i < 4; ++i)
                to.mData.mAttributes[i] = ESM::Attribute::indexToRefId(from.mData.mAttributes[i]);
            for (int i = 0; i < 4; ++i)
                to.mData.mSkills[i] = ESM::Skill::indexToRefId(from.mData.mSkills[i]);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Ingredient& from, records::Ingredient& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            for (int i = 0; i < 4; ++i)
                to.mData.mEffectID[i] = ESM::MagicEffect::refIdToIndex(from.mData.mEffectID[i]);
            for (int i = 0; i < 4; ++i)
                to.mData.mAttributes[i] = ESM::Attribute::refIdToIndex(from.mData.mAttributes[i]);
            for (int i = 0; i < 4; ++i)
                to.mData.mSkills[i] = ESM::Skill::refIdToIndex(from.mData.mSkills[i]);
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Light& from, ESM::Light& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mSound = RefIdCompat::fromWireCreate(from.mSound);
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mTime = from.mData.mTime;
            to.mData.mRadius = from.mData.mRadius;
            to.mData.mColor = from.mData.mColor;
            to.mData.mFlags = from.mData.mFlags;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Light& from, records::Light& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mSound = RefIdCompat::toWire(from.mSound);
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mTime = from.mData.mTime;
            to.mData.mRadius = from.mData.mRadius;
            to.mData.mColor = from.mData.mColor;
            to.mData.mFlags = from.mData.mFlags;
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Lockpick& from, ESM::Lockpick& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mData.mUses = from.mData.mUses;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Lockpick& from, records::Lockpick& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mData.mUses = from.mData.mUses;
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::NPC& from, ESM::NPC& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mFlags = from.mFlags;
            to.mRace = RefIdCompat::fromWireCreate(from.mRace);
            to.mModel = from.mModel;
            to.mHair = RefIdCompat::fromWireCreate(from.mHair);
            to.mHead = RefIdCompat::fromWireCreate(from.mHead);
            to.mClass = RefIdCompat::fromWireCreate(from.mClass);
            to.mFaction = RefIdCompat::fromWireCreate(from.mFaction);
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
            toEngine(from.mNpdt, to.mNpdt);
            toEngine(from.mAiData, to.mAiData);
            to.mNpdtType = from.mNpdtType;
            toEngine(from.mInventory, to.mInventory);
        }

        inline void fromEngine(const ESM::NPC& from, records::NPC& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mFlags = from.mFlags;
            to.mRace = RefIdCompat::toWire(from.mRace);
            to.mModel = from.mModel;
            to.mHair = RefIdCompat::toWire(from.mHair);
            to.mHead = RefIdCompat::toWire(from.mHead);
            to.mClass = RefIdCompat::toWire(from.mClass);
            to.mFaction = RefIdCompat::toWire(from.mFaction);
            to.mScript = RefIdCompat::toWire(from.mScript);
            fromEngine(from.mNpdt, to.mNpdt);
            fromEngine(from.mAiData, to.mAiData);
            to.mNpdtType = from.mNpdtType;
            fromEngine(from.mInventory, to.mInventory);
        }

        inline void toEngine(const records::Probe& from, ESM::Probe& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mData.mUses = from.mData.mUses;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Probe& from, records::Probe& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mData.mUses = from.mData.mUses;
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Repair& from, ESM::Repair& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mData.mUses = from.mData.mUses;
            to.mScript = RefIdCompat::fromWireCreate(from.mScript);
        }

        inline void fromEngine(const ESM::Repair& from, records::Repair& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mModel = from.mModel;
            to.mIcon = from.mIcon;
            to.mData.mWeight = from.mData.mWeight;
            to.mData.mValue = from.mData.mValue;
            to.mData.mQuality = from.mData.mQuality;
            to.mData.mUses = from.mData.mUses;
            to.mScript = RefIdCompat::toWire(from.mScript);
        }

        inline void toEngine(const records::Script& from, ESM::Script& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mScriptText = from.mScriptText;
        }

        inline void fromEngine(const ESM::Script& from, records::Script& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mScriptText = from.mScriptText;
        }

        inline void toEngine(const records::Static& from, ESM::Static& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mModel = from.mModel;
        }

        inline void fromEngine(const ESM::Static& from, records::Static& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mModel = from.mModel;
        }

        inline void toEngine(const records::Sound& from, ESM::Sound& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mSound = from.mSound;
            to.mData.mVolume = from.mData.mVolume;
            to.mData.mMinRange = from.mData.mMinRange;
            to.mData.mMaxRange = from.mData.mMaxRange;
        }

        inline void fromEngine(const ESM::Sound& from, records::Sound& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mSound = from.mSound;
            to.mData.mVolume = from.mData.mVolume;
            to.mData.mMinRange = from.mData.mMinRange;
            to.mData.mMaxRange = from.mData.mMaxRange;
        }
    }
}

#endif

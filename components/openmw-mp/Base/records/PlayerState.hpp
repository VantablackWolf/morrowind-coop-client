#ifndef OPENMW_MP_RECORDS_PLAYERSTATE_H
#define OPENMW_MP_RECORDS_PLAYERSTATE_H

/*
    Mirrors for the player/actor state structs TES3MP synchronises, completing
    the dedicated server's decoupling from components/esm.

    Same rule as records/Records.hpp: field types come from OpenMW 0.47, which
    is what the current wire format encodes. Engine churn is absorbed in
    apps/openmw/mwmp/RecordConvert*.hpp, never here.

    Mirroring Position is worth calling out separately: ESM::Position is
    layout-identical in 0.51, but it now pulls in <osg/Vec3f> for its
    asVec3() helpers. Mirroring it keeps OpenSceneGraph out of the dedicated
    server's include graph entirely.
*/

#include <cstdint>
#include <string>
#include <vector>

namespace mwmp
{
    namespace records
    {
        // Wire-critical counts. The engine equivalents are pinned by
        // static assertions in apps/openmw/mwmp/EngineSurfaceCheck.hpp; if
        // OpenMW ever changes one, that build fails rather than this
        // silently truncating.
        enum : int
        {
            sAttributeCount = 8,
            sSkillCount = 27,
            sDynamicCount = 3,
        };

        // Mirrors ESM::Position. Layout unchanged since 0.47.
        struct Position
        {
            float pos[3];
            float rot[3];
        };

        // Mirrors the serialized subset of ESM::Cell.
        struct Cell
        {
            struct DATAstruct
            {
                int mFlags;
                int mX;
                int mY;
            };

            DATAstruct mData;
            std::string mName;
            std::string mRegion; // ESM::RefId in 0.51

            /*
                Start of tes3mp addition

                The dedicated server subclasses this and uses these accessors, which used to
                come from ESM::Cell. Reimplemented here so the mirror stays self-contained --
                they are pure reads of mData and involve no engine types.
            */
            enum Flags
            {
                Interior = 0x01,
            };

            bool isExterior() const { return !(mData.mFlags & Interior); }
            int getGridX() const { return mData.mX; }
            int getGridY() const { return mData.mY; }

            std::string getShortDescription() const
            {
                if (!isExterior())
                    return mName;
                return std::to_string(mData.mX) + ", " + std::to_string(mData.mY);
            }

            void blank()
            {
                mName.clear();
                mRegion.clear();
                mData.mFlags = 0;
                mData.mX = 0;
                mData.mY = 0;
            }
            /*
                End of tes3mp addition
            */
        };

        // Mirrors ESM::StatState<T>.
        template <class T>
        struct StatState
        {
            T mBase;
            T mMod;
            T mCurrent;
            float mDamage;
            float mProgress;
        };

        // Mirrors the serialized subset of ESM::Class.
        struct Class
        {
            struct CLDTstruct
            {
                int mAttribute[2];
                int mSpecialization;
                int mSkills[5][2];
            };

            CLDTstruct mData;
            std::string mId; // ESM::RefId in 0.51
            std::string mName;
            std::string mDescription;

            void blank()
            {
                mId.clear();
                mName.clear();
                mDescription.clear();
                mData = {};
            }
        };

        // Mirrors the serialized subset of ESM::CreatureStats.
        struct CreatureStats
        {
            StatState<float> mAttributes[sAttributeCount];
            StatState<float> mDynamic[sDynamicCount];
            bool mDead;
            int mLevel;

            void blank()
            {
                *this = CreatureStats{};
            }
        };

        /*
            Mirrors ESM::ActiveEffect.

            0.51 reworked this one substantially:
              int mEffectId -> ESM::RefId
              int mArg      -> std::variant<RefId, RefNum>  (skill, attribute
                               OR summoned actor -- a case 0.47 had no notion of)
            and it gained mMinMagnitude / mMaxMagnitude / mFlags, none of which
            cross the wire.

            The wire keeps the 0.47 integers. Disambiguating mArg on the way in
            needs the effect id, and RecordConvertPlayer replicates OpenMW's own
            classification for that.
        */
        struct ActiveEffect
        {
            int mEffectId;
            float mMagnitude;
            int mArg; // skill or attribute index, -1 if none
            float mDuration;
            float mTimeLeft;
            int mEffectIndex;
        };

        /*
            Mirrors the serialized subset of ESM::ActiveSpells::ActiveSpellParams.
            Only mEffects and mDisplayName cross the wire; mCasterActorId is
            engine-local and 0.51 added further bookkeeping fields that TES3MP
            has never sent.
        */
        struct ActiveSpellParams
        {
            std::vector<ActiveEffect> mEffects;
            std::string mDisplayName;
        };

        // Mirrors the serialized subset of ESM::NpcStats.
        struct NpcStats
        {
            StatState<float> mSkills[sSkillCount];
            int mSkillIncrease[sAttributeCount];
            int mBounty;
            int mReputation;
            int mLevelProgress;

            void blank()
            {
                *this = NpcStats{};
            }
        };
    }
}

#endif

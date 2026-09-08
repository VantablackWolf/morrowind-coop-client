#ifndef OPENMW_MWMP_RECORDCONVERTPLAYER_H
#define OPENMW_MWMP_RECORDCONVERTPLAYER_H

/*
    Conversions for components/openmw-mp/Base/records/PlayerState.hpp.

    Client-side only. Most of the churn here is C arrays becoming std::array
    in 0.51 -- same lengths, same meaning, so the copies are element-wise
    rather than assignments.
*/

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <variant>

#include <components/esm/attr.hpp>
#include <components/esm/position.hpp>
#include <components/esm3/activespells.hpp>
#include <components/esm3/refnum.hpp>
#include <components/esm3/creaturestats.hpp>
#include <components/esm3/loadcell.hpp>
#include <components/esm3/loadclas.hpp>
#include <components/esm3/loadmgef.hpp>
#include <components/esm3/loadskil.hpp>
#include <components/esm3/npcstats.hpp>
#include <components/esm3/statstate.hpp>

#include <components/openmw-mp/Base/records/PlayerState.hpp>

#include "../mwworld/cell.hpp"

#include "RefIdCompat.hpp"

namespace mwmp
{
    namespace RecordConvert
    {
        inline void toEngine(const records::Position& from, ESM::Position& to)
        {
            std::copy(std::begin(from.pos), std::end(from.pos), std::begin(to.pos));
            std::copy(std::begin(from.rot), std::end(from.rot), std::begin(to.rot));
        }

        inline void fromEngine(const ESM::Position& from, records::Position& to)
        {
            std::copy(std::begin(from.pos), std::end(from.pos), std::begin(to.pos));
            std::copy(std::begin(from.rot), std::end(from.rot), std::begin(to.rot));
        }

        inline void toEngine(const records::Cell& from, ESM::Cell& to)
        {
            to.mData.mFlags = from.mData.mFlags;
            to.mData.mX = from.mData.mX;
            to.mData.mY = from.mData.mY;
            to.mName = from.mName;
            to.mRegion = RefIdCompat::fromWireCreate(from.mRegion);
        }

        inline void fromEngine(const ESM::Cell& from, records::Cell& to)
        {
            to.mData.mFlags = from.mData.mFlags;
            to.mData.mX = from.mData.mX;
            to.mData.mY = from.mData.mY;
            to.mName = from.mName;
            to.mRegion = RefIdCompat::toWire(from.mRegion);
        }

        /*
            0.51 introduced MWWorld::Cell, a unified view over ESM3 and ESM4 cells, and
            most engine code now hands that out rather than an ESM::Cell. The wire format
            only ever carried the ESM3 subset -- name, region, grid position, interior
            flag -- so it converts directly, and ESM4 cells simply have no ESM3 region.

            The interior flag is rebuilt rather than copied: MWWorld::Cell exposes
            isExterior() but not the raw ESM::Cell::DATAstruct flags, and Interior is the
            only flag bit tes3mp puts on the wire.
        */
        inline void fromEngine(const MWWorld::Cell& from, records::Cell& to)
        {
            to.mData.mFlags = from.isExterior() ? 0 : records::Cell::Interior;
            to.mData.mX = from.getGridX();
            to.mData.mY = from.getGridY();
            to.mName = std::string(from.getNameId());
            to.mRegion = RefIdCompat::toWire(from.getRegion());
        }

        /*
            By-value forms.

            Ported call sites are overwhelmingly of the shape "mirror = engineValue", and
            an out-parameter turns each of those into two statements plus a temporary. The
            overload set is resolved on the engine type, so a mismatch is still a compile
            error naming both types -- which is the property that matters here, because the
            mirrors deliberately reuse the engine's member names and a wrong pairing would
            otherwise assign happily.
        */
        inline records::Position toMirror(const ESM::Position& from)
        {
            records::Position to;
            fromEngine(from, to);
            return to;
        }

        inline records::Cell toMirror(const ESM::Cell& from)
        {
            records::Cell to;
            fromEngine(from, to);
            return to;
        }

        inline records::Cell toMirror(const MWWorld::Cell& from)
        {
            records::Cell to;
            fromEngine(from, to);
            return to;
        }

        template <class T>
        inline void toEngine(const records::StatState<T>& from, ESM::StatState<T>& to)
        {
            to.mBase = from.mBase;
            to.mMod = from.mMod;
            to.mCurrent = from.mCurrent;
            to.mDamage = from.mDamage;
            to.mProgress = from.mProgress;
        }

        template <class T>
        inline void fromEngine(const ESM::StatState<T>& from, records::StatState<T>& to)
        {
            to.mBase = from.mBase;
            to.mMod = from.mMod;
            to.mCurrent = from.mCurrent;
            to.mDamage = from.mDamage;
            to.mProgress = from.mProgress;
        }

        inline void toEngine(const records::Class& from, ESM::Class& to)
        {
            to.mId = RefIdCompat::fromWireCreate(from.mId);
            to.mName = from.mName;
            to.mDescription = from.mDescription;
            to.mData.mSpecialization = from.mData.mSpecialization;
            to.mData.mIsPlayable = from.mData.mIsPlayable;
            std::copy(std::begin(from.mData.mAttribute), std::end(from.mData.mAttribute),
                std::begin(to.mData.mAttribute));
            // 0.47 int[5][2] -> 0.51 std::array<std::array<int32_t, 2>, 5>
            for (std::size_t i = 0; i < to.mData.mSkills.size(); ++i)
            {
                to.mData.mSkills[i][0] = from.mData.mSkills[i][0];
                to.mData.mSkills[i][1] = from.mData.mSkills[i][1];
            }
        }

        inline void fromEngine(const ESM::Class& from, records::Class& to)
        {
            to.mId = RefIdCompat::toWire(from.mId);
            to.mName = from.mName;
            to.mDescription = from.mDescription;
            to.mData.mSpecialization = from.mData.mSpecialization;
            to.mData.mIsPlayable = from.mData.mIsPlayable;
            std::copy(std::begin(from.mData.mAttribute), std::end(from.mData.mAttribute),
                std::begin(to.mData.mAttribute));
            for (std::size_t i = 0; i < from.mData.mSkills.size(); ++i)
            {
                to.mData.mSkills[i][0] = from.mData.mSkills[i][0];
                to.mData.mSkills[i][1] = from.mData.mSkills[i][1];
            }
        }

        namespace detail
        {
            /*
                Replicated from the anonymous namespace in
                components/esm3/activespells.cpp, which faces exactly this
                problem when loading pre-RefId saves. Kept identical on purpose:
                if OpenMW's classification changes, ours must change with it.
            */
            inline bool isSummon(ESM::RefId id)
            {
                static const std::array effects{ ESM::MagicEffect::SummonScamp, ESM::MagicEffect::SummonClannfear,
                    ESM::MagicEffect::SummonDaedroth, ESM::MagicEffect::SummonDremora,
                    ESM::MagicEffect::SummonAncestralGhost, ESM::MagicEffect::SummonSkeletalMinion,
                    ESM::MagicEffect::SummonBonewalker, ESM::MagicEffect::SummonGreaterBonewalker,
                    ESM::MagicEffect::SummonBonelord, ESM::MagicEffect::SummonWingedTwilight,
                    ESM::MagicEffect::SummonHunger, ESM::MagicEffect::SummonGoldenSaint,
                    ESM::MagicEffect::SummonFlameAtronach, ESM::MagicEffect::SummonFrostAtronach,
                    ESM::MagicEffect::SummonStormAtronach, ESM::MagicEffect::SummonCenturionSphere,
                    ESM::MagicEffect::SummonFabricant, ESM::MagicEffect::SummonWolf, ESM::MagicEffect::SummonBear,
                    ESM::MagicEffect::SummonBonewolf, ESM::MagicEffect::SummonCreature04,
                    ESM::MagicEffect::SummonCreature05 };
                return std::find(effects.begin(), effects.end(), id) != effects.end();
            }

            inline bool affectsAttribute(ESM::RefId id)
            {
                static const std::array effects{ ESM::MagicEffect::DrainAttribute, ESM::MagicEffect::DamageAttribute,
                    ESM::MagicEffect::RestoreAttribute, ESM::MagicEffect::FortifyAttribute,
                    ESM::MagicEffect::AbsorbAttribute };
                return std::find(effects.begin(), effects.end(), id) != effects.end();
            }

            inline bool affectsSkill(ESM::RefId id)
            {
                static const std::array effects{ ESM::MagicEffect::DrainSkill, ESM::MagicEffect::DamageSkill,
                    ESM::MagicEffect::RestoreSkill, ESM::MagicEffect::FortifySkill, ESM::MagicEffect::AbsorbSkill };
                return std::find(effects.begin(), effects.end(), id) != effects.end();
            }
        }

        inline void toEngine(const records::ActiveEffect& from, ESM::ActiveEffect& to)
        {
            to.mEffectId = ESM::MagicEffect::indexToRefId(from.mEffectId);
            to.mMagnitude = from.mMagnitude;
            // 0.51 splits magnitude into a min/max range. The wire carries a
            // single value, so collapse the range onto it rather than leaving
            // the bounds at zero, which would read as "no effect".
            to.mMinMagnitude = from.mMagnitude;
            to.mMaxMagnitude = from.mMagnitude;
            to.mDuration = from.mDuration;
            to.mTimeLeft = from.mTimeLeft;
            to.mEffectIndex = from.mEffectIndex;

            if (from.mArg >= 0)
            {
                if (detail::isSummon(to.mEffectId))
                    to.mArg = ESM::RefNum{ .mIndex = static_cast<std::uint32_t>(from.mArg), .mContentFile = -1 };
                else if (detail::affectsAttribute(to.mEffectId))
                    to.mArg = ESM::Attribute::indexToRefId(from.mArg);
                else if (detail::affectsSkill(to.mEffectId))
                    to.mArg = ESM::Skill::indexToRefId(from.mArg);
            }
        }

        inline void fromEngine(const ESM::ActiveEffect& from, records::ActiveEffect& to)
        {
            to.mEffectId = ESM::MagicEffect::refIdToIndex(from.mEffectId);
            to.mMagnitude = from.mMagnitude;
            to.mDuration = from.mDuration;
            to.mTimeLeft = from.mTimeLeft;
            to.mEffectIndex = from.mEffectIndex;

            to.mArg = -1;
            if (const ESM::RefNum* actor = std::get_if<ESM::RefNum>(&from.mArg))
            {
                // Summons have no representation in the 0.47 wire format beyond
                // the bare index; the content-file half is dropped.
                to.mArg = static_cast<int>(actor->mIndex);
            }
            else if (const ESM::RefId* id = std::get_if<ESM::RefId>(&from.mArg))
            {
                // Skill and attribute index spaces overlap, so try both rather
                // than guessing from the effect.
                int index = ESM::Skill::refIdToIndex(*id);
                if (index < 0)
                    index = ESM::Attribute::refIdToIndex(*id);
                to.mArg = index;
            }
        }

        /*
            Effect vectors.

            The active-spell packets carry a whole effect list at a time, and both sides
            spell it as a plain vector, so converting element-wise here keeps every call
            site to one line.
        */
        inline void fromEngine(const std::vector<ESM::ActiveEffect>& from, std::vector<records::ActiveEffect>& to)
        {
            to.clear();
            to.reserve(from.size());
            for (const auto& effect : from)
            {
                records::ActiveEffect converted;
                fromEngine(effect, converted);
                to.push_back(converted);
            }
        }

        inline void toEngine(const std::vector<records::ActiveEffect>& from, std::vector<ESM::ActiveEffect>& to)
        {
            to.clear();
            to.reserve(from.size());
            for (const auto& effect : from)
            {
                ESM::ActiveEffect converted;
                toEngine(effect, converted);
                to.push_back(converted);
            }
        }

        inline void toEngine(const records::CreatureStats& from, ESM::CreatureStats& to)
        {
            for (std::size_t i = 0; i < to.mAttributes.size(); ++i)
                toEngine(from.mAttributes[i], to.mAttributes[i]);
            for (std::size_t i = 0; i < to.mDynamic.size(); ++i)
                toEngine(from.mDynamic[i], to.mDynamic[i]);
            to.mDead = from.mDead;
            to.mLevel = from.mLevel;
        }

        inline void fromEngine(const ESM::CreatureStats& from, records::CreatureStats& to)
        {
            for (std::size_t i = 0; i < from.mAttributes.size(); ++i)
                fromEngine(from.mAttributes[i], to.mAttributes[i]);
            for (std::size_t i = 0; i < from.mDynamic.size(); ++i)
                fromEngine(from.mDynamic[i], to.mDynamic[i]);
            to.mDead = from.mDead;
            to.mLevel = from.mLevel;
        }

        inline void toEngine(const records::NpcStats& from, ESM::NpcStats& to)
        {
            for (std::size_t i = 0; i < to.mSkills.size(); ++i)
                toEngine(from.mSkills[i], to.mSkills[i]);
            std::copy(std::begin(from.mSkillIncrease), std::end(from.mSkillIncrease),
                std::begin(to.mSkillIncrease));
            to.mBounty = from.mBounty;
            to.mReputation = from.mReputation;
            to.mLevelProgress = from.mLevelProgress;
        }

        inline void fromEngine(const ESM::NpcStats& from, records::NpcStats& to)
        {
            for (std::size_t i = 0; i < from.mSkills.size(); ++i)
                fromEngine(from.mSkills[i], to.mSkills[i]);
            std::copy(std::begin(from.mSkillIncrease), std::end(from.mSkillIncrease),
                std::begin(to.mSkillIncrease));
            to.mBounty = from.mBounty;
            to.mReputation = from.mReputation;
            to.mLevelProgress = from.mLevelProgress;
        }
    }
}

#endif

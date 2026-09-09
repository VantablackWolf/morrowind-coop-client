#ifndef GAME_MWMECHANICS_ACTIVESPELLS_H
#define GAME_MWMECHANICS_ACTIVESPELLS_H

#include <functional>
#include <list>
#include <queue>
#include <string>
#include <variant>
#include <vector>

#include <components/esm3/activespells.hpp>

#include "../mwworld/ptr.hpp"
#include "../mwworld/timestamp.hpp"

#include "spellcasting.hpp"

namespace ESM
{
    struct Enchantment;
    struct Spell;
}

namespace MWMechanics
{
    /// \brief Lasting spell effects
    ///
    /// \note The name of this class is slightly misleading, since it also handles lasting potion
    /// effects.
    class ActiveSpells
    {
    public:
        using ActiveEffect = ESM::ActiveEffect;
        class ActiveSpellParams
        {
            ESM::RefId mActiveSpellId;
            ESM::RefId mSourceSpellId;
            std::vector<ActiveEffect> mEffects;
            std::string mDisplayName;
            ESM::RefNum mCaster;
            ESM::RefNum mItem;
            ESM::ActiveSpells::Flags mFlags;
            int mWorsenings;
            MWWorld::TimeStamp mNextWorsening;
            MWWorld::Ptr mSource;

            ActiveSpellParams(const ESM::ActiveSpells::ActiveSpellParams& params);

            ActiveSpellParams(const ESM::Spell* spell, const MWWorld::Ptr& actor, bool ignoreResistances = false);

            ActiveSpellParams(
                const MWWorld::ConstPtr& item, const ESM::Enchantment* enchantment, const MWWorld::Ptr& actor);

            ActiveSpellParams(const ActiveSpellParams& params, const MWWorld::Ptr& actor);

            ESM::ActiveSpells::ActiveSpellParams toEsm() const;

            /*
                Start of tes3mp addition

                Track when this spell was applied.

                0.8.1 tracked an integer caster actorId here. 0.51 identifies casters by
                ESM::RefNum in mCaster, which supersedes it, so that field is gone.

                The timestamp has no 0.51 equivalent and is still needed: it is what the
                ID_*_SPELLS_ACTIVE packets carry, and what lets a removal name one of
                several stacked copies of the same spell. It is deliberately NOT written
                to ESM by toEsm() -- it is multiplayer session state, and the server
                resends active spells on connect.
            */
            MWWorld::TimeStamp mTimeStamp;

            /*
                Whether gaining this spell should be reported to the server.

                0.8.1 passed this as an argument to addSpell(), because addSpell() both
                inserted the spell and sent the packet. 0.51 splits those in two: addSpell()
                only queues, and the spell becomes active later, in addToSpells() during
                update(). The flag therefore has to travel with the params instead of with
                the call, or a spell that arrived from another client would be echoed
                straight back to the server a frame after it was applied.
            */
            bool mSendPacket = true;
            /*
                End of tes3mp addition
            */
            friend class ActiveSpells;

        public:
            ActiveSpellParams(
                const MWWorld::Ptr& caster, const ESM::RefId& id, std::string_view sourceName, ESM::RefNum item);

            ESM::RefId getActiveSpellId() const { return mActiveSpellId; }
            void setActiveSpellId(ESM::RefId id) { mActiveSpellId = id; }

            const ESM::RefId& getSourceSpellId() const { return mSourceSpellId; }

            /*
                Start of tes3mp addition

                Expose the timestamp recorded above
            */
            MWWorld::TimeStamp getTimeStamp() const { return mTimeStamp; }
            void setTimeStamp(MWWorld::TimeStamp timestamp) { mTimeStamp = timestamp; }

            bool getSendPacket() const { return mSendPacket; }
            void setSendPacket(bool sendPacket) { mSendPacket = sendPacket; }
            /*
                End of tes3mp addition
            */
            const std::vector<ActiveEffect>& getEffects() const { return mEffects; }
            std::vector<ActiveEffect>& getEffects() { return mEffects; }

            ESM::RefNum getCaster() const { return mCaster; }

            int getWorsenings() const { return mWorsenings; }

            const std::string& getDisplayName() const { return mDisplayName; }

            ESM::RefNum getItem() const { return mItem; }
            ESM::RefId getEnchantment() const;

            /*
                Start of tes3mp addition

                Allow the purging of an effect for a specific arg (attribute or skill)
            */
            /*
                Start of tes3mp change (major)

                0.51 provides purgeEffect(ptr, effectId, effectArg) with RefId arguments,
                which replaces this. Callers were migrated to it.
            */
            // void purgeEffectByArg(short effectId, int effectArg);
            /*
                End of tes3mp change (major)
            */
            /*
                End of tes3mp addition
            */
            const ESM::Spell* getSpell() const;
            bool hasFlag(ESM::ActiveSpells::Flags flags) const;
            void setFlag(ESM::ActiveSpells::Flags flags);

            // Increments worsenings count and sets the next timestamp
            void worsen();

            bool shouldWorsen() const;

            void resetWorsenings();
        };

        typedef std::list<ActiveSpellParams> Collection;
        typedef Collection::const_iterator TIterator;

        void readState(const ESM::ActiveSpells& state);
        void writeState(ESM::ActiveSpells& state) const;

        TIterator begin() const;

        TIterator end() const;

        TIterator getActiveSpellById(const ESM::RefId& id);

        void update(const MWWorld::Ptr& ptr, float duration);

    private:
        using ParamsPredicate = std::function<bool(const ActiveSpellParams&)>;
        using EffectPredicate = std::function<bool(const ActiveSpellParams&, const ESM::ActiveEffect&)>;
        using Predicate = std::variant<ParamsPredicate, EffectPredicate>;

        struct IterationGuard
        {
            ActiveSpells& mActiveSpells;

            IterationGuard(ActiveSpells& spells);
            ~IterationGuard();
        };
        struct UpdateContext;

        std::list<ActiveSpellParams> mSpells;
        std::vector<ActiveSpellParams> mQueue;
        std::queue<Predicate> mPurges;
        bool mIterating;

        void addToSpells(const MWWorld::Ptr& ptr, const ActiveSpellParams& spell, UpdateContext& context);

        bool applyPurges(const MWWorld::Ptr& ptr, std::list<ActiveSpellParams>::iterator* currentSpell = nullptr,
            std::vector<ActiveEffect>::iterator* currentEffect = nullptr);

        bool updateActiveSpell(
            const MWWorld::Ptr& ptr, float duration, Collection::iterator& spellIt, UpdateContext& context);

        /*
            Start of tes3mp change (minor)

            Optionally report the id given to the spell that was added, so addToSpells() can
            find it again after it has been applied and read its rolled magnitude.
        */
        bool initParams(const MWWorld::Ptr& ptr, const ActiveSpellParams& params, UpdateContext& context,
            ESM::RefId* addedId = nullptr);
        /*
            End of tes3mp change (minor)
        */

    public:
        ActiveSpells();

        /// Add lasting effects
        ///
        /// \brief addSpell
        /// \param id ID for stacking purposes.
        ///
        void addSpell(const ActiveSpellParams& params);

        /*
            Start of tes3mp addition

            Add a separate addSpell() with a timestamp argument, so a spell arriving from
            another client keeps the timing it had there, and so relaying it straight back
            to the server can be suppressed.

            0.8.1 took (id, stack, effects, displayName, casterActorId, timestamp,
            sendPacket) against a map keyed by spell id. 0.51's ActiveSpells is a queue of
            ActiveSpellParams in which every instance already carries its own
            mActiveSpellId, so stacking needs no flag and the caster is an ESM::RefNum --
            the arguments that are gone are gone because 0.51 models them better, not
            because the feature was dropped.
        */
        void addSpell(const ActiveSpellParams& params, MWWorld::TimeStamp timestamp, bool sendPacket = true);
        /*
            End of tes3mp addition
        */
        /*
            Start of tes3mp addition

            Remove the spell with a certain ID and a certain timestamp, useful
            when there are stacked spells with the same ID

            Returns a boolean that indicates whether the corresponding spell was found
        */
        bool removeSpellByTimestamp(const MWWorld::Ptr& ptr, const ESM::RefId& id, MWWorld::TimeStamp timestamp);
        /*
            End of tes3mp addition
        */
        /*
            Start of tes3mp addition

            Make it easy to get an effect's duration
        */
        float getEffectDuration(const ESM::RefId& effectId, const ESM::RefId& sourceId) const;
        /*
            End of tes3mp addition
        */

        /// Force resistances
        void addSpell(const ESM::Spell* spell, const MWWorld::Ptr& actor, bool ignoreResistances = true);

        /// Removes the active effects from this spell/potion/.. with \a id
        void removeEffectsBySourceSpellId(const MWWorld::Ptr& ptr, const ESM::RefId& id);
        /// Removes the active effects of a specific active spell
        void removeEffectsByActiveSpellId(const MWWorld::Ptr& ptr, const ESM::RefId& id);

        /// Remove all active effects with this effect id
        void purgeEffect(const MWWorld::Ptr& ptr, ESM::RefId effectId, ESM::RefId effectArg = {});

        void purge(EffectPredicate predicate, const MWWorld::Ptr& ptr);
        void purge(ParamsPredicate predicate, const MWWorld::Ptr& ptr);

        /// Remove all effects that were cast by \a actor
        void purge(const MWWorld::Ptr& ptr, ESM::RefNum actor);

        /// Remove all spells
        void clear(const MWWorld::Ptr& ptr);

        /// True if a spell associated with this id is active
        /// \note For enchantments, this is the id of the enchanted item, not the enchantment itself
        bool isSpellActive(const ESM::RefId& id) const;

        /// True if the enchantment is active
        bool isEnchantmentActive(const ESM::RefId& id) const;

        void skipWorsenings(double hours);

        void unloadActor(const MWWorld::Ptr& ptr);
    };
}

#endif

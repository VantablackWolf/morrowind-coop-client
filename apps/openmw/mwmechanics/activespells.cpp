#include "activespells.hpp"

#include <optional>

#include <components/debug/debuglog.hpp>

#include <components/misc/resourcehelpers.hpp>

#include <components/misc/strings/algorithm.hpp>

#include <components/esm/generatedrefid.hpp>
#include <components/esm3/actoridconverter.hpp>
#include <components/esm3/loadench.hpp>
#include <components/esm3/loadmgef.hpp>
#include <components/esm3/loadstat.hpp>

#include <components/settings/values.hpp>

#include "actorutil.hpp"
#include "creaturestats.hpp"
#include "spellcasting.hpp"
#include "spelleffects.hpp"

/*
    Start of tes3mp addition

    Include additional headers for multiplayer purposes
*/
#include "../mwmechanics/actorutil.hpp"
#include "../mwmechanics/creaturestats.hpp"
#include "../mwworld/class.hpp"
#include "../mwmp/Main.hpp"
#include "../mwmp/LocalPlayer.hpp"
#include "../mwmp/CellController.hpp"
#include "../mwmp/MechanicsHelper.hpp"
#include "../mwmp/RefIdCompat.hpp"
/*
    End of tes3mp addition
*/

#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwrender/animation.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/manualref.hpp"
#include "../mwworld/worldmodel.hpp"

namespace
{
    bool merge(std::vector<ESM::ActiveEffect>& present, const std::vector<ESM::ActiveEffect>& queued)
    {
        // Can't merge if we already have an effect with the same effect index
        auto problem = std::find_if(queued.begin(), queued.end(), [&](const auto& qEffect) {
            return std::find_if(present.begin(), present.end(), [&](const auto& pEffect) {
                return pEffect.mEffectIndex == qEffect.mEffectIndex;
            }) != present.end();
        });
        if (problem != queued.end())
            return false;
        present.insert(present.end(), queued.begin(), queued.end());
        return true;
    }

    void addEffects(
        std::vector<ESM::ActiveEffect>& effects, const ESM::EffectList& list, bool ignoreResistances = false)
    {
        for (const auto& enam : list.mList)
        {
            if (enam.mData.mRange != ESM::RT_Self)
                continue;
            ESM::ActiveEffect effect;
            effect.mEffectId = enam.mData.mEffectID;
            effect.mArg = MWMechanics::EffectKey(enam.mData).mArg;
            effect.mMagnitude = 0.f;
            effect.mMinMagnitude = static_cast<float>(enam.mData.mMagnMin);
            effect.mMaxMagnitude = static_cast<float>(enam.mData.mMagnMax);
            effect.mEffectIndex = static_cast<int32_t>(enam.mIndex);
            effect.mFlags = ESM::ActiveEffect::Flag_None;
            if (ignoreResistances)
                effect.mFlags |= ESM::ActiveEffect::Flag_Ignore_Resistances;
            effect.mDuration = -1;
            effect.mTimeLeft = -1;
            effects.emplace_back(effect);
        }
    }
}

namespace MWMechanics
{
    struct ActiveSpells::UpdateContext
    {
        bool mUpdatedEnemy = false;
        bool mUpdatedHitOverlay = false;
        bool mUpdateSpellWindow = false;
        bool mPlayNonLooping = false;
        bool mEraseRemoved = false;
        bool mUpdate;

        UpdateContext(bool update)
            : mUpdate(update)
        {
        }
    };

    ActiveSpells::IterationGuard::IterationGuard(ActiveSpells& spells)
        : mActiveSpells(spells)
    {
        mActiveSpells.mIterating = true;
    }

    ActiveSpells::IterationGuard::~IterationGuard()
    {
        mActiveSpells.mIterating = false;
    }

    ActiveSpells::ActiveSpellParams::ActiveSpellParams(
        const MWWorld::Ptr& caster, const ESM::RefId& id, std::string_view sourceName, ESM::RefNum item)
        : mSourceSpellId(id)
        , mDisplayName(sourceName)
        , mItem(item)
        , mFlags()
        , mWorsenings(-1)
    {
        if (!caster.isEmpty() && caster.getClass().isActor())
            mCaster = caster.getCellRef().getRefNum();
    }

    ActiveSpells::ActiveSpellParams::ActiveSpellParams(
        const ESM::Spell* spell, const MWWorld::Ptr& actor, bool ignoreResistances)
        : mSourceSpellId(spell->mId)
        , mDisplayName(spell->mName)
        , mCaster(actor.getCellRef().getRefNum())
        , mFlags()
        , mWorsenings(-1)
    {
        assert(spell->mData.mType != ESM::Spell::ST_Spell && spell->mData.mType != ESM::Spell::ST_Power);
        setFlag(ESM::ActiveSpells::Flag_SpellStore);
        if (spell->mData.mType == ESM::Spell::ST_Ability)
            setFlag(ESM::ActiveSpells::Flag_AffectsBaseValues);
        addEffects(mEffects, spell->mEffects, ignoreResistances);
    }

    ActiveSpells::ActiveSpellParams::ActiveSpellParams(
        const MWWorld::ConstPtr& item, const ESM::Enchantment* enchantment, const MWWorld::Ptr& actor)
        : mSourceSpellId(item.getCellRef().getRefId())
        , mDisplayName(item.getClass().getName(item))
        , mCaster(actor.getCellRef().getRefNum())
        , mItem(item.getCellRef().getRefNum())
        , mFlags()
        , mWorsenings(-1)
    {
        assert(enchantment->mData.mType == ESM::Enchantment::ConstantEffect);
        addEffects(mEffects, enchantment->mEffects);
        setFlag(ESM::ActiveSpells::Flag_Equipment);
    }

    ActiveSpells::ActiveSpellParams::ActiveSpellParams(const ESM::ActiveSpells::ActiveSpellParams& params)
        : mActiveSpellId(params.mActiveSpellId)
        , mSourceSpellId(params.mSourceSpellId)
        , mEffects(params.mEffects)
        , mDisplayName(params.mDisplayName)
        , mCaster(params.mCaster)
        , mItem(params.mItem)
        , mFlags(params.mFlags)
        , mWorsenings(params.mWorsenings)
        , mNextWorsening({ params.mNextWorsening })
    {
    }

    ActiveSpells::ActiveSpellParams::ActiveSpellParams(const ActiveSpellParams& params, const MWWorld::Ptr& actor)
        : mSourceSpellId(params.mSourceSpellId)
        , mDisplayName(params.mDisplayName)
        , mCaster(actor.getCellRef().getRefNum())
        , mItem(params.mItem)
        , mFlags(params.mFlags)
        , mWorsenings(-1)
    {
    }

    ESM::ActiveSpells::ActiveSpellParams ActiveSpells::ActiveSpellParams::toEsm() const
    {
        ESM::ActiveSpells::ActiveSpellParams params;
        params.mActiveSpellId = mActiveSpellId;
        params.mSourceSpellId = mSourceSpellId;
        params.mEffects = mEffects;
        params.mDisplayName = mDisplayName;
        params.mCaster = mCaster;
        params.mItem = mItem;
        params.mFlags = mFlags;
        params.mWorsenings = mWorsenings;
        params.mNextWorsening = mNextWorsening.toEsm();
        return params;
    }

    void ActiveSpells::ActiveSpellParams::setFlag(ESM::ActiveSpells::Flags flag)
    {
        mFlags = static_cast<ESM::ActiveSpells::Flags>(mFlags | flag);
    }

    void ActiveSpells::ActiveSpellParams::worsen()
    {
        ++mWorsenings;
        if (!mWorsenings)
            mNextWorsening = MWBase::Environment::get().getWorld()->getTimeStamp();
        mNextWorsening += CorprusStats::sWorseningPeriod;
    }

    bool ActiveSpells::ActiveSpellParams::shouldWorsen() const
    {
        return mWorsenings >= 0 && MWBase::Environment::get().getWorld()->getTimeStamp() >= mNextWorsening;
    }

    void ActiveSpells::ActiveSpellParams::resetWorsenings()
    {
        mWorsenings = -1;
    }

    ESM::RefId ActiveSpells::ActiveSpellParams::getEnchantment() const
    {
        // Enchantment id is not stored directly. Instead the enchanted item is stored.
        const auto& store = MWBase::Environment::get().getESMStore();
        switch (store->find(mSourceSpellId))
        {
            case ESM::REC_ARMO:
                return store->get<ESM::Armor>().find(mSourceSpellId)->mEnchant;
            case ESM::REC_BOOK:
                return store->get<ESM::Book>().find(mSourceSpellId)->mEnchant;
            case ESM::REC_CLOT:
                return store->get<ESM::Clothing>().find(mSourceSpellId)->mEnchant;
            case ESM::REC_WEAP:
                return store->get<ESM::Weapon>().find(mSourceSpellId)->mEnchant;
            default:
                return {};
        }
    }

    const ESM::Spell* ActiveSpells::ActiveSpellParams::getSpell() const
    {
        return MWBase::Environment::get().getESMStore()->get<ESM::Spell>().search(getSourceSpellId());
    }

    bool ActiveSpells::ActiveSpellParams::hasFlag(ESM::ActiveSpells::Flags flags) const
    {
        return static_cast<ESM::ActiveSpells::Flags>(mFlags & flags) == flags;
    }

    void ActiveSpells::update(const MWWorld::Ptr& ptr, float duration)
    {
        if (mIterating)
            return;
        auto& creatureStats = ptr.getClass().getCreatureStats(ptr);
        assert(&creatureStats.getActiveSpells() == this);
        IterationGuard guard{ *this };
        // Erase no longer active spells and effects
        for (auto spellIt = mSpells.begin(); spellIt != mSpells.end();)
        {
            if (spellIt->hasFlag(ESM::ActiveSpells::Flag_SpellStore))
            {
                const ESM::Spell* spell
                    = MWBase::Environment::get().getESMStore()->get<ESM::Spell>().search(spellIt->mSourceSpellId);
                if (spell && ptr.getClass().getCreatureStats(ptr).getSpells().hasSpell(spell))
                    ++spellIt;
                else
                {
                    /*
                        Start of tes3mp addition

                        Whenever the local player loses an active spell, send an ID_PLAYER_SPELLS_ACTIVE packet to the server with it

                        Whenever a local actor loses an active spell, send an ID_ACTOR_SPELLS_ACTIVE packet to the server with it
                    */
                    const std::string lostSpellId = mwmp::RefIdCompat::toWire(spellIt->getSourceSpellId());
                    const MWWorld::TimeStamp lostTimeStamp = spellIt->getTimeStamp();

                    if (this == &MWMechanics::getPlayer().getClass().getCreatureStats(MWMechanics::getPlayer()).getActiveSpells())
                    {
                        mwmp::Main::get().getLocalPlayer()->sendSpellsActiveRemoval(lostSpellId,
                            MechanicsHelper::isStackingSpell(lostSpellId), lostTimeStamp);
                    }
                    /*
                        0.8.1 found the owning actor with searchPtrViaActorId(getActorId()),
                        neither of which exists in 0.51. It does not need finding: update()
                        is handed the actor these spells belong to, and asserts as much.
                    */
                    else if (mwmp::Main::get().getCellController()->isLocalActor(ptr))
                    {
                        mwmp::Main::get().getCellController()->getLocalActor(ptr)->sendSpellsActiveRemoval(lostSpellId,
                            MechanicsHelper::isStackingSpell(lostSpellId), lostTimeStamp);
                    }
                    /*
                        End of tes3mp addition
                    */
                    if (spell == nullptr)
                        Log(Debug::Error) << "Dropping non-existent active effect: " << spellIt->mSourceSpellId;
                    auto params = *spellIt;
                    spellIt = mSpells.erase(spellIt);
                    for (const auto& effect : params.mEffects)
                        onMagicEffectRemoved(ptr, params, effect);
                    applyPurges(ptr, &spellIt);
                }
                continue;
            }
            else if (!spellIt->hasFlag(ESM::ActiveSpells::Flag_Temporary))
            {
                ++spellIt;
                continue;
            }
            bool removedSpell = false;
            for (auto effectIt = spellIt->mEffects.begin(); effectIt != spellIt->mEffects.end();)
            {
                if (effectIt->mFlags & ESM::ActiveEffect::Flag_Remove && effectIt->mTimeLeft <= 0.f)
                {
                    auto effect = *effectIt;
                    effectIt = spellIt->mEffects.erase(effectIt);
                    onMagicEffectRemoved(ptr, *spellIt, effect);
                    removedSpell = applyPurges(ptr, &spellIt, &effectIt);
                    if (removedSpell)
                        break;
                }
                else
                {
                    ++effectIt;
                }
            }
            if (removedSpell)
                continue;
            if (spellIt->mEffects.empty())
                spellIt = mSpells.erase(spellIt);
            else
                ++spellIt;
        }

        UpdateContext context(duration > 0.f);
        for (const auto& spell : mQueue)
            addToSpells(ptr, spell, context);
        mQueue.clear();

        if (!creatureStats.isDead())
        {
            // Vanilla only does this on cell change I think
            const auto& spells = creatureStats.getSpells();
            for (const ESM::Spell* spell : spells)
            {
                if (spell->mData.mType != ESM::Spell::ST_Spell && spell->mData.mType != ESM::Spell::ST_Power
                    && !isSpellActive(spell->mId))
                {
                    initParams(ptr, ActiveSpellParams{ spell, ptr, true }, context);
                }
            }
        }

        if (ptr.getClass().hasInventoryStore(ptr)
            && !(creatureStats.isDead() && creatureStats.isDeathAnimationFinished()))
        {
            auto& store = ptr.getClass().getInventoryStore(ptr);
            if (store.getInvListener() != nullptr)
            {
                context.mPlayNonLooping = !store.isFirstEquip();
                const auto world = MWBase::Environment::get().getWorld();
                for (int slotIndex = 0; slotIndex < MWWorld::InventoryStore::Slots; slotIndex++)
                {
                    auto slot = store.getSlot(slotIndex);
                    if (slot == store.end())
                        continue;
                    const ESM::RefId& enchantmentId = slot->getClass().getEnchantment(*slot);
                    if (enchantmentId.empty())
                        continue;
                    const ESM::Enchantment* enchantment
                        = world->getStore().get<ESM::Enchantment>().search(enchantmentId);
                    if (enchantment == nullptr || enchantment->mData.mType != ESM::Enchantment::ConstantEffect)
                        continue;
                    if (std::find_if(mSpells.begin(), mSpells.end(),
                            [&](const ActiveSpellParams& params) {
                                return params.mItem == slot->getCellRef().getRefNum()
                                    && params.hasFlag(ESM::ActiveSpells::Flag_Equipment)
                                    && params.mSourceSpellId == slot->getCellRef().getRefId();
                            })
                        != mSpells.end())
                        continue;
                    // world->breakInvisibility leads to a stack overflow as it calls this method so just break
                    // invisibility manually
                    purgeEffect(ptr, ESM::MagicEffect::Invisibility);
                    applyPurges(ptr);
                    const bool added = initParams(ptr, ActiveSpellParams{ *slot, enchantment, ptr }, context);
                    if (added)
                        context.mUpdateSpellWindow = true;
                }
            }
        }

        const MWWorld::Ptr player = MWMechanics::getPlayer();
        // Update effects
        context.mEraseRemoved = true;
        for (auto spellIt = mSpells.begin(); spellIt != mSpells.end();)
        {
            updateActiveSpell(ptr, duration, spellIt, context);
        }

        if (Settings::game().mClassicCalmSpellsBehavior)
        {
            ESM::RefId effect
                = ptr.getClass().isNpc() ? ESM::MagicEffect::CalmHumanoid : ESM::MagicEffect::CalmCreature;
            if (creatureStats.getMagicEffects().getOrDefault(effect).getMagnitude() > 0.f)
                creatureStats.getAiSequence().stopCombat();
        }

        if (ptr == player && context.mUpdateSpellWindow)
        {
            // Something happened with the spell list -- possibly while the game is paused,
            // so we want to make the spell window get the memo.
            // We don't normally want to do this, so this targets constant enchantments.
            MWBase::Environment::get().getWindowManager()->updateSpellWindow();
        }
    }

    bool ActiveSpells::updateActiveSpell(
        const MWWorld::Ptr& ptr, float duration, Collection::iterator& spellIt, UpdateContext& context)
    {
        const auto caster = MWBase::Environment::get().getWorldModel()->getPtr(spellIt->mCaster);
        bool removedSpell = false;
        std::optional<ActiveSpellParams> reflected;
        for (auto it = spellIt->mEffects.begin(); it != spellIt->mEffects.end();)
        {
            if (it->mFlags & ESM::ActiveEffect::Flag_Remove && it->mTimeLeft <= 0.f
                && spellIt->hasFlag(ESM::ActiveSpells::Flag_Temporary))
            {
                ++it;
                continue;
            }
            auto result = applyMagicEffect(ptr, caster, *spellIt, *it, duration, context.mPlayNonLooping);
            if (result.mType == MagicApplicationResult::Type::REFLECTED)
            {
                if (!reflected)
                {
                    if (Settings::game().mClassicReflectedAbsorbSpellsBehavior)
                        reflected = { *spellIt, caster };
                    else
                        reflected = { *spellIt, ptr };
                }
                auto& reflectedEffect = reflected->mEffects.emplace_back(*it);
                reflectedEffect.mFlags
                    = ESM::ActiveEffect::Flag_Ignore_Reflect | ESM::ActiveEffect::Flag_Ignore_SpellAbsorption;
                it = spellIt->mEffects.erase(it);
            }
            else if (result.mType == MagicApplicationResult::Type::REMOVED)
                it = spellIt->mEffects.erase(it);
            else
            {
                const MWWorld::Ptr player = MWMechanics::getPlayer();
                ++it;
                if (!context.mUpdatedEnemy && result.mShowHealth && caster == player && ptr != player)
                {
                    MWBase::Environment::get().getWindowManager()->setEnemy(ptr);
                    context.mUpdatedEnemy = true;
                }
                if (!context.mUpdatedHitOverlay && result.mShowHit && ptr == player)
                {
                    MWBase::Environment::get().getWindowManager()->activateHitOverlay(false);
                    context.mUpdatedHitOverlay = true;
                }
            }
            removedSpell = applyPurges(ptr, &spellIt, &it);
            if (removedSpell)
                break;
        }
        if (reflected)
        {
            const ESM::Static* reflectStatic = MWBase::Environment::get().getESMStore()->get<ESM::Static>().find(
                ESM::RefId::stringRefId("VFX_Reflect"));
            MWRender::Animation* animation = MWBase::Environment::get().getWorld()->getAnimation(ptr);
            if (animation && !reflectStatic->mModel.empty())
            {
                const VFS::Path::Normalized reflectStaticModel
                    = Misc::ResourceHelpers::correctMeshPath(VFS::Path::Normalized(reflectStatic->mModel));
                animation->addEffect(reflectStaticModel, ESM::MagicEffect::Reflect.getValue(), false);
            }
            caster.getClass().getCreatureStats(caster).getActiveSpells().addSpell(*reflected);
        }
        if (removedSpell)
            return true;

        if (context.mEraseRemoved)
        {
            bool remove = false;
            if (spellIt->hasFlag(ESM::ActiveSpells::Flag_Equipment))
            {
                // Remove effects tied to equipment that has been unequipped
                const auto& store = ptr.getClass().getInventoryStore(ptr);
                remove = true;
                for (int slotIndex = 0; slotIndex < MWWorld::InventoryStore::Slots; slotIndex++)
                {
                    auto slot = store.getSlot(slotIndex);
                    if (slot != store.end() && slot->getCellRef().getRefNum().isSet()
                        && slot->getCellRef().getRefNum() == spellIt->mItem)
                    {
                        remove = false;
                        break;
                    }
                }
            }
            if (remove)
            {
                auto params = *spellIt;
                spellIt = mSpells.erase(spellIt);
                for (const auto& effect : params.mEffects)
                    onMagicEffectRemoved(ptr, params, effect);
                applyPurges(ptr, &spellIt);
                context.mUpdateSpellWindow = true;
                return true;
            }
        }
        ++spellIt;
        return false;
    }

    bool ActiveSpells::initParams(
        const MWWorld::Ptr& ptr, const ActiveSpellParams& params, UpdateContext& context, ESM::RefId* addedId)
    {
        const ESM::RefId activeSpellId = MWBase::Environment::get().getESMStore()->generateId();
        mSpells.emplace_back(params).setActiveSpellId(activeSpellId);
        auto it = mSpells.end();
        --it;
        // We instantly apply the effect with a duration of 0 so continuous effects can be purged before truly applying
        if (context.mUpdate && updateActiveSpell(ptr, 0.f, it, context))
            return false;
        /*
            Start of tes3mp addition

            Report which spell was added, so the caller can find it again once it has been
            applied. See addToSpells().
        */
        if (addedId != nullptr)
            *addedId = activeSpellId;
        /*
            End of tes3mp addition
        */
        return true;
    }

    void ActiveSpells::addToSpells(const MWWorld::Ptr& ptr, const ActiveSpellParams& spell, UpdateContext& context)
    {
        if (!spell.hasFlag(ESM::ActiveSpells::Flag_Stackable))
        {
            auto found = std::find_if(mSpells.begin(), mSpells.end(), [&](const auto& existing) {
                return spell.mSourceSpellId == existing.mSourceSpellId && spell.mCaster == existing.mCaster
                    && spell.mItem == existing.mItem;
            });
            if (found != mSpells.end())
            {
                if (!spell.hasFlag(ESM::ActiveSpells::Flag_Temporary))
                    return;
                if (merge(found->mEffects, spell.mEffects))
                    return;
                for (auto& effect : found->mEffects)
                    effect.mTimeLeft = 0.f;
            }
        }

        /*
            Start of tes3mp addition

            Whenever a player gains an active spell as a result of gameplay, send an ID_PLAYER_SPELLS_ACTIVE packet
            to the server with it

            Whenever a local actor gains an active spell, send an ID_ACTOR_SPELLS_ACTIVE packet to the server with it
        */
        /*
            0.8.1 sent this from addSpell(), which both inserted the spell and knew whether
            to send. Neither is true in 0.51: addSpell() only appends to mQueue, and it has
            no idea whose ActiveSpells it belongs to -- 0.8.1 recovered that with
            searchPtrViaActorId(getActorId()), and neither of those exists any more.

            addToSpells() is where the spell genuinely becomes active, and update() hands it
            the actor, so that is where the packet goes. The suppression flag rides along on
            the params (see ActiveSpellParams::mSendPacket) because the queue puts a frame
            between the call and the effect.

            It is sent AFTER initParams() rather than before, because 0.51 moved when an
            effect's magnitude is decided. 0.47 rolled it in inflict(), at cast time, so
            0.8.1's hook saw the final number; 0.51 rolls it in applyMagicEffect(), which
            runs inside initParams(). Announcing the spell any earlier puts a magnitude of
            zero on the wire, and a shield worth zero points is reaped by
            CharacterController::updateContinuousVfx() on the receiving client the moment it
            appears -- the cast is heard and seen, and nothing remains of it.

            This also means the merge and already-active paths above send nothing, unlike
            0.8.1. Those paths do not add a spell; 0.8.1 sent a packet for them anyway and
            the receiver, whose addSpellsActive() always adds, grew a duplicate copy of a
            spell it already had.
        */
        ESM::RefId addedId;
        if (!initParams(ptr, spell, context, &addedId) || addedId.empty())
            return;

        /*
            Only temporary spells are announced. 0.47 kept abilities, diseases and constant
            enchantments out of ActiveSpells entirely, so 0.8.1's hook could not see them
            and the ID_*_SPELLS_ACTIVE packets have no way to describe one -- the receiving
            side rebuilds every spell it is told about as temporary. 0.51 does keep them
            here, so without this test a contracted disease would be broadcast and would
            come back as a temporary effect that never expires. They are already covered by
            the spellbook and equipment packets.
        */
        if (!spell.getSendPacket() || !spell.hasFlag(ESM::ActiveSpells::Flag_Temporary))
            return;

        const TIterator applied = getActiveSpellById(addedId);
        if (applied == end())
            return;

        const std::string spellId = mwmp::RefIdCompat::toWire(applied->getSourceSpellId());

        if (this == &MWMechanics::getPlayer().getClass().getCreatureStats(MWMechanics::getPlayer()).getActiveSpells())
        {
            mwmp::Main::get().getLocalPlayer()->sendSpellsActiveAddition(
                spellId, MechanicsHelper::isStackingSpell(spellId), *applied);
        }
        else if (mwmp::Main::get().getCellController()->isLocalActor(ptr))
        {
            mwmp::Main::get().getCellController()->getLocalActor(ptr)->sendSpellsActiveAddition(
                spellId, MechanicsHelper::isStackingSpell(spellId), *applied);
        }
        /*
            End of tes3mp addition
        */
    }

    ActiveSpells::ActiveSpells()
        : mIterating(false)
    {
    }

    ActiveSpells::TIterator ActiveSpells::begin() const
    {
        return mSpells.begin();
    }

    ActiveSpells::TIterator ActiveSpells::end() const
    {
        return mSpells.end();
    }

    ActiveSpells::TIterator ActiveSpells::getActiveSpellById(const ESM::RefId& id)
    {
        for (TIterator it = begin(); it != end(); it++)
            if (it->getActiveSpellId() == id)
                return it;
        return end();
    }

    bool ActiveSpells::isSpellActive(const ESM::RefId& id) const
    {
        return std::find_if(mSpells.begin(), mSpells.end(), [&](const auto& spell) {
            return spell.mSourceSpellId == id;
        }) != mSpells.end();
    }

    bool ActiveSpells::isEnchantmentActive(const ESM::RefId& id) const
    {
        const auto& store = MWBase::Environment::get().getESMStore();
        if (store->get<ESM::Enchantment>().search(id) == nullptr)
            return false;

        return std::find_if(mSpells.begin(), mSpells.end(), [&](const auto& spell) {
            return spell.getEnchantment() == id;
        }) != mSpells.end();
    }

    /*
        Start of tes3mp change (major)

        0.8.1 added an ActiveSpells::addSpell overload taking
        (id, stack, effects, displayName, casterActorId, timestamp, sendPacket) so spells
        arriving from other clients could keep their original timestamps, and so echoing a
        spell back to the server could be suppressed.

        0.51 rebuilt ActiveSpells around a queue of ActiveSpellParams, and three of those
        seven arguments no longer describe anything:

          - "stack" -- every queued ActiveSpellParams already has its own mActiveSpellId,
            so two copies of one spell coexist by default and are individually
            addressable. Stacking is the model, not a flag on it.
          - casterActorId -- casters are ESM::RefNum now, held in mCaster, and resolved
            through WorldModel::getPtr().
          - the id and effects, which are what ActiveSpellParams itself carries.

        What genuinely has no 0.51 equivalent is the timestamp, so that is the only thing
        the tes3mp overload still adds; see addSpell(params, timestamp, sendPacket) below.
        The packet itself is sent from mwmp/, which has the player identity this class
        does not -- that is also the cheaper shape for the next port.
    */
    /*
        End of tes3mp change (major)
    */
    void ActiveSpells::addSpell(const ActiveSpellParams& params)
    {
        /*
            Start of tes3mp addition

            Stamp a spell gained through gameplay with the current time, which is what 0.8.1
            did by giving the timestamp argument a default. It is what a removal packet names
            to say WHICH of several stacked copies of one spell went away, so a spell that
            reaches the server with no timestamp cannot be individually removed later.
        */
        ActiveSpellParams stamped = params;
        stamped.setTimeStamp(MWBase::Environment::get().getWorld()->getTimeStamp());
        mQueue.emplace_back(std::move(stamped));
        /*
            End of tes3mp addition
        */
    }

    void ActiveSpells::addSpell(const ESM::Spell* spell, const MWWorld::Ptr& actor, bool ignoreResistances)
    {
        /*
            Start of tes3mp change (minor)

            As above: stamp it with the current time.
        */
        ActiveSpellParams stamped{ spell, actor, ignoreResistances };
        stamped.setTimeStamp(MWBase::Environment::get().getWorld()->getTimeStamp());
        mQueue.emplace_back(std::move(stamped));
        /*
            End of tes3mp change (minor)
        */
    }

    void ActiveSpells::purge(ParamsPredicate predicate, const MWWorld::Ptr& ptr)
    {
        assert(&ptr.getClass().getCreatureStats(ptr).getActiveSpells() == this);
        mPurges.emplace(predicate);
        if (!mIterating)
        {
            IterationGuard guard{ *this };
            applyPurges(ptr);
        }
    }

    void ActiveSpells::purge(EffectPredicate predicate, const MWWorld::Ptr& ptr)
    {
        assert(&ptr.getClass().getCreatureStats(ptr).getActiveSpells() == this);
        mPurges.emplace(predicate);
        if (!mIterating)
        {
            IterationGuard guard{ *this };
            applyPurges(ptr);
        }
    }

    bool ActiveSpells::applyPurges(const MWWorld::Ptr& ptr, std::list<ActiveSpellParams>::iterator* currentSpell,
        std::vector<ActiveEffect>::iterator* currentEffect)
    {
        bool removedCurrentSpell = false;
        while (!mPurges.empty())
        {
            auto predicate = mPurges.front();
            mPurges.pop();
            for (auto spellIt = mSpells.begin(); spellIt != mSpells.end();)
            {
                bool isCurrentSpell = currentSpell && *currentSpell == spellIt;
                std::visit(
                    [&](auto&& variant) {
                        using T = std::decay_t<decltype(variant)>;
                        if constexpr (std::is_same_v<T, ParamsPredicate>)
                        {
                            if (variant(*spellIt))
                            {
                                auto params = *spellIt;
                                spellIt = mSpells.erase(spellIt);
                                if (isCurrentSpell)
                                {
                                    *currentSpell = spellIt;
                                    removedCurrentSpell = true;
                                }
                                for (const auto& effect : params.mEffects)
                                    onMagicEffectRemoved(ptr, params, effect);
                            }
                            else
                                ++spellIt;
                        }
                        else
                        {
                            static_assert(std::is_same_v<T, EffectPredicate>, "Non-exhaustive visitor");
                            for (auto effectIt = spellIt->mEffects.begin(); effectIt != spellIt->mEffects.end();)
                            {
                                if (variant(*spellIt, *effectIt))
                                {
                                    auto effect = *effectIt;
                                    if (isCurrentSpell && currentEffect)
                                    {
                                        auto distance = std::distance(spellIt->mEffects.begin(), *currentEffect);
                                        if (effectIt <= *currentEffect)
                                            distance--;
                                        effectIt = spellIt->mEffects.erase(effectIt);
                                        *currentEffect = spellIt->mEffects.begin() + distance;
                                    }
                                    else
                                        effectIt = spellIt->mEffects.erase(effectIt);
                                    onMagicEffectRemoved(ptr, *spellIt, effect);
                                }
                                else
                                    ++effectIt;
                            }
                            ++spellIt;
                        }
                    },
                    predicate);
            }
        }
        return removedCurrentSpell;
    }

    void ActiveSpells::removeEffectsBySourceSpellId(const MWWorld::Ptr& ptr, const ESM::RefId& id)
    {
        purge([=](const ActiveSpellParams& params) { return params.mSourceSpellId == id; }, ptr);
    }

    void ActiveSpells::removeEffectsByActiveSpellId(const MWWorld::Ptr& ptr, const ESM::RefId& id)
    {
        purge([=](const ActiveSpellParams& params) { return params.mActiveSpellId == id; }, ptr);
    }

    void ActiveSpells::purgeEffect(const MWWorld::Ptr& ptr, ESM::RefId effectId, ESM::RefId effectArg)
    {
        purge(
            [=](const ActiveSpellParams&, const ESM::ActiveEffect& effect) {
                if (!(effect.mFlags & ESM::ActiveEffect::Flag_Applied))
                    return false;
                if (effectArg.empty())
                    return effect.mEffectId == effectId;
                return effect.mEffectId == effectId && effect.getSkillOrAttribute() == effectArg;
            },
            ptr);
    }

    void ActiveSpells::purge(const MWWorld::Ptr& ptr, ESM::RefNum actor)
    {
        purge([=](const ActiveSpellParams& params) { return params.mCaster == actor; }, ptr);
    }

    void ActiveSpells::clear(const MWWorld::Ptr& ptr)
    {
        mQueue.clear();
        purge([](const ActiveSpellParams& params) { return true; }, ptr);
    }

    void ActiveSpells::skipWorsenings(double hours)
    {
        for (auto& spell : mSpells)
        {
            if (spell.mWorsenings >= 0)
                spell.mNextWorsening += hours;
        }
    }

    void ActiveSpells::writeState(ESM::ActiveSpells& state) const
    {
        for (const auto& spell : mSpells)
            state.mSpells.emplace_back(spell.toEsm());
        for (const auto& spell : mQueue)
            state.mQueue.emplace_back(spell.toEsm());
    }

    void ActiveSpells::readState(const ESM::ActiveSpells& state)
    {
        for (const ESM::ActiveSpells::ActiveSpellParams& spell : state.mSpells)
        {
            mSpells.emplace_back(ActiveSpellParams{ spell });
            // Generate ID for older saves that didn't have any.
            if (mSpells.back().getActiveSpellId().empty())
                mSpells.back().setActiveSpellId(MWBase::Environment::get().getESMStore()->generateId());
        }
        for (const ESM::ActiveSpells::ActiveSpellParams& spell : state.mQueue)
            mQueue.emplace_back(ActiveSpellParams{ spell });
        if (state.mActorIdConverter)
        {
            const auto convertSummons = [converter = state.mActorIdConverter](auto& collection) {
                for (ActiveSpellParams& params : collection)
                {
                    converter->convert(params.mCaster, params.mCaster.mIndex);
                    for (ESM::ActiveEffect& effect : params.mEffects)
                    {
                        if (ESM::RefNum* refNum = std::get_if<ESM::RefNum>(&effect.mArg))
                            converter->convert(*refNum, refNum->mIndex);
                    }
                }
            };
            convertSummons(mSpells);
            convertSummons(mQueue);
        }
    }

    void ActiveSpells::unloadActor(const MWWorld::Ptr& ptr)
    {
        purge([](const auto& spell) { return spell.hasFlag(ESM::ActiveSpells::Flag_Temporary); }, ptr);
        mQueue.clear();
    }

    /*
        Start of tes3mp addition

        Add a separate addSpell() with a timestamp argument, so a spell arriving from
        another client keeps the timing it had there, and so relaying it straight back to
        the server can be suppressed.
    */
    void ActiveSpells::addSpell(const ActiveSpellParams& params, MWWorld::TimeStamp timestamp, bool sendPacket)
    {
        ActiveSpellParams stamped = params;
        stamped.setTimeStamp(timestamp);

        /*
            0.8.1 read sendPacket here, because addSpell() was also what sent the packet.
            0.51 only queues here and applies the spell a frame later in addToSpells(), so
            the flag has to travel on the params to reach the hook that reads it.
        */
        stamped.setSendPacket(sendPacket);

        /*
            0.8.1 took a "stack" flag here. 0.51 does not need one: every queued
            ActiveSpellParams gets its own mActiveSpellId, so two copies of the same spell
            coexist as a matter of course and are individually addressable. Passing the
            params through unchanged therefore reproduces stack == true; the non-stacking
            case is expressed by the caller purging the previous instance first, which is
            what 0.51's own code does.
        */
        mQueue.emplace_back(stamped);

    }
    /*
        End of tes3mp addition
    */

    /*
        Start of tes3mp addition

        Remove the spell with a certain ID and a certain timestamp, useful
        when there are stacked spells with the same ID

        Returns a boolean that indicates whether the corresponding spell was found
    */
    bool ActiveSpells::removeSpellByTimestamp(const MWWorld::Ptr& ptr, const ESM::RefId& id, MWWorld::TimeStamp timestamp)
    {
        /*
            0.8.1 cleared the effect vector in place and set a dirty flag. 0.51 removes
            active spells through purge(), which also runs onMagicEffectRemoved for each
            effect -- so this now actually undoes the effect rather than just dropping the
            bookkeeping. That is a behaviour fix, not a port artefact: the old version
            leaked permanent modifiers for stacked spells.

            Both the queue and the applied list are searched, because a spell added this
            frame has not been promoted out of mQueue yet.
        */
        const auto matches = [&](const ActiveSpellParams& spell) {
            return spell.getSourceSpellId() == id && spell.getTimeStamp() == timestamp;
        };

        bool found = std::any_of(mSpells.begin(), mSpells.end(), matches);

        if (found)
            purge(ParamsPredicate{ matches }, ptr);

        const auto queued = std::find_if(mQueue.begin(), mQueue.end(), matches);
        if (queued != mQueue.end())
        {
            mQueue.erase(queued);
            found = true;
        }

        return found;
    }
    /*
        End of tes3mp addition
    */

    /*
        Start of tes3mp addition

        Make it easy to get an effect's duration
    */
    float ActiveSpells::getEffectDuration(const ESM::RefId& effectId, const ESM::RefId& sourceId) const
    {
        for (const auto& spell : mSpells)
        {
            if (spell.getSourceSpellId() != sourceId)
                continue;

            for (const auto& effect : spell.getEffects())
            {
                if (effect.mEffectId == effectId)
                    return effect.mDuration;
            }
        }

        return 0.f;
    }
    /*
        End of tes3mp addition
    */

}

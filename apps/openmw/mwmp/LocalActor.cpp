#include <components/openmw-mp/TimedLog.hpp>

#include "../mwbase/environment.hpp"

#include "../mwmechanics/mechanicsmanagerimp.hpp"
#include "../mwmechanics/movement.hpp"

#include "../mwrender/animation.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/worldimp.hpp"

#include <components/esm3/statstate.hpp>

#include "LocalActor.hpp"
#include "Main.hpp"
#include "Networking.hpp"
#include "ActorList.hpp"
#include "MechanicsHelper.hpp"
#include "RecordConvertPlayer.hpp"
#include "RefIdCompat.hpp"
#include "RefNumCompat.hpp"

#include "../mwworld/worldmodel.hpp"

using namespace mwmp;

LocalActor::LocalActor()
{
    hasSentData = false;
    posWasChanged = false;
    equipmentChanged = false;

    wasRunning = false;
    wasSneaking = false;
    wasForceJumping = false;
    wasForceMoveJumping = false;
    wasFlying = false;

    attack.type = Attack::MELEE;
    attack.shouldSend = false;
    attack.instant = false;
    attack.pressed = false;

    cast.type = Cast::REGULAR;
    cast.shouldSend = false;
    cast.instant = false;
    cast.pressed = false;

    killer.isPlayer = false;
    killer.refId = "";
    killer.name = "";

    creatureStats.mDead = false;
    creatureStats.mDeathAnimationFinished = false;
}

LocalActor::~LocalActor()
{

}

void LocalActor::update(bool forceUpdate)
{
    updateStatsDynamic(forceUpdate);
    updateEquipment(forceUpdate, false);

    /*
        Start of tes3mp change (major)

        Only stop updating an actor once it is DEAD and its death animation has finished.

        The gate used to be "!mDeathAnimationFinished" alone. That reads as "this actor has
        settled, stop sending it", but the flag does not mean that on a living actor:
        Creature::ensureCustomData and Npc::ensureCustomData both initialise it with

            setDeathAnimationFinished(isPersistent(ptr))

        so every persistent actor -- which is most named NPCs, guards and shopkeepers -- has
        it set to true from the moment it is created, while alive and walking.

        The consequence was that a persistent actor got exactly one position update, the
        forced one when its LocalActor is first created, and was never reported again. On
        every other client it stood frozen where it happened to be, forever. Non-persistent
        creatures -- rats, cliff racers, scribs -- have the flag false and synchronised
        perfectly, which is why this looked like "NPCs are broken but wildlife is fine".

        Measured before the fix: updatePosition was reached for about 4% of the calls the
        send loop made, and the authority queued 0.1 of 4.4 actors per cycle while 8 of 12
        were provably moving.
    */
    if (forceUpdate || !creatureStats.mDead || !creatureStats.mDeathAnimationFinished)
    {
    /*
        End of tes3mp change (major)
    */
        updatePosition(forceUpdate);
        updateAnimFlags(forceUpdate);
        updateAnimPlay();
        updateSpeech();
        updateAttackOrCast();
    }

    hasSentData = true;
}

void LocalActor::updateCell()
{
    LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Sending ID_ACTOR_CELL_CHANGE about %s %i-%i in cell %s to server",
                       refId.c_str(), refNum, mpNum, cell.getShortDescription().c_str());

    LOG_APPEND(TimedLog::LOG_VERBOSE, "- Moved to cell %s", ptr.getCell()->getCell()->getShortDescription().c_str());

    cell = mwmp::RecordConvert::toMirror(*ptr.getCell()->getCell());
    position = mwmp::RecordConvert::toMirror(ptr.getRefData().getPosition());
    isFollowerCellChange = false;

    mwmp::Main::get().getNetworking()->getActorList()->addCellChangeActor(*this);
}

void LocalActor::updatePosition(bool forceUpdate)
{
    bool posIsChanging = false;

    if (creatureStats.mDead)
    {
        ESM::Position ptrPosition = ptr.getRefData().getPosition();
        posIsChanging = position.pos[0] != ptrPosition.pos[0] || position.pos[1] != ptrPosition.pos[1] ||
            position.pos[2] != ptrPosition.pos[2];
    }
    else
    {
        /*
            Start of tes3mp change (major)

            Decide "is this actor moving" from its actual position, not from direction alone.

            0.8.1 could rely on direction because in 0.47 nothing stood between the AI writing
            the actor's movement settings and CharacterController::update, where tes3mp copies
            them into direction. 0.51 inserted updateLuaControls into exactly that gap
            (Actors::update calls it between AiSequence::execute and ctrl.update), and it both
            reads and rewrites mov.mPosition. By the time the hook runs, direction is zero for
            all but a handful of frames.

            Measured on a fort full of NPCs: 4 of 2500 updatePosition calls saw any direction
            at all, so the authority sent 3 position packets in a session and the other client
            received 2. Its copies then applied the last direction they were given forever --
            NPCs animating a walk while their position never advanced, which is what "walking
            on the spot" is.

            Comparing the position we last sent against the position the actor is at now needs
            nothing from the engine's movement plumbing, so no future reshuffle of it can
            silently switch actor sync off again. It is also what the dead-actor branch above
            has always done. direction and isOnGround are kept as additional triggers so a
            turn on the spot, or a fall, still reports.
        */
        ESM::Position ptrPosition = ptr.getRefData().getPosition();

        posIsChanging = position.pos[0] != ptrPosition.pos[0] || position.pos[1] != ptrPosition.pos[1]
            || position.pos[2] != ptrPosition.pos[2] || position.rot[0] != ptrPosition.rot[0]
            || position.rot[2] != ptrPosition.rot[2] || direction.pos[0] != 0 || direction.pos[1] != 0
            || direction.pos[2] != 0 || direction.rot[0] != 0 || direction.rot[1] != 0
            || direction.rot[2] != 0 || !MWBase::Environment::get().getWorld()->isOnGround(ptr)
            || direction.pos[0] != sentDirection.pos[0] || direction.pos[1] != sentDirection.pos[1]
            || direction.pos[2] != sentDirection.pos[2] || direction.rot[0] != sentDirection.rot[0]
            || direction.rot[1] != sentDirection.rot[1] || direction.rot[2] != sentDirection.rot[2];
        /*
            End of tes3mp change (major)
        */
    }

    if (forceUpdate || posIsChanging || posWasChanged)
    {
        posWasChanged = posIsChanging;
        position = mwmp::RecordConvert::toMirror(ptr.getRefData().getPosition());
        sentDirection = direction;
        mwmp::Main::get().getNetworking()->getActorList()->addPositionActor(*this);
    }
}

void LocalActor::updateAnimFlags(bool forceUpdate)
{
    MWBase::World *world = MWBase::Environment::get().getWorld();
    MWMechanics::CreatureStats ptrCreatureStats = ptr.getClass().getCreatureStats(ptr);

    using namespace MWMechanics;

    bool isRunning = ptrCreatureStats.getMovementFlag(CreatureStats::Flag_Run);
    bool isSneaking = ptrCreatureStats.getMovementFlag(CreatureStats::Flag_Sneak);
    bool isForceJumping = ptrCreatureStats.getMovementFlag(CreatureStats::Flag_ForceJump);
    bool isForceMoveJumping = ptrCreatureStats.getMovementFlag(CreatureStats::Flag_ForceMoveJump);

    isFlying = world->isFlying(ptr);

    MWMechanics::DrawState currentDrawState = ptr.getClass().getCreatureStats(ptr).getDrawState();

    if (wasRunning != isRunning || wasSneaking != isSneaking ||
        wasForceJumping != isForceJumping || wasForceMoveJumping != isForceMoveJumping ||
        lastDrawState != currentDrawState || wasFlying != isFlying ||
        forceUpdate)
    {

        wasRunning = isRunning;
        wasSneaking = isSneaking;
        wasForceJumping = isForceJumping;
        wasForceMoveJumping = isForceMoveJumping;
        lastDrawState = currentDrawState;

        wasFlying = isFlying;

        movementFlags = 0;

#define __SETFLAG(flag, value) (value) ? (movementFlags | flag) : (movementFlags & ~flag)

        movementFlags = __SETFLAG(CreatureStats::Flag_Sneak, isSneaking);
        movementFlags = __SETFLAG(CreatureStats::Flag_Run, isRunning);
        movementFlags = __SETFLAG(CreatureStats::Flag_ForceJump, isForceJumping);
        movementFlags = __SETFLAG(CreatureStats::Flag_ForceMoveJump, isForceMoveJumping);

#undef __SETFLAG

        drawState = static_cast<char>(currentDrawState);

        mwmp::Main::get().getNetworking()->getActorList()->addAnimFlagsActor(*this);
    }
}

void LocalActor::updateAnimPlay()
{
    if (!animation.groupname.empty())
    {
        mwmp::Main::get().getNetworking()->getActorList()->addAnimPlayActor(*this);
        animation.groupname.clear();
    }
}

void LocalActor::updateSpeech()
{
    if (!sound.empty())
    {
        mwmp::Main::get().getNetworking()->getActorList()->addSpeechActor(*this);
        sound.clear();
    }
}

void LocalActor::updateStatsDynamic(bool forceUpdate)
{
    MWMechanics::CreatureStats *ptrCreatureStats = &ptr.getClass().getCreatureStats(ptr);
    MWMechanics::DynamicStat<float> health(ptrCreatureStats->getHealth());
    MWMechanics::DynamicStat<float> magicka(ptrCreatureStats->getMagicka());
    MWMechanics::DynamicStat<float> fatigue(ptrCreatureStats->getFatigue());

    // Update stats when they become 0 or they have changed enough
    //
    // Also check for an oldHealth of 0 changing to something else for resurrected NPCs

    auto needUpdate = [](MWMechanics::DynamicStat<float> &oldVal, MWMechanics::DynamicStat<float> &newVal, int limit) {
        return oldVal != newVal && (newVal.getCurrent() == 0 || oldVal.getCurrent() == 0
                                    || abs(oldVal.getCurrent() - newVal.getCurrent()) >= limit);
    };

    if (forceUpdate || needUpdate(oldHealth, health, 3) || needUpdate(oldMagicka, magicka, 7) ||
        needUpdate(oldFatigue, fatigue, 7))
    {
        oldHealth = health;
        oldMagicka = magicka;
        oldFatigue = fatigue;

        // The engine writes into its own StatState; the mirror keeps the 0.47 layout.
        ESM::StatState<float> dynamicState;
        health.writeState(dynamicState);
        mwmp::RecordConvert::fromEngine(dynamicState, creatureStats.mDynamic[0]);
        magicka.writeState(dynamicState);
        mwmp::RecordConvert::fromEngine(dynamicState, creatureStats.mDynamic[1]);
        fatigue.writeState(dynamicState);
        mwmp::RecordConvert::fromEngine(dynamicState, creatureStats.mDynamic[2]);

        creatureStats.mDead = ptrCreatureStats->isDead();
        creatureStats.mDeathAnimationFinished = ptrCreatureStats->isDeathAnimationFinished();

        mwmp::Main::get().getNetworking()->getActorList()->addStatsDynamicActor(*this);
    }
}

void LocalActor::updateEquipment(bool forceUpdate, bool sendImmediately)
{
    if (!ptr.getClass().hasInventoryStore(ptr))
        return;

    MWWorld::InventoryStore &invStore = ptr.getClass().getInventoryStore(ptr);
    
    // If we've never sent any data, autoEquip the actor just in case its inventory
    // slots have been cleared by a previous Container packet
    if (!hasSentData)
        invStore.autoEquip();

    if (forceUpdate)
        equipmentChanged = true;

    for (int slot = 0; slot < MWWorld::InventoryStore::Slots; slot++)
    {
        auto &item = equipmentItems[slot];
        MWWorld::ContainerStoreIterator it = invStore.getSlot(slot);

        if (it != invStore.end())
        {
            auto &cellRef = it->getCellRef();
            // RefId interning is already case-insensitive, so comparing the
            // serialized forms carries the old ciEqual semantics.
            if (mwmp::RefIdCompat::toWire(cellRef.getRefId()) != item.refId)
            {
                equipmentChanged = true;

                item.refId = mwmp::RefIdCompat::toWire(cellRef.getRefId());
                item.charge = cellRef.getCharge();
                item.enchantmentCharge = it->getCellRef().getEnchantmentCharge();
                item.count = it->getCellRef().getCount();
            }
        }
        else if (!item.refId.empty())
        {
            equipmentChanged = true;
            item.refId = "";
            item.count = 0;
            item.charge = -1;
            item.enchantmentCharge = -1;
        }
    }

    if (equipmentChanged)
    {
        if (sendImmediately)
            sendEquipment();
        else
            mwmp::Main::get().getNetworking()->getActorList()->addEquipmentActor(*this);

        equipmentChanged = false;
    }
}

void LocalActor::updateAttackOrCast()
{
    if (attack.shouldSend)
    {
        mwmp::Main::get().getNetworking()->getActorList()->addAttackActor(*this);
        attack.shouldSend = false;
    }
    else if (cast.shouldSend)
    {
        mwmp::Main::get().getNetworking()->getActorList()->addCastActor(*this);
        cast.shouldSend = false;
        cast.hasProjectile = false;
    }
}

void LocalActor::sendEquipment()
{
    ActorList actorList;
    actorList.cell = cell;
    actorList.addActor(*this);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_EQUIPMENT)->setActorList(&actorList);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_EQUIPMENT)->Send();
}

void LocalActor::sendSpellsActiveAddition(const std::string id, bool isStackingSpell, const MWMechanics::ActiveSpells::ActiveSpellParams& params)
{
    // Skip any bugged spells that somehow have clientside-only dynamic IDs
    if (id.find("$dynamic") != std::string::npos)
        return;

    spellsActiveChanges.activeSpells.clear();

    // 0.51 identifies the caster by ESM::RefNum; WorldModel resolves it to a Ptr.
    const MWWorld::Ptr caster = MWBase::Environment::get().getWorldModel()->getPtr(params.getCaster());

    mwmp::ActiveSpell spell;
    spell.id = id;
    spell.isStackingSpell = isStackingSpell;
    spell.caster = MechanicsHelper::getTarget(caster);
    spell.timestampDay = params.getTimeStamp().getDay();
    spell.timestampHour = params.getTimeStamp().getHour();
    mwmp::RecordConvert::fromEngine(params.getEffects(), spell.params.mEffects);
    spell.params.mDisplayName = params.getDisplayName();
    spellsActiveChanges.activeSpells.push_back(spell);

    spellsActiveChanges.action = mwmp::SpellsActiveChanges::ADD;

    ActorList actorList;
    actorList.cell = cell;
    actorList.addActor(*this);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_SPELLS_ACTIVE)->setActorList(&actorList);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_SPELLS_ACTIVE)->Send();
}

void LocalActor::sendSpellsActiveRemoval(const std::string id, bool isStackingSpell, MWWorld::TimeStamp timestamp)
{
    // Skip any bugged spells that somehow have clientside-only dynamic IDs
    if (id.find("$dynamic") != std::string::npos)
        return;

    spellsActiveChanges.activeSpells.clear();

    mwmp::ActiveSpell spell;
    spell.id = id;
    spell.isStackingSpell = isStackingSpell;
    spell.timestampDay = timestamp.getDay();
    spell.timestampHour = timestamp.getHour();
    spellsActiveChanges.activeSpells.push_back(spell);

    spellsActiveChanges.action = mwmp::SpellsActiveChanges::REMOVE;

    ActorList actorList;
    actorList.cell = cell;
    actorList.addActor(*this);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_SPELLS_ACTIVE)->setActorList(&actorList);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_SPELLS_ACTIVE)->Send();
}

void LocalActor::sendDeath(char newDeathState)
{
    deathState = newDeathState;

    if (MechanicsHelper::isEmptyTarget(killer))
        killer = MechanicsHelper::getTarget(ptr);

    LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Sending ID_ACTOR_DEATH about %s %i-%i in cell %s to server\n- deathState: %d",
        refId.c_str(), refNum, mpNum, cell.getShortDescription().c_str(), deathState);

    ActorList actorList;
    actorList.cell = cell;
    actorList.addActor(*this);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_DEATH)->setActorList(&actorList);
    Main::get().getNetworking()->getActorPacket(ID_ACTOR_DEATH)->Send();

    MechanicsHelper::clearTarget(killer);
}

MWWorld::Ptr LocalActor::getPtr()
{
    return ptr;
}

void LocalActor::setPtr(const MWWorld::Ptr& newPtr)
{
    ptr = newPtr;

    refId = mwmp::RefIdCompat::toWire(ptr.getCellRef().getRefId());
    refNum = mwmp::RefNumCompat::toWire(ptr.getCellRef());
    mpNum = ptr.getCellRef().getMpNum();

    lastDrawState = ptr.getClass().getCreatureStats(ptr).getDrawState();
    oldHealth = ptr.getClass().getCreatureStats(ptr).getHealth();
    oldMagicka = ptr.getClass().getCreatureStats(ptr).getMagicka();
    oldFatigue = ptr.getClass().getCreatureStats(ptr).getFatigue();
}

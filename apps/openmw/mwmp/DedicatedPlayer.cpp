#include <boost/algorithm/clamp.hpp>
#include <components/openmw-mp/TimedLog.hpp>
#include <apps/openmw/mwmechanics/steering.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/soundmanager.hpp"

#include "../mwclass/npc.hpp"

#include "../mwdialogue/dialoguemanagerimp.hpp"

#include "../mwgui/windowmanagerimp.hpp"

#include "../mwinput/inputmanagerimp.hpp"

#include "../mwmechanics/actor.hpp"
#include "../mwmechanics/aitravel.hpp"
#include "../mwmechanics/creaturestats.hpp"
#include "../mwmechanics/movement.hpp"
#include "../mwmechanics/npcstats.hpp"
#include "../mwmechanics/mechanicsmanagerimp.hpp"
#include "../mwmechanics/spellcasting.hpp"

#include "../mwstate/statemanagerimp.hpp"

#include "../mwworld/action.hpp"
#include "../mwworld/cellstore.hpp"
#include "../mwworld/customdata.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/player.hpp"
#include "../mwworld/worldimp.hpp"

#include <components/esm3/statstate.hpp>
#include <components/settings/values.hpp>
#include <components/vfs/pathutil.hpp>

#include "../mwworld/worldmodel.hpp"

#include "DedicatedPlayer.hpp"
#include "RecordConvert.hpp"
#include "RecordConvertPlayer.hpp"
#include "RefIdCompat.hpp"
#include "Main.hpp"
#include "GUIController.hpp"
#include "CellController.hpp"
#include "MechanicsHelper.hpp"
#include "RecordHelper.hpp"


using namespace mwmp;

DedicatedPlayer::DedicatedPlayer(RakNet::RakNetGUID guid) : BasePlayer(guid)
{
    reference = 0;
    attack.pressed = false;
    cast.pressed = false;

    creatureStats.mDead = false;
    // Give this new character a temporary high fatigue so it doesn't spawn on
    // the ground
    creatureStats.mDynamic[2].mBase = 1000;

    attack.instant = false;

    MWBase::World* world = MWBase::Environment::get().getWorld();
    
    cell = mwmp::RecordConvert::toMirror(*MWBase::Environment::get().getWorldModel()->getInterior(
        RecordHelper::getPlaceholderInteriorCellName()).getCell());
    position.pos[0] = position.pos[1] = position.pos[2] = 0;

    mwmp::RecordConvert::fromEngine(*world->getPlayerPtr().get<ESM::NPC>()->mBase, npc);
    npc.mId = "";
    previousRace = npc.mRace;

    hasReceivedInitialEquipment = false;
    hasFinishedInitialTeleportation = false;

    isJumping = false;
    wasJumping = false;
}

DedicatedPlayer::~DedicatedPlayer()
{

}

void DedicatedPlayer::update(float dt)
{
    // Only move and set anim flags if the framerate isn't too low
    if (dt < 0.1)
    {
        move(dt);
        setAnimFlags();
    }

    MWMechanics::CreatureStats *ptrCreatureStats = &ptr.getClass().getCreatureStats(ptr);

    MWMechanics::DynamicStat<float> value;

    if (creatureStats.mDead)
    {
        ESM::StatState<float> healthState;
        mwmp::RecordConvert::toEngine(creatureStats.mDynamic[0], healthState);

        value.readState(healthState);
        ptrCreatureStats->setHealth(value);
        return;
    }

    for (int i = 0; i < 3; ++i)
    {
        ESM::StatState<float> dynamicState;
        mwmp::RecordConvert::toEngine(creatureStats.mDynamic[i], dynamicState);

        value.readState(dynamicState);
        ptrCreatureStats->setDynamic(i, value);
    }

    if (ptrCreatureStats->isDead())
        MWBase::Environment::get().getMechanicsManager()->resurrect(ptr);

    ptrCreatureStats->setAttacked(false);

    ptrCreatureStats->getAiSequence().stopCombat();

    ptrCreatureStats->setAlarmed(false);
    ptrCreatureStats->setAiSetting(MWMechanics::AiSetting::Alarm, 0);
    ptrCreatureStats->setAiSetting(MWMechanics::AiSetting::Fight, 0);
    ptrCreatureStats->setAiSetting(MWMechanics::AiSetting::Flee, 0);
    ptrCreatureStats->setAiSetting(MWMechanics::AiSetting::Hello, 0);
}

void DedicatedPlayer::move(float dt)
{
    if (!reference) return;

    ESM::Position refPos = ptr.getRefData().getPosition();
    MWBase::World *world = MWBase::Environment::get().getWorld();
    const int maxInterpolationDistance = 80;

    // Apply interpolation only if the position hasn't changed too much from last time
    bool shouldInterpolate =
            abs(position.pos[0] - refPos.pos[0]) < maxInterpolationDistance &&
            abs(position.pos[1] - refPos.pos[1]) < maxInterpolationDistance &&
            abs(position.pos[2] - refPos.pos[2]) < maxInterpolationDistance;

    if (shouldInterpolate)
    {
        static const int timeMultiplier = 15;
        osg::Vec3f lerp = MechanicsHelper::getLinearInterpolation(refPos.asVec3(),
            osg::Vec3f(position.pos[0], position.pos[1], position.pos[2]), dt * timeMultiplier);

        world->moveObject(ptr, lerp);
    }
    else
        world->moveObject(ptr, osg::Vec3f(position.pos[0], position.pos[1], position.pos[2]));

    world->rotateObject(ptr, osg::Vec3f(position.rot[0], 0, position.rot[2]));

    MWMechanics::Movement *move = &ptr.getClass().getMovementSettings(ptr);
    move->mPosition[0] = direction.pos[0];
    move->mPosition[1] = direction.pos[1];
    move->mPosition[2] = direction.pos[2];

    // Make sure the values are valid, or we'll get an infinite error loop
    if (!isnan(direction.rot[0]) && !isnan(direction.rot[1]) && !isnan(direction.rot[2]))
    {
        move->mRotation[0] = direction.rot[0];
        move->mRotation[1] = direction.rot[1];
        move->mRotation[2] = direction.rot[2];
    }
}

void DedicatedPlayer::setBaseInfo()
{
    // Use the previous race if the new one doesn't exist
    if (!RecordHelper::doesRecordIdExist<ESM::Race>(npc.mRace))
        npc.mRace = previousRace;

    /*
        npc is the protocol mirror; the store holds engine records. 0.8.1 could pass it
        straight through because the two were the same type.
    */
    ESM::NPC engineNpc;
    mwmp::RecordConvert::toEngine(npc, engineNpc);

    if (!reference)
    {
        npc.mId = mwmp::RefIdCompat::toWire(RecordHelper::createRecord(engineNpc)->mId);
        createReference(npc.mId);
    }
    else
    {
        RecordHelper::overrideRecord(engineNpc);
        reloadPtr();
    }

    // Only set equipment if the player isn't disguised as a creature
    if (ptr.getType() == ESM::NPC::sRecordId)
        setEquipment();

    previousRace = npc.mRace;
}

void DedicatedPlayer::setStatsDynamic()
{
    MWMechanics::CreatureStats* ptrCreatureStats = &getPtr().getClass().getCreatureStats(getPtr());
    MWMechanics::DynamicStat<float> value;

    for (int i = 0; i < 3; ++i)
    {
        ESM::StatState<float> dynamicState;
        mwmp::RecordConvert::toEngine(creatureStats.mDynamic[i], dynamicState);

        value.readState(dynamicState);
        ptrCreatureStats->setDynamic(i, value);
    }
}

void DedicatedPlayer::setAnimFlags()
{
    using namespace MWMechanics;

    MWBase::World *world = MWBase::Environment::get().getWorld();

    // Until we figure out a better workaround for disabling player gravity,
    // simply cast Levitate over and over on a player that's supposed to be flying
    if (!isFlying && !hasTcl && !isLevitationPurged)
    {
        ptr.getClass().getCreatureStats(ptr).getActiveSpells().purgeEffect(
            ptr, ESM::MagicEffect::Levitate);
        isLevitationPurged = true;
    }
    else if ((isFlying || hasTcl) && !world->isFlying(ptr))
    {
        MWMechanics::CastSpell levitationCast(ptr, ptr);
        levitationCast.mHitPosition = ptr.getRefData().getPosition().asVec3();
        levitationCast.mAlwaysSucceed = true;
        levitationCast.cast(ESM::RefId::stringRefId("Levitate"));
        isLevitationPurged = false;
    }

    if (isJumping && !wasJumping)
    {
        world->setOnGround(ptr, false);
        wasJumping = true;
    }
    else if (wasJumping && !isJumping)
    {
        world->setOnGround(ptr, true);
        wasJumping = false;
    }

    MWMechanics::CreatureStats *ptrCreatureStats = &ptr.getClass().getCreatureStats(ptr);

    ptrCreatureStats->setDrawState(static_cast<MWMechanics::DrawState>(drawState));

    ptrCreatureStats->setMovementFlag(CreatureStats::Flag_Run, (movementFlags & CreatureStats::Flag_Run) != 0);
    ptrCreatureStats->setMovementFlag(CreatureStats::Flag_Sneak, (movementFlags & CreatureStats::Flag_Sneak) != 0);
    ptrCreatureStats->setMovementFlag(CreatureStats::Flag_ForceJump, (movementFlags & CreatureStats::Flag_ForceJump) != 0);
    ptrCreatureStats->setMovementFlag(CreatureStats::Flag_ForceMoveJump, (movementFlags & CreatureStats::Flag_ForceMoveJump) != 0);
}

void DedicatedPlayer::setAttributes()
{
    MWMechanics::CreatureStats *ptrCreatureStats = &ptr.getClass().getCreatureStats(ptr);
    MWMechanics::AttributeValue attributeValue;

    for (int i = 0; i < ESM::Attribute::Length; ++i)
    {
        ESM::StatState<float> attributeState;
        mwmp::RecordConvert::toEngine(creatureStats.mAttributes[i], attributeState);

        attributeValue.readState(attributeState);
        ptrCreatureStats->setAttribute(ESM::Attribute::indexToRefId(i), attributeValue);
    }
}

void DedicatedPlayer::setSkills()
{
    // Go no further if the player is disguised as a creature
    if (ptr.getType() != ESM::NPC::sRecordId) return;

    MWMechanics::NpcStats *ptrNpcStats = &ptr.getClass().getNpcStats(ptr);
    MWMechanics::SkillValue skillValue;

    for (int i = 0; i < ESM::Skill::Length; ++i)
    {
        ESM::StatState<float> skillState;
        mwmp::RecordConvert::toEngine(npcStats.mSkills[i], skillState);

        skillValue.readState(skillState);
        ptrNpcStats->setSkill(ESM::Skill::indexToRefId(i), skillValue);
    }
}

void DedicatedPlayer::setEquipment()
{
    // Go no further if the player is disguised as a creature
    if (!ptr.getClass().hasInventoryStore(ptr)) return;

    bool equippedSomething = false;

    MWWorld::InventoryStore& invStore = ptr.getClass().getInventoryStore(ptr);
    for (int slot = 0; slot < MWWorld::InventoryStore::Slots; ++slot)
    {
        MWWorld::ContainerStoreIterator it = invStore.getSlot(slot);

        const std::string &packetRefId = equipmentItems[slot].refId;
        std::string ptrItemId = "";
        bool equal = false;

        if (it != invStore.end())
        {
            ptrItemId = mwmp::RefIdCompat::toWire(it->getCellRef().getRefId());

            if (ptrItemId != packetRefId) // if other item is now equipped
            {
                MWWorld::ContainerStore &store = ptr.getClass().getContainerStore(ptr);

                // Remove the items that are no longer equipped, except for throwing weapons and ranged weapon ammo that
                // have just run out but still need to be kept briefly so they can be used in attacks about to be released
                bool shouldRemove = true;

                if (attack.type == mwmp::Attack::RANGED && packetRefId.empty() && !attack.pressed)
                {
                    if (slot == MWWorld::InventoryStore::Slot_CarriedRight && Misc::StringUtils::ciEqual(ptrItemId, attack.rangedWeaponId))
                        shouldRemove = false;
                    else if (slot == MWWorld::InventoryStore::Slot_Ammunition && Misc::StringUtils::ciEqual(ptrItemId, attack.rangedAmmoId))
                        shouldRemove = false;
                }
                
                if (shouldRemove)
                {
                    const ESM::RefId storedId = mwmp::RefIdCompat::fromWireCreate(ptrItemId);
                    store.remove(storedId, store.count(storedId));
                }
            }
            else
                equal = true;
        }

        if (packetRefId.empty() || equal)
            continue;

        const int count = equipmentItems[slot].count;
        ptr.getClass().getContainerStore(ptr).add(mwmp::RefIdCompat::fromWireCreate(packetRefId), count);
        // Equip items silently if this is the first time equipment is being set for this character
        equipItem(packetRefId, !hasReceivedInitialEquipment);
        equippedSomething = true;
    }

    // Only track the initial equipment as received if at least one item has been equipped
    if (equippedSomething)
        hasReceivedInitialEquipment = true;
}

void DedicatedPlayer::setShapeshift()
{
    MWBase::World* world = MWBase::Environment::get().getWorld();

    bool isNpc = false;

    if (reference)
        isNpc = ptr.getType() == ESM::NPC::sRecordId;

    if (creatureRefId != previousCreatureRefId || displayCreatureName != previousDisplayCreatureName)
    {
        if (!creatureRefId.empty() && RecordHelper::doesRecordIdExist<ESM::Creature>(creatureRefId))
        {
            deleteReference();

            const ESM::Creature* tmpCreature
                = world->getStore().get<ESM::Creature>().search(mwmp::RefIdCompat::fromWireCreate(creatureRefId));
            mwmp::RecordConvert::fromEngine(*tmpCreature, creature);
            creature.mScript = "";
            if (!displayCreatureName)
                creature.mName = npc.mName;
            LOG_APPEND(TimedLog::LOG_INFO, "- %s is disguised as %s", npc.mName.c_str(), creatureRefId.c_str());

            // Is this our first time creating a creature record id for this player? If so, keep it around
            // and reuse it
            ESM::Creature engineCreature;
            mwmp::RecordConvert::toEngine(creature, engineCreature);

            if (creatureRecordId.empty())
            {
                creature.mId = creatureRecordId
                    = mwmp::RefIdCompat::toWire(RecordHelper::createRecord(engineCreature)->mId);
                LOG_APPEND(TimedLog::LOG_INFO, "- Creating new creature record %s", creatureRecordId.c_str());
            }
            else
            {
                creature.mId = creatureRecordId;
                engineCreature.mId = mwmp::RefIdCompat::fromWireCreate(creatureRecordId);
                RecordHelper::overrideRecord(engineCreature);
            }

            LOG_APPEND(TimedLog::LOG_INFO, "- Creating reference for %s", creature.mId.c_str());
            createReference(creature.mId);
        }
        // This player was already a creature, but the new creature refId was empty or
        // invalid, so we'll turn this player into their NPC self again as a result
        else if (!isNpc)
        {
            if (reference)
            {
                deleteReference();
            }

            ESM::NPC engineNpcAgain;
            mwmp::RecordConvert::toEngine(npc, engineNpcAgain);
            RecordHelper::overrideRecord(engineNpcAgain);

            createReference(npc.mId);
            reloadPtr();
        }

        previousCreatureRefId = creatureRefId;
        previousDisplayCreatureName = displayCreatureName;
    }

    if (ptr.getType() == ESM::NPC::sRecordId)
    {
        MWBase::Environment::get().getMechanicsManager()->setWerewolf(ptr, isWerewolf);

        if (!isWerewolf)
            setEquipment();
    }

    MWBase::Environment::get().getWorld()->scaleObject(ptr, scale);
}

void DedicatedPlayer::setCell()
{
    // Prevent cell update when reference doesn't exist
    if (!reference) return;

    MWBase::World *world = MWBase::Environment::get().getWorld();

    LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Server says DedicatedPlayer %s moved to %s",
        npc.mName.c_str(), cell.getShortDescription().c_str());

    MWWorld::CellStore *cellStore = Main::get().getCellController()->getCellStore(cell);

    if (!cellStore)
    {
        LOG_APPEND(TimedLog::LOG_INFO, "%s", "- Cell doesn't exist on this client");
        world->disable(getPtr());
        return;
    }
    else
        world->enable(getPtr());

    // Make sure the Ptr's dynamic stats and anim flags are up-to-date, so it doesn't show up
    // knocked down or in a jump loop when it shouldn't
    setStatsDynamic();
    setAnimFlags();

    // Allow this player's reference to move across a cell now that a manual cell
    // update has been called
    setPtr(world->moveObject(ptr, cellStore, osg::Vec3f(position.pos[0], position.pos[1], position.pos[2])));

    // Remove the marker entirely if this player has moved to an interior that is inactive for us
    if (!cell.isExterior() && !Main::get().getCellController()->isActiveWorldCell(cell))
        removeMarker();
    // Otherwise, update their marker so the player shows up in the right cell on the world map
    else
    {
        enableMarker();
    }

    // If this player is now in a cell that we are the local authority over, we should send them all
    // NPC data in that cell
    if (Main::get().getCellController()->hasLocalAuthority(cell))
        Main::get().getCellController()->getCell(cell)->updateLocal(true);

    // If this player is a new player or is now in a region that we are the weather authority over,
    // or is a new player, we should send our latest weather data to the server
    if (world->getWeatherCreationState())
    {
        // MWWorld::Cell keeps mRegion private; getRegion() is the accessor, and RefId
        // equality already carries ciEqual's case-insensitivity.
        if (!hasFinishedInitialTeleportation
            || getPtr().getCell()->getCell()->getRegion() == world->getPlayerPtr().getCell()->getCell()->getRegion())
        {
            world->sendWeather();
        }
    }

    hasFinishedInitialTeleportation = true;
}

void DedicatedPlayer::playAnimation()
{
    MWBase::Environment::get().getMechanicsManager()->playAnimationGroup(getPtr(),
        animation.groupname, animation.mode, animation.count, animation.persist);
}

void DedicatedPlayer::playSpeech()
{
    MWBase::Environment::get().getSoundManager()->say(getPtr(), VFS::Path::Normalized(sound));

    MWBase::WindowManager *winMgr = MWBase::Environment::get().getWindowManager();
    // 0.51 reads subtitles straight from the settings index.
    if (Settings::gui().mSubtitles)
        winMgr->messageBox(MWBase::Environment::get().getDialogueManager()->getVoiceCaption(sound), MWGui::ShowInDialogueMode_Never);
}

void DedicatedPlayer::equipItem(std::string itemId, bool noSound)
{
    for (const auto& itemPtr : ptr.getClass().getInventoryStore(ptr))
    {
        if (mwmp::RefIdCompat::toWire(itemPtr.getCellRef().getRefId()) == itemId)
        {
            std::shared_ptr<MWWorld::Action> action = itemPtr.getClass().use(itemPtr);
            action->execute(ptr, noSound);
            break;
        }
    }
}

void DedicatedPlayer::die()
{
    MWMechanics::DynamicStat<float> health;
    creatureStats.mDead = true;

    ESM::StatState<float> healthState;
    mwmp::RecordConvert::toEngine(creatureStats.mDynamic[0], healthState);
    health.readState(healthState);
    health.setCurrent(0);
    health.writeState(healthState);
    mwmp::RecordConvert::fromEngine(healthState, creatureStats.mDynamic[0]);

    ptr.getClass().getCreatureStats(ptr).setHealth(health);
}

void DedicatedPlayer::resurrect()
{
    creatureStats.mDead = false;
    if (creatureStats.mDynamic[0].mMod < 1)
        creatureStats.mDynamic[0].mMod = 1;
    creatureStats.mDynamic[0].mCurrent = creatureStats.mDynamic[0].mMod;

    MWBase::Environment::get().getMechanicsManager()->resurrect(getPtr());

    MWMechanics::DynamicStat<float> health;

    ESM::StatState<float> healthState;
    mwmp::RecordConvert::toEngine(creatureStats.mDynamic[0], healthState);
    health.readState(healthState);

    getPtr().getClass().getCreatureStats(getPtr()).setHealth(health);
}

void DedicatedPlayer::addSpellsActive()
{
    MWMechanics::ActiveSpells& activeSpells = getPtr().getClass().getCreatureStats(getPtr()).getActiveSpells();

    for (const auto& activeSpell : spellsActiveChanges.activeSpells)
    {
        MWWorld::TimeStamp timestamp = MWWorld::TimeStamp(activeSpell.timestampHour, activeSpell.timestampDay);
        std::vector<ESM::ActiveEffect> effects;
        mwmp::RecordConvert::toEngine(activeSpell.params.mEffects, effects);
        MechanicsHelper::createSpellGfx(getPtr(), effects);

        // Don't do a check for a spell's existence, because active effects from potions need to be applied here too
        activeSpells.addSpell(MechanicsHelper::makeActiveSpellParams(activeSpell), timestamp, false);
    }
}

void DedicatedPlayer::removeSpellsActive()
{
    MWMechanics::ActiveSpells& activeSpells = getPtr().getClass().getCreatureStats(getPtr()).getActiveSpells();

    for (const auto& activeSpell : spellsActiveChanges.activeSpells)
    {
        // Remove stacking spells based on their timestamps
        if (activeSpell.isStackingSpell)
        {
            MWWorld::TimeStamp timestamp = MWWorld::TimeStamp(activeSpell.timestampHour, activeSpell.timestampDay);
            activeSpells.removeSpellByTimestamp(
                ptr, mwmp::RefIdCompat::fromWireCreate(activeSpell.id), timestamp);
        }
        else
        {
            // 0.51 renamed removeEffects() to say which id it matches on.
            activeSpells.removeEffectsBySourceSpellId(ptr, mwmp::RefIdCompat::fromWireCreate(activeSpell.id));
        }
    }
}

void DedicatedPlayer::setSpellsActive()
{
    MWMechanics::ActiveSpells& activeSpells = getPtr().getClass().getCreatureStats(getPtr()).getActiveSpells();
    // 0.51's clear() needs the owning actor so it can undo the effects.
    activeSpells.clear(getPtr());

    // Proceed by adding spells active
    addSpellsActive();
}

void DedicatedPlayer::updateMarker()
{
    if (!markerEnabled)
    {
        return;
    }

    GUIController* gui = Main::get().getGUIController();

    if (gui->mPlayerMarkers.contains(marker))
    {
        gui->mPlayerMarkers.deleteMarker(marker);
        marker = gui->createMarker(guid);
        gui->mPlayerMarkers.addMarker(marker);
    }
    else
    {
        gui->mPlayerMarkers.addMarker(marker, true);
    }
}

void DedicatedPlayer::enableMarker()
{
    markerEnabled = true;
    updateMarker();
}

void DedicatedPlayer::removeMarker()
{
    if (!markerEnabled)
        return;

    markerEnabled = false;
    GUIController* gui = Main::get().getGUIController();

    if (gui->mPlayerMarkers.contains(marker))
    {
        Main::get().getGUIController()->mPlayerMarkers.deleteMarker(marker);
    }
}

void DedicatedPlayer::createReference(const std::string& recId)
{
    MWBase::World *world = MWBase::Environment::get().getWorld();

    reference = new MWWorld::ManualRef(world->getStore(), mwmp::RefIdCompat::fromWireCreate(recId), 1);

    LOG_APPEND(TimedLog::LOG_INFO, "- Creating new reference pointer for %s", npc.mName.c_str());

    ESM::Position enginePosition;
    mwmp::RecordConvert::toEngine(position, enginePosition);

    ptr = world->placeObject(reference->getPtr(), Main::get().getCellController()->getCellStore(cell), enginePosition);

    ESM::CustomMarker mEditingMarker = Main::get().getGUIController()->createMarker(guid);
    marker = mEditingMarker;
    enableMarker();
}

void DedicatedPlayer::deleteReference()
{
    MWBase::World *world = MWBase::Environment::get().getWorld();

    LOG_APPEND(TimedLog::LOG_INFO, "- Deleting reference");
    world->deleteObject(ptr);
    delete reference;
    reference = nullptr;
}

MWWorld::Ptr DedicatedPlayer::getPtr()
{
    return ptr;
}

MWWorld::ManualRef *DedicatedPlayer::getRef()
{
    return reference;
}

void DedicatedPlayer::setPtr(const MWWorld::Ptr& newPtr)
{
    ptr = newPtr;
}

void DedicatedPlayer::reloadPtr()
{
    MWBase::World *world = MWBase::Environment::get().getWorld();
    world->disable(ptr);
    world->enable(ptr);
}

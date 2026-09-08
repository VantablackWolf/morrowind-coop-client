#include <components/openmw-mp/TimedLog.hpp>
#include <components/openmw-mp/Utils.hpp>

#include <components/misc/rng.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/mechanicsmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwmechanics/actorutil.hpp"
#include "../mwmechanics/creaturestats.hpp"
#include "../mwmechanics/combat.hpp"
#include "../mwmechanics/levelledlist.hpp"
#include "../mwmechanics/spellcasting.hpp"
#include "../mwmechanics/spellutil.hpp"

#include "../mwrender/animation.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/inventorystore.hpp"

#include <components/esm3/loadench.hpp>
#include <components/esm3/loadlevlist.hpp>
#include <components/esm3/loadstat.hpp>

#include <components/misc/resourcehelpers.hpp>

#include "../mwmechanics/damagesourcetype.hpp"

#include "MechanicsHelper.hpp"
#include "RecordConvertPlayer.hpp"
#include "RefIdCompat.hpp"
#include "Main.hpp"
#include "Networking.hpp"
#include "LocalPlayer.hpp"
#include "DedicatedPlayer.hpp"
#include "PlayerList.hpp"
#include "ObjectList.hpp"
#include "CellController.hpp"

using namespace mwmp;

osg::Vec3f MechanicsHelper::getLinearInterpolation(osg::Vec3f start, osg::Vec3f end, float percent)
{
    osg::Vec3f position(percent, percent, percent);

    return (start + osg::componentMultiply(position, (end - start)));
}

ESM::Position MechanicsHelper::getPositionFromVector(osg::Vec3f vector)
{
    ESM::Position position;
    position.pos[0] = vector.x();
    position.pos[1] = vector.y();
    position.pos[2] = vector.z();

    return position;
}

// Inspired by similar code in mwclass\creaturelevlist.cpp
//
// TODO: Add handling of scaling based on leveled list's assigned scale
void MechanicsHelper::spawnLeveledCreatures(MWWorld::CellStore* cellStore)
{
    mwmp::ObjectList *objectList = mwmp::Main::get().getNetworking()->getObjectList();
    objectList->reset();
    objectList->packetOrigin = mwmp::CLIENT_GAMEPLAY;

    int spawnCount = 0;

    const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
    auto& prng = MWBase::Environment::get().getWorld()->getPrng();

    /*
        0.51 no longer exposes the per-type CellRefList; forEachType() is the supported
        way in. It explicitly forbids adding to or removing from the cell while it runs,
        and that is exactly what spawning does, so the resolution is done in a first pass
        and the spawning in a second.
    */
    std::vector<std::pair<ESM::RefId, ESM::Position>> resolved;

    cellStore->forEachType<ESM::CreatureLevList>([&](const MWWorld::Ptr& ptr) {
        ESM::RefId id = MWMechanics::getLevelledItem(
            store.get<ESM::CreatureLevList>().find(ptr.getCellRef().getRefId()), true, prng);

        if (!id.empty())
            resolved.emplace_back(id, ptr.getCellRef().getPosition());

        return true;
    });

    for (const auto& [id, position] : resolved)
    {
        MWWorld::ManualRef manualRef(store, id);
        manualRef.getPtr().getCellRef().setPosition(position);
        MWWorld::Ptr placed = MWBase::Environment::get().getWorld()->placeObject(manualRef.getPtr(), cellStore,
                                                                                position);
        objectList->addObjectSpawn(placed);
        MWBase::Environment::get().getWorld()->deleteObject(placed);

        spawnCount++;
    }

    if (spawnCount > 0)
        objectList->sendObjectSpawn();
}

bool MechanicsHelper::isUsingRangedWeapon(const MWWorld::Ptr& ptr)
{
    if (ptr.getClass().hasInventoryStore(ptr))
    {
        MWWorld::InventoryStore &inventoryStore = ptr.getClass().getInventoryStore(ptr);
        MWWorld::ContainerStoreIterator weaponSlot = inventoryStore.getSlot(
            MWWorld::InventoryStore::Slot_CarriedRight);

        if (weaponSlot != inventoryStore.end() && weaponSlot->getType() == ESM::Weapon::sRecordId)
        {
            const ESM::Weapon* weaponRecord = weaponSlot->get<ESM::Weapon>()->mBase;

            if (weaponRecord->mData.mType >= ESM::Weapon::MarksmanBow)
                return true;
        }
    }

    return false;
}

Attack *MechanicsHelper::getLocalAttack(const MWWorld::Ptr& ptr)
{
    if (ptr == MWMechanics::getPlayer())
        return &mwmp::Main::get().getLocalPlayer()->attack;
    else if (mwmp::Main::get().getCellController()->isLocalActor(ptr))
        return &mwmp::Main::get().getCellController()->getLocalActor(ptr)->attack;

    return nullptr;
}

Attack *MechanicsHelper::getDedicatedAttack(const MWWorld::Ptr& ptr)
{
    if (mwmp::PlayerList::isDedicatedPlayer(ptr))
        return &mwmp::PlayerList::getPlayer(ptr)->attack;
    else if (mwmp::Main::get().getCellController()->isDedicatedActor(ptr))
        return &mwmp::Main::get().getCellController()->getDedicatedActor(ptr)->attack;

    return nullptr;
}

Cast *MechanicsHelper::getLocalCast(const MWWorld::Ptr& ptr)
{
    if (ptr == MWMechanics::getPlayer())
        return &mwmp::Main::get().getLocalPlayer()->cast;
    else if (mwmp::Main::get().getCellController()->isLocalActor(ptr))
        return &mwmp::Main::get().getCellController()->getLocalActor(ptr)->cast;

    return nullptr;
}

Cast *MechanicsHelper::getDedicatedCast(const MWWorld::Ptr& ptr)
{
    if (mwmp::PlayerList::isDedicatedPlayer(ptr))
        return &mwmp::PlayerList::getPlayer(ptr)->cast;
    else if (mwmp::Main::get().getCellController()->isDedicatedActor(ptr))
        return &mwmp::Main::get().getCellController()->getDedicatedActor(ptr)->cast;

    return nullptr;
}

MWWorld::Ptr MechanicsHelper::getPlayerPtr(const Target& target)
{
    if (target.guid == mwmp::Main::get().getLocalPlayer()->guid)
    {
        return MWMechanics::getPlayer();
    }
    else
    {
        mwmp::DedicatedPlayer* dedicatedPlayer = mwmp::PlayerList::getPlayer(target.guid);

        if (dedicatedPlayer != nullptr)
        {
            return dedicatedPlayer->getPtr();
        }
    }

    return nullptr;
}

ESM::RefNum MechanicsHelper::getActorRefNum(const mwmp::Target& target)
{
    MWWorld::Ptr targetPtr;

    if (target.isPlayer)
    {
        targetPtr = getPlayerPtr(target);
    }
    else
    {
        auto controller = mwmp::Main::get().getCellController();
        if (controller->isLocalActor(target.refNum, target.mpNum))
        {
            targetPtr = controller->getLocalActor(target.refNum, target.mpNum)->getPtr();
        }
        else if (controller->isDedicatedActor(target.refNum, target.mpNum))
        {
            targetPtr = controller->getDedicatedActor(target.refNum, target.mpNum)->getPtr();
        }
    }

    if (targetPtr.isEmpty())
        return {};

    return targetPtr.getCellRef().getRefNum();
}

mwmp::Item MechanicsHelper::getItem(const MWWorld::Ptr& itemPtr, int count)
{
    mwmp::Item item;

    if (itemPtr.getClass().isGold(itemPtr))
        item.refId = mwmp::RefIdCompat::toWire(MWWorld::ContainerStore::sGoldId);
    else
        item.refId = mwmp::RefIdCompat::toWire(itemPtr.getCellRef().getRefId());

    item.count = count;
    item.charge = itemPtr.getCellRef().getCharge();
    item.enchantmentCharge = itemPtr.getCellRef().getEnchantmentCharge();
    item.soul = mwmp::RefIdCompat::toWire(itemPtr.getCellRef().getSoul());

    return item;
}

mwmp::Target MechanicsHelper::getTarget(const MWWorld::Ptr& ptr)
{
    mwmp::Target target;
    clearTarget(target);

    if (!ptr.isEmpty())
    {
        if (ptr == MWMechanics::getPlayer())
        {
            target.isPlayer = true;
            target.guid = mwmp::Main::get().getLocalPlayer()->guid;
        }
        else if (mwmp::PlayerList::isDedicatedPlayer(ptr))
        {
            target.isPlayer = true;
            target.guid = mwmp::PlayerList::getPlayer(ptr)->guid;
        }
        else
        {
            MWWorld::CellRef *ptrRef = &ptr.getCellRef();

            if (ptrRef != nullptr)
            {
                target.isPlayer = false;
                target.refId = mwmp::RefIdCompat::toWire(ptrRef->getRefId());
                target.refNum = ptrRef->getRefNum().mIndex;
                target.mpNum = ptrRef->getMpNum();
                target.name = std::string(ptr.getClass().getName(ptr));
            }
        }
    }

    return target;
}

void MechanicsHelper::clearTarget(mwmp::Target& target)
{
    target.isPlayer = false;
    target.refId.clear();
    target.refNum = -1;
    target.mpNum = -1;

    target.name.clear();
}

bool MechanicsHelper::isEmptyTarget(const mwmp::Target& target)
{
    if (target.isPlayer == false && target.refId.empty())
        return true;

    return false;
}

void MechanicsHelper::assignAttackTarget(Attack* attack, const MWWorld::Ptr& target)
{
    if (target == MWMechanics::getPlayer())
    {
        attack->target.isPlayer = true;
        attack->target.guid = mwmp::Main::get().getLocalPlayer()->guid;
    }
    else if (mwmp::PlayerList::isDedicatedPlayer(target))
    {
        attack->target.isPlayer = true;
        attack->target.guid = mwmp::PlayerList::getPlayer(target)->guid;
    }
    else
    {
        MWWorld::CellRef *targetRef = &target.getCellRef();

        attack->target.isPlayer = false;
        attack->target.refId = mwmp::RefIdCompat::toWire(targetRef->getRefId());
        attack->target.refNum = targetRef->getRefNum().mIndex;
        attack->target.mpNum = targetRef->getMpNum();
    }
}

void MechanicsHelper::resetAttack(Attack* attack)
{
    attack->isHit = false;
    attack->success = false;
    attack->knockdown = false;
    attack->block = false;
    attack->applyWeaponEnchantment = false;
    attack->applyAmmoEnchantment = false;
    attack->hitPosition.pos[0] = attack->hitPosition.pos[1] = attack->hitPosition.pos[2] = 0;
    attack->target.guid = RakNet::RakNetGUID();
    attack->target.refId.clear();
    attack->target.refNum = 0;
    attack->target.mpNum = 0;
}

void MechanicsHelper::resetCast(Cast* cast)
{
    cast->isHit = false;
    cast->success = false;
    cast->target.guid = RakNet::RakNetGUID();
    cast->target.refId.clear();
    cast->target.refNum = 0;
    cast->target.mpNum = 0;
}

bool MechanicsHelper::getSpellSuccess(std::string spellId, const MWWorld::Ptr& caster)
{
    return Misc::Rng::roll0to99()
        < MWMechanics::getSpellSuccessChance(mwmp::RefIdCompat::fromWireCreate(spellId), caster, nullptr, true, false);
}

bool MechanicsHelper::isTeamMember(const MWWorld::Ptr& playerChecked, const MWWorld::Ptr& playerWithTeam)
{
    bool isTeamMember = false;
    bool playerCheckedIsLocal = playerChecked == MWMechanics::getPlayer();
    bool playerCheckedIsDedicated = !playerCheckedIsLocal ? mwmp::PlayerList::isDedicatedPlayer(playerChecked) : false;
    bool playerWithTeamIsLocal = !playerCheckedIsLocal ? playerWithTeam == MWMechanics::getPlayer() : false;
    bool playerWithTeamIsDedicated = !playerWithTeamIsLocal ? mwmp::PlayerList::isDedicatedPlayer(playerWithTeam) : false;

    if (playerCheckedIsLocal || playerCheckedIsDedicated)
    {
        if (playerWithTeamIsLocal || playerWithTeamIsDedicated)
        {
            RakNet::RakNetGUID playerCheckedGuid;

            if (playerCheckedIsLocal)
                playerCheckedGuid = mwmp::Main::get().getLocalPlayer()->guid;
            else
                playerCheckedGuid = PlayerList::getPlayer(playerChecked)->guid;

            if (playerWithTeamIsLocal)
                isTeamMember = Utils::vectorContains(mwmp::Main::get().getLocalPlayer()->alliedPlayers, playerCheckedGuid);
            else
                isTeamMember = Utils::vectorContains(PlayerList::getPlayer(playerWithTeam)->alliedPlayers, playerCheckedGuid);
        }
    }

    return isTeamMember;
}

void MechanicsHelper::processAttack(Attack attack, const MWWorld::Ptr& attacker)
{
    LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Processing attack from %s of type %i",
        std::string(attacker.getClass().getName(attacker)).c_str(), attack.type);

    LOG_APPEND(TimedLog::LOG_VERBOSE, "- pressed: %s", attack.pressed ? "true" : "false");

    if (!attack.pressed)
    {
        LOG_APPEND(TimedLog::LOG_VERBOSE, "- success: %s", attack.success ? "true" : "false");

        if (attack.success)
            LOG_APPEND(TimedLog::LOG_VERBOSE, "- damage: %f", attack.damage);
    }
    else
    {
        if (attack.type == attack.MELEE)
            LOG_APPEND(TimedLog::LOG_VERBOSE, "- animation: %s", attack.attackAnimation.c_str());
    }

    MWBase::Environment::get().getMechanicsManager()->setAttackingOrSpell(attacker, attack.pressed);

    MWWorld::Ptr victim;

    if (attack.target.isPlayer)
    {
        if (attack.target.guid == mwmp::Main::get().getLocalPlayer()->guid)
            victim = MWMechanics::getPlayer();
        else if (PlayerList::getPlayer(attack.target.guid) != nullptr)
            victim = PlayerList::getPlayer(attack.target.guid)->getPtr();
    }
    else
    {
        auto controller = mwmp::Main::get().getCellController();
        if (controller->isLocalActor(attack.target.refNum, attack.target.mpNum))
            victim = controller->getLocalActor(attack.target.refNum, attack.target.mpNum)->getPtr();
        else if (controller->isDedicatedActor(attack.target.refNum, attack.target.mpNum))
            victim = controller->getDedicatedActor(attack.target.refNum, attack.target.mpNum)->getPtr();
    }

    if (attack.isHit)
    {
        bool isRanged = attack.type == attack.RANGED;

        MWWorld::Ptr weaponPtr;
        MWWorld::Ptr ammoPtr;

        bool usedTempRangedWeapon = false;
        bool usedTempRangedAmmo = false;

        // Get the attacker's current weapon
        //
        // Note: if using hand-to-hand, the weapon is equal to inv.end()
        if (attacker.getClass().hasInventoryStore(attacker))
        {
            MWWorld::InventoryStore &inventoryStore = attacker.getClass().getInventoryStore(attacker);
            MWWorld::ContainerStoreIterator weaponSlot = inventoryStore.getSlot(
                MWWorld::InventoryStore::Slot_CarriedRight);

            weaponPtr = weaponSlot != inventoryStore.end() ? *weaponSlot : MWWorld::Ptr();

            // Is the currently selected weapon different from the one recorded for this ranged attack?
            // If so, try to find the correct one in the attacker's inventory and use it here. If it
            // no longer exists, add it back temporarily.
            if (isRanged)
            {
                if (weaponPtr.isEmpty() || mwmp::RefIdCompat::toWire(weaponPtr.getCellRef().getRefId()) != attack.rangedWeaponId)
                {
                    weaponPtr = inventoryStore.search(mwmp::RefIdCompat::fromWireCreate(attack.rangedWeaponId));

                    if (weaponPtr.isEmpty())
                    {
                        weaponPtr = *attacker.getClass().getContainerStore(attacker).add(mwmp::RefIdCompat::fromWireCreate(attack.rangedWeaponId), 1);
                        usedTempRangedWeapon = true;
                    }
                }

                if (!attack.rangedAmmoId.empty())
                {
                    MWWorld::ContainerStoreIterator ammoSlot = inventoryStore.getSlot(
                        MWWorld::InventoryStore::Slot_Ammunition);
                    ammoPtr = ammoSlot != inventoryStore.end() ? *ammoSlot : MWWorld::Ptr();

                    if (ammoPtr.isEmpty() || mwmp::RefIdCompat::toWire(ammoPtr.getCellRef().getRefId()) != attack.rangedAmmoId)
                    {
                        ammoPtr = inventoryStore.search(mwmp::RefIdCompat::fromWireCreate(attack.rangedAmmoId));

                        if (ammoPtr.isEmpty())
                        {
                            ammoPtr = *attacker.getClass().getContainerStore(attacker).add(mwmp::RefIdCompat::fromWireCreate(attack.rangedAmmoId), 1);
                            usedTempRangedAmmo = true;
                        }
                    }
                }
            }

            if (!weaponPtr.isEmpty() && weaponPtr.getType() != ESM::Weapon::sRecordId)
                weaponPtr = MWWorld::Ptr();
        }

        if (!weaponPtr.isEmpty())
        {
            LOG_APPEND(TimedLog::LOG_VERBOSE, "- weapon: %s\n- isRanged: %s\n- applyWeaponEnchantment: %s\n- applyAmmoEnchantment: %s",
                weaponPtr.getCellRef().getRefId().getRefIdString().c_str(), isRanged ? "true" : "false", attack.applyWeaponEnchantment ? "true" : "false",
                attack.applyAmmoEnchantment ? "true" : "false");

            if (attack.applyWeaponEnchantment)
            {
                MWMechanics::CastSpell cast(attacker, victim, isRanged);
                cast.mHitPosition = osg::Vec3f(attack.hitPosition.pos[0], attack.hitPosition.pos[1], attack.hitPosition.pos[2]);
                cast.cast(weaponPtr, false);
            }

            if (isRanged && !ammoPtr.isEmpty() && attack.applyAmmoEnchantment)
            {
                MWMechanics::CastSpell cast(attacker, victim, isRanged);
                cast.mHitPosition = osg::Vec3f(attack.hitPosition.pos[0], attack.hitPosition.pos[1], attack.hitPosition.pos[2]);
                cast.cast(ammoPtr, false);
            }
        }

        if (victim.mRef != nullptr)
        {
            bool isHealthDamage = true;

            if (weaponPtr.isEmpty())
            {
                if (attacker.getClass().isBipedal(attacker))
                {
                    MWMechanics::CreatureStats &victimStats = victim.getClass().getCreatureStats(victim);
                    isHealthDamage = victimStats.isParalyzed() || victimStats.getKnockedDown();
                }
            }

            if (!isRanged)
                MWMechanics::blockMeleeAttack(attacker, victim, weaponPtr, attack.damage, 1);

            /*
                0.51 rebuilt onHit around a per-stat damage map, so the isHealthDamage flag
                selects a key rather than a branch, the weapon is named by RefId rather
                than passed as a Ptr, and the source type is explicit.

                The hit position argument is gone -- upstream dropped it from onHit
                entirely, so there is nothing to pass it to. It is still used just above
                for the weapon enchantment, which is where it actually mattered.
            */
            std::map<std::string, float> damages;
            damages[isHealthDamage ? "health" : "fatigue"] = attack.damage;

            victim.getClass().onHit(victim, damages,
                weaponPtr.isEmpty() ? ESM::RefId() : weaponPtr.getCellRef().getRefId(), attacker, attack.success,
                isRanged ? MWMechanics::DamageSourceType::Ranged : MWMechanics::DamageSourceType::Melee);
        }

        // Remove temporary items that may have been added above for ranged attacks
        if (isRanged && attacker.getClass().hasInventoryStore(attacker))
        {
            MWWorld::InventoryStore &inventoryStore = attacker.getClass().getInventoryStore(attacker);

            if (usedTempRangedWeapon)
                inventoryStore.remove(weaponPtr, 1);
            
            if (usedTempRangedAmmo)
                inventoryStore.remove(ammoPtr, 1);
        }
    }
}

void MechanicsHelper::processCast(Cast cast, const MWWorld::Ptr& caster)
{
    LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Processing cast from %s of type %i",
        std::string(caster.getClass().getName(caster)).c_str(), cast.type);

    LOG_APPEND(TimedLog::LOG_VERBOSE, "- pressed: %s", cast.pressed ? "true" : "false");

    if (!cast.pressed)
    {
        LOG_APPEND(TimedLog::LOG_VERBOSE, "- success: %s", cast.success ? "true" : "false");
    }

    MWBase::Environment::get().getMechanicsManager()->setAttackingOrSpell(caster, cast.pressed);
    MWMechanics::CreatureStats &casterStats = caster.getClass().getCreatureStats(caster);

    MWWorld::Ptr victim;

    if (cast.target.isPlayer)
    {
        if (cast.target.guid == mwmp::Main::get().getLocalPlayer()->guid)
            victim = MWMechanics::getPlayer();
        else if (PlayerList::getPlayer(cast.target.guid) != nullptr)
            victim = PlayerList::getPlayer(cast.target.guid)->getPtr();
    }
    else
    {
        auto controller = mwmp::Main::get().getCellController();
        if (controller->isLocalActor(cast.target.refNum, cast.target.mpNum))
            victim = controller->getLocalActor(cast.target.refNum, cast.target.mpNum)->getPtr();
        else if (controller->isDedicatedActor(cast.target.refNum, cast.target.mpNum))
            victim = controller->getDedicatedActor(cast.target.refNum, cast.target.mpNum)->getPtr();
    }

    if (cast.type == cast.REGULAR)
    {
        casterStats.getSpells().setSelectedSpell(mwmp::RefIdCompat::fromWireCreate(cast.spellId));

        if (cast.success)
            MWBase::Environment::get().getWorld()->castSpell(caster);

        LOG_APPEND(TimedLog::LOG_VERBOSE, "- spellId: %s", cast.spellId.c_str());
    }
    else if (cast.type == cast.ITEM)
    {
        casterStats.getSpells().setSelectedSpell(ESM::RefId());

        MWWorld::InventoryStore& inventoryStore = caster.getClass().getInventoryStore(caster);

        MWWorld::ContainerStoreIterator it = inventoryStore.begin();
        for (; it != inventoryStore.end(); ++it)
        {
            if (mwmp::RefIdCompat::toWire(it->getCellRef().getRefId()) == cast.itemId)
                break;
        }

        // Add the item if it's missing
        if (it == inventoryStore.end())
            it = caster.getClass().getContainerStore(caster).add(mwmp::RefIdCompat::fromWireCreate(cast.itemId), 1);

        inventoryStore.setSelectedEnchantItem(it);
        LOG_APPEND(TimedLog::LOG_VERBOSE, "- itemId: %s", cast.itemId.c_str());
        MWBase::Environment::get().getWorld()->castSpell(caster);
        inventoryStore.setSelectedEnchantItem(inventoryStore.end());
    }
}

void MechanicsHelper::createSpellGfx(const MWWorld::Ptr& targetPtr, const std::vector<ESM::ActiveEffect>& mEffects)
{
    for (auto&& effect : mEffects)
    {
        const ESM::MagicEffect* magicEffect = MWBase::Environment::get().getWorld()->getStore().get<ESM::MagicEffect>().find(effect.mEffectId);

        const ESM::Static* castStatic;
        if (!magicEffect->mHit.empty())
            castStatic = MWBase::Environment::get().getWorld()->getStore().get<ESM::Static>().find(magicEffect->mHit);
        else
            castStatic = MWBase::Environment::get().getWorld()->getStore().get<ESM::Static>().find(ESM::RefId::stringRefId("VFX_DefaultHit"));

        bool loop = (magicEffect->mData.mFlags & ESM::MagicEffect::ContinuousVfx) != 0;
        // Note: in case of non actor, a free effect should be fine as well
        MWRender::Animation* anim = MWBase::Environment::get().getWorld()->getAnimation(targetPtr);
        if (anim && !castStatic->mModel.empty())
        {
            // 0.51 identifies the effect by RefId string and wants a corrected mesh path.
            anim->addEffect(Misc::ResourceHelpers::correctMeshPath(VFS::Path::Normalized(castStatic->mModel)).value(),
                magicEffect->mId.getRefIdString(), loop, {}, magicEffect->mParticle);
        }
    }
}

bool MechanicsHelper::isStackingSpell(const std::string& id)
{
    return MWBase::Environment::get().getWorld()->getStore().get<ESM::Spell>().search(
               mwmp::RefIdCompat::fromWireCreate(id))
        == nullptr;
}

bool MechanicsHelper::doesEffectListContainEffect(const ESM::EffectList& effectList, const ESM::RefId& effectId,
    const ESM::RefId& attributeId, const ESM::RefId& skillId)
{
    for (const auto &effect : effectList.mList)
    {
        if (effect.mData.mEffectID == effectId)
        {
            if (attributeId.empty() || effect.mData.mAttribute == attributeId)
            {
                if (skillId.empty() || effect.mData.mSkill == skillId)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void MechanicsHelper::unequipItemsByEffect(const MWWorld::Ptr& ptr, short enchantmentType, const ESM::RefId& effectId,
    const ESM::RefId& attributeId, const ESM::RefId& skillId)
{
    MWBase::World *world = MWBase::Environment::get().getWorld();
    MWWorld::InventoryStore &ptrInventory = ptr.getClass().getInventoryStore(ptr);

    for (int slot = 0; slot < MWWorld::InventoryStore::Slots; slot++)
    {
        if (ptrInventory.getSlot(slot) != ptrInventory.end())
        {
            MWWorld::ConstContainerStoreIterator itemIterator = ptrInventory.getSlot(slot);
            const ESM::RefId& enchantmentName = itemIterator->getClass().getEnchantment(*itemIterator);

            if (!enchantmentName.empty())
            {
                const ESM::Enchantment* enchantment = world->getStore().get<ESM::Enchantment>().find(enchantmentName);

                if (enchantment->mData.mType == enchantmentType && doesEffectListContainEffect(enchantment->mEffects, effectId, attributeId, skillId))
                    ptrInventory.unequipSlot(slot);
            }
        }
    }
}

MWWorld::Ptr MechanicsHelper::getItemPtrFromStore(const mwmp::Item& item, MWWorld::ContainerStore& store)
{
    MWWorld::Ptr closestPtr;

    for (MWWorld::ContainerStoreIterator storeIterator = store.begin(); storeIterator != store.end(); ++storeIterator)
    {
        // Enchantment charges are often in the process of refilling themselves, so don't check for them here
        if (item.refId == mwmp::RefIdCompat::toWire(storeIterator->getCellRef().getRefId()) &&
            item.count == storeIterator->getCellRef().getCount() &&
            item.charge == storeIterator->getCellRef().getCharge() &&
            item.soul == mwmp::RefIdCompat::toWire(storeIterator->getCellRef().getSoul()))
        {
            // If we have no closestPtr, set it to the Ptr corresponding to this storeIterator; otherwise, make
            // sure the storeIterator's enchantmentCharge is closer to our goal than that of the previous closestPtr
            if (closestPtr.isEmpty() || abs(storeIterator->getCellRef().getEnchantmentCharge() - item.enchantmentCharge) <
                abs(closestPtr.getCellRef().getEnchantmentCharge() - item.enchantmentCharge))
            {
                closestPtr = *storeIterator;
            }
        }
    }

    return closestPtr;
}

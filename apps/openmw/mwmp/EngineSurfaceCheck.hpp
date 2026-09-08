#ifndef OPENMW_MWMP_ENGINESURFACECHECK_H
#define OPENMW_MWMP_ENGINESURFACECHECK_H

/*
    Forward-compatibility canary for the TES3MP record mirrors.

    THE PROBLEM THIS SOLVES

    components/openmw-mp/Base/records/Records.hpp mirrors OpenMW record fields
    using the 0.47 types that define the TES3MP wire format. RecordConvert.hpp
    translates between those mirrors and the live engine structs.

    When a future OpenMW release RENAMES or REMOVES a field, RecordConvert
    stops compiling and you find out immediately. That case is safe.

    The dangerous case is a field whose type or meaning changes while still
    compiling -- int32_t widened to int64_t, a plain id promoted to ESM::RefId,
    a raw array swapped for std::array, a count changing units. Those convert
    silently and corrupt data on the wire rather than failing the build.

    Every assertion below pins one engine field to the type this port was
    written against. If OpenMW changes it, the build stops here with a message
    naming the field, instead of shipping a silent protocol break.

    WHEN AN ASSERTION FIRES

    Do NOT simply update the type to make it compile. Work out what changed:

      1. If the type changed but the MEANING did not (e.g. int -> int32_t),
         update the assertion and move on.
      2. If it became an ESM::RefId, add a RefIdCompat conversion in
         RecordConvert.hpp -- do NOT change the mirror, since the mirror
         defines the wire format and changing it breaks every existing server.
      3. If the meaning changed (different units, different semantics), the
         converter needs real translation logic, and the change likely needs a
         protocol version bump.

    Keeping the mirrors fixed and absorbing engine churn here is what makes
    the dedicated server survive OpenMW upgrades untouched.
*/

#include <type_traits>

#include "RecordConvert.hpp"

namespace mwmp
{
    namespace EngineSurfaceCheck
    {
    // Spell
    static_assert(std::is_same_v<decltype(ESM::Spell{}.mId), ESM::RefId>,
        "Spell.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Spell{}.mName), std::string>,
        "Spell.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Spell{}.mData.mType), int32_t>,
        "Spell.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Spell{}.mData.mCost), int32_t>,
        "Spell.mData.mCost changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Spell{}.mData.mFlags), int32_t>,
        "Spell.mData.mFlags changed type in OpenMW; review RecordConvert before updating this line");

    // Potion
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mId), ESM::RefId>,
        "Potion.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mName), std::string>,
        "Potion.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mModel), std::string>,
        "Potion.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mIcon), std::string>,
        "Potion.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mData.mWeight), float>,
        "Potion.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mData.mValue), int32_t>,
        "Potion.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mData.mFlags), int32_t>,
        "Potion.mData.mAutoCalc changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Potion{}.mScript), ESM::RefId>,
        "Potion.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Enchantment
    static_assert(std::is_same_v<decltype(ESM::Enchantment{}.mId), ESM::RefId>,
        "Enchantment.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Enchantment{}.mData.mType), int32_t>,
        "Enchantment.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Enchantment{}.mData.mCost), int32_t>,
        "Enchantment.mData.mCost changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Enchantment{}.mData.mCharge), int32_t>,
        "Enchantment.mData.mCharge changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Enchantment{}.mData.mFlags), int32_t>,
        "Enchantment.mData.mFlags changed type in OpenMW; review RecordConvert before updating this line");

    // Armor
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mId), ESM::RefId>,
        "Armor.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mName), std::string>,
        "Armor.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mModel), std::string>,
        "Armor.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mIcon), std::string>,
        "Armor.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mData.mType), int32_t>,
        "Armor.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mData.mWeight), float>,
        "Armor.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mData.mValue), int32_t>,
        "Armor.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mData.mHealth), int32_t>,
        "Armor.mData.mHealth changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mData.mArmor), int32_t>,
        "Armor.mData.mArmor changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mData.mEnchant), int32_t>,
        "Armor.mData.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mEnchant), ESM::RefId>,
        "Armor.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Armor{}.mScript), ESM::RefId>,
        "Armor.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Book
    static_assert(std::is_same_v<decltype(ESM::Book{}.mId), ESM::RefId>,
        "Book.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mName), std::string>,
        "Book.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mModel), std::string>,
        "Book.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mIcon), std::string>,
        "Book.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mText), std::string>,
        "Book.mText changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mData.mWeight), float>,
        "Book.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mData.mValue), int32_t>,
        "Book.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mData.mIsScroll), int32_t>,
        "Book.mData.mIsScroll changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mData.mSkillId), int32_t>,
        "Book.mData.mSkillId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mData.mEnchant), int32_t>,
        "Book.mData.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mEnchant), ESM::RefId>,
        "Book.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Book{}.mScript), ESM::RefId>,
        "Book.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Clothing
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mId), ESM::RefId>,
        "Clothing.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mName), std::string>,
        "Clothing.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mModel), std::string>,
        "Clothing.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mIcon), std::string>,
        "Clothing.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mData.mType), int32_t>,
        "Clothing.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mData.mWeight), float>,
        "Clothing.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mData.mValue), uint16_t>,
        "Clothing.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mData.mEnchant), uint16_t>,
        "Clothing.mData.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mEnchant), ESM::RefId>,
        "Clothing.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Clothing{}.mScript), ESM::RefId>,
        "Clothing.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Miscellaneous
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mId), ESM::RefId>,
        "Miscellaneous.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mName), std::string>,
        "Miscellaneous.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mModel), std::string>,
        "Miscellaneous.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mIcon), std::string>,
        "Miscellaneous.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mData.mWeight), float>,
        "Miscellaneous.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mData.mValue), int32_t>,
        "Miscellaneous.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mData.mFlags), int32_t>,
        "Miscellaneous.mData.mIsKey changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Miscellaneous{}.mScript), ESM::RefId>,
        "Miscellaneous.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Weapon
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mId), ESM::RefId>,
        "Weapon.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mName), std::string>,
        "Weapon.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mModel), std::string>,
        "Weapon.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mIcon), std::string>,
        "Weapon.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mType), int16_t>,
        "Weapon.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mWeight), float>,
        "Weapon.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mValue), int32_t>,
        "Weapon.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mHealth), uint16_t>,
        "Weapon.mData.mHealth changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mSpeed), float>,
        "Weapon.mData.mSpeed changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mReach), float>,
        "Weapon.mData.mReach changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mChop), std::array<unsigned char, 2>>,
        "Weapon.mData.mChop changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mSlash), std::array<unsigned char, 2>>,
        "Weapon.mData.mSlash changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mThrust), std::array<unsigned char, 2>>,
        "Weapon.mData.mThrust changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mFlags), int32_t>,
        "Weapon.mData.mFlags changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mData.mEnchant), uint16_t>,
        "Weapon.mData.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mEnchant), ESM::RefId>,
        "Weapon.mEnchant changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Weapon{}.mScript), ESM::RefId>,
        "Weapon.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Activator
    static_assert(std::is_same_v<decltype(ESM::Activator{}.mId), ESM::RefId>,
        "Activator.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Activator{}.mName), std::string>,
        "Activator.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Activator{}.mModel), std::string>,
        "Activator.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Activator{}.mScript), ESM::RefId>,
        "Activator.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Apparatus
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mId), ESM::RefId>,
        "Apparatus.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mName), std::string>,
        "Apparatus.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mModel), std::string>,
        "Apparatus.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mIcon), std::string>,
        "Apparatus.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mData.mType), int32_t>,
        "Apparatus.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mData.mWeight), float>,
        "Apparatus.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mData.mValue), int32_t>,
        "Apparatus.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mData.mQuality), float>,
        "Apparatus.mData.mQuality changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Apparatus{}.mScript), ESM::RefId>,
        "Apparatus.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // BodyPart
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mId), ESM::RefId>,
        "BodyPart.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mModel), std::string>,
        "BodyPart.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mRace), ESM::RefId>,
        "BodyPart.mRace changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mData.mType), unsigned char>,
        "BodyPart.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mData.mPart), unsigned char>,
        "BodyPart.mData.mPart changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mData.mVampire), unsigned char>,
        "BodyPart.mData.mVampire changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::BodyPart{}.mData.mFlags), unsigned char>,
        "BodyPart.mData.mFlags changed type in OpenMW; review RecordConvert before updating this line");

    // Cell
    static_assert(std::is_same_v<decltype(ESM::Cell{}.mName), std::string>,
        "Cell.mName changed type in OpenMW; review RecordConvert before updating this line");

    // Container
    static_assert(std::is_same_v<decltype(ESM::Container{}.mId), ESM::RefId>,
        "Container.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Container{}.mName), std::string>,
        "Container.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Container{}.mModel), std::string>,
        "Container.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Container{}.mWeight), float>,
        "Container.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Container{}.mFlags), int32_t>,
        "Container.mFlags changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Container{}.mScript), ESM::RefId>,
        "Container.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Creature
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mId), ESM::RefId>,
        "Creature.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mName), std::string>,
        "Creature.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mModel), std::string>,
        "Creature.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mScale), float>,
        "Creature.mScale changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mBloodType), int32_t>,
        "Creature.mBloodType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mType), int32_t>,
        "Creature.mData.mType changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mLevel), int32_t>,
        "Creature.mData.mLevel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mHealth), int32_t>,
        "Creature.mData.mHealth changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mMana), int32_t>,
        "Creature.mData.mMana changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mFatigue), int32_t>,
        "Creature.mData.mFatigue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mSoul), int32_t>,
        "Creature.mData.mSoul changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mData.mAttack), int32_t[6]>,
        "Creature.mData.mAttack changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mFlags), unsigned char>,
        "Creature.mFlags changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Creature{}.mScript), ESM::RefId>,
        "Creature.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Door
    static_assert(std::is_same_v<decltype(ESM::Door{}.mId), ESM::RefId>,
        "Door.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Door{}.mName), std::string>,
        "Door.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Door{}.mModel), std::string>,
        "Door.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Door{}.mOpenSound), ESM::RefId>,
        "Door.mOpenSound changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Door{}.mCloseSound), ESM::RefId>,
        "Door.mCloseSound changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Door{}.mScript), ESM::RefId>,
        "Door.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // GameSetting
    static_assert(std::is_same_v<decltype(ESM::GameSetting{}.mId), ESM::RefId>,
        "GameSetting.mId changed type in OpenMW; review RecordConvert before updating this line");

    // Ingredient
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mId), ESM::RefId>,
        "Ingredient.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mName), std::string>,
        "Ingredient.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mModel), std::string>,
        "Ingredient.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mIcon), std::string>,
        "Ingredient.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mData.mWeight), float>,
        "Ingredient.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mData.mValue), int32_t>,
        "Ingredient.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mData.mEffectID), ESM::RefId[4]>,
        "Ingredient.mData.mEffectID changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mData.mAttributes), ESM::RefId[4]>,
        "Ingredient.mData.mAttributes changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mData.mSkills), ESM::RefId[4]>,
        "Ingredient.mData.mSkills changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Ingredient{}.mScript), ESM::RefId>,
        "Ingredient.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Light
    static_assert(std::is_same_v<decltype(ESM::Light{}.mId), ESM::RefId>,
        "Light.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mName), std::string>,
        "Light.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mModel), std::string>,
        "Light.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mIcon), std::string>,
        "Light.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mSound), ESM::RefId>,
        "Light.mSound changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mData.mWeight), float>,
        "Light.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mData.mValue), int32_t>,
        "Light.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mData.mTime), int32_t>,
        "Light.mData.mTime changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mData.mRadius), int32_t>,
        "Light.mData.mRadius changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mData.mColor), uint32_t>,
        "Light.mData.mColor changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mData.mFlags), int32_t>,
        "Light.mData.mFlags changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Light{}.mScript), ESM::RefId>,
        "Light.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Lockpick
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mId), ESM::RefId>,
        "Lockpick.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mName), std::string>,
        "Lockpick.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mModel), std::string>,
        "Lockpick.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mIcon), std::string>,
        "Lockpick.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mData.mWeight), float>,
        "Lockpick.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mData.mValue), int32_t>,
        "Lockpick.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mData.mQuality), float>,
        "Lockpick.mData.mQuality changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mData.mUses), int32_t>,
        "Lockpick.mData.mUses changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Lockpick{}.mScript), ESM::RefId>,
        "Lockpick.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // NPC
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mId), ESM::RefId>,
        "NPC.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mName), std::string>,
        "NPC.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mFlags), unsigned char>,
        "NPC.mFlags changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mRace), ESM::RefId>,
        "NPC.mRace changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mModel), std::string>,
        "NPC.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mHair), ESM::RefId>,
        "NPC.mHair changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mHead), ESM::RefId>,
        "NPC.mHead changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mClass), ESM::RefId>,
        "NPC.mClass changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mFaction), ESM::RefId>,
        "NPC.mFaction changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mScript), ESM::RefId>,
        "NPC.mScript changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::NPC{}.mNpdtType), unsigned char>,
        "NPC.mNpdtType changed type in OpenMW; review RecordConvert before updating this line");

    // Probe
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mId), ESM::RefId>,
        "Probe.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mName), std::string>,
        "Probe.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mModel), std::string>,
        "Probe.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mIcon), std::string>,
        "Probe.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mData.mWeight), float>,
        "Probe.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mData.mValue), int32_t>,
        "Probe.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mData.mQuality), float>,
        "Probe.mData.mQuality changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mData.mUses), int32_t>,
        "Probe.mData.mUses changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Probe{}.mScript), ESM::RefId>,
        "Probe.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Repair
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mId), ESM::RefId>,
        "Repair.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mName), std::string>,
        "Repair.mName changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mModel), std::string>,
        "Repair.mModel changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mIcon), std::string>,
        "Repair.mIcon changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mData.mWeight), float>,
        "Repair.mData.mWeight changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mData.mValue), int32_t>,
        "Repair.mData.mValue changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mData.mQuality), float>,
        "Repair.mData.mQuality changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mData.mUses), int32_t>,
        "Repair.mData.mUses changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Repair{}.mScript), ESM::RefId>,
        "Repair.mScript changed type in OpenMW; review RecordConvert before updating this line");

    // Script
    static_assert(std::is_same_v<decltype(ESM::Script{}.mId), ESM::RefId>,
        "Script.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Script{}.mScriptText), std::string>,
        "Script.mScriptText changed type in OpenMW; review RecordConvert before updating this line");

    // Static
    static_assert(std::is_same_v<decltype(ESM::Static{}.mId), ESM::RefId>,
        "Static.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Static{}.mModel), std::string>,
        "Static.mModel changed type in OpenMW; review RecordConvert before updating this line");

    // Sound
    static_assert(std::is_same_v<decltype(ESM::Sound{}.mId), ESM::RefId>,
        "Sound.mId changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Sound{}.mSound), std::string>,
        "Sound.mSound changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Sound{}.mData.mVolume), unsigned char>,
        "Sound.mData.mVolume changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Sound{}.mData.mMinRange), unsigned char>,
        "Sound.mData.mMinRange changed type in OpenMW; review RecordConvert before updating this line");
    static_assert(std::is_same_v<decltype(ESM::Sound{}.mData.mMaxRange), unsigned char>,
        "Sound.mData.mMaxRange changed type in OpenMW; review RecordConvert before updating this line");
    }
}

#endif

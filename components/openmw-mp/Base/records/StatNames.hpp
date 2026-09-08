#ifndef OPENMW_MP_RECORDS_STATNAMES_H
#define OPENMW_MP_RECORDS_STATNAMES_H

/*
    Canonical attribute and skill names for the TES3MP server script API.

    OpenMW 0.47 exposed these as ESM::Attribute::sAttributeNames and
    ESM::Skill::sSkillNames. Both were REMOVED in 0.51, where names are
    reached through RefIds instead -- ESM::Skill::indexToRefId(5) rather than
    a name table.

    The server cannot follow that change for two reasons:

      1. It must not depend on components/esm at all; that is the entire point
         of the mirror layer.
      2. These strings are the server's Lua-facing API. GetAttributeId("Luck")
         and GetSkillId("Longblade") are called by every existing server
         script, and 0.51's RefId spelling is lowercase ("luck", "longblade"),
         so switching to it would silently change the API surface.

    Keeping the 0.47 spellings here, compared case-insensitively exactly as
    TES3MP 0.8.1 did, means existing scripts keep working unchanged whichever
    case they used.

    EngineSurfaceCheck.hpp pins these against the engine's own RefId spellings,
    so if OpenMW ever renames a skill or attribute, the build fails rather than
    the two quietly diverging.
*/

#include <string_view>

namespace mwmp
{
    namespace records
    {
        inline constexpr std::string_view sAttributeNames[] = {
            "Strength",
            "Intelligence",
            "Willpower",
            "Agility",
            "Speed",
            "Endurance",
            "Personality",
            "Luck",
        };

        inline constexpr std::string_view sSkillNames[] = {
            "Block",
            "Armorer",
            "Mediumarmor",
            "Heavyarmor",
            "Bluntweapon",
            "Longblade",
            "Axe",
            "Spear",
            "Athletics",
            "Enchant",
            "Destruction",
            "Alteration",
            "Illusion",
            "Conjuration",
            "Mysticism",
            "Restoration",
            "Alchemy",
            "Unarmored",
            "Security",
            "Sneak",
            "Acrobatics",
            "Lightarmor",
            "Shortblade",
            "Marksman",
            "Mercantile",
            "Speechcraft",
            "Handtohand",
        };
    }
}

#endif

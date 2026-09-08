// Compiled WITHOUT the OpenMW source tree or vcpkg on the include path.
// If this builds, the protocol layer genuinely has no engine dependency.
#include "records/Records.hpp"
#include "records/PlayerState.hpp"
#include "records/StatNames.hpp"
#include "records/Common.hpp"
int main()
{
    mwmp::records::Armor a{}; a.mId = "test";
    mwmp::records::NPC n{}; n.mRace = "dark elf";
    mwmp::records::ActiveEffect e{}; e.mArg = -1;
    return (int)(a.mId.size() + n.mRace.size() + mwmp::records::sSkillNames[0].size() + e.mArg);
}

#include "difficultyscaling.hpp"

#include <components/settings/values.hpp>

/*
    Start of tes3mp addition

    Include additional headers for multiplayer purposes
*/
#include "../mwmp/Main.hpp"
#include "../mwmp/LocalPlayer.hpp"
/*
    End of tes3mp addition
*/
#include "../mwbase/environment.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/ptr.hpp"

#include "actorutil.hpp"

float scaleDamage(float damage, const MWWorld::Ptr& attacker, const MWWorld::Ptr& victim)
{
    const MWWorld::Ptr& player = MWMechanics::getPlayer();

    static const float fDifficultyMult
        = MWBase::Environment::get().getESMStore()->get<ESM::GameSetting>().find("fDifficultyMult")->mValue.getFloat();

    /*
        Start of tes3mp change (major)

        Use the difficulty setting received from the server instead of the client's own.

        0.51 reads Settings::game().mDifficulty directly into difficultyTerm, so the
        server value is substituted here rather than assigned to a local first.
    */
    const float difficultyTerm = 0.01f * mwmp::Main::get().getLocalPlayer()->difficulty;
    /*
        End of tes3mp change (major)
    */

    float x = 0;
    if (victim == player)
    {
        if (difficultyTerm > 0)
            x = fDifficultyMult * difficultyTerm;
        else
            x = difficultyTerm / fDifficultyMult;
    }
    else if (attacker == player)
    {
        if (difficultyTerm > 0)
            x = -difficultyTerm / fDifficultyMult;
        else
            x = fDifficultyMult * (-difficultyTerm);
    }

    damage *= 1 + x;
    return damage;
}

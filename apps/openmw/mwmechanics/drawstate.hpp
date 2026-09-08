#ifndef GAME_MWMECHANICS_DRAWSTATE_H
#define GAME_MWMECHANICS_DRAWSTATE_H

/*
    Start of tes3mp addition

    <winuser.h> defines DrawState as a macro expanding to DrawStateA/DrawStateW. OpenMW
    never trips over it because OpenMW does not include <windows.h> here; tes3mp does,
    through RakNet.

    The undef has to be in THIS header, not at the tes3mp include site: if <windows.h> is
    seen first, the macro rewrites the enum's own declaration below to DrawStateW, and no
    later undef can bring the name back. Undoing it here makes the type well-formed no
    matter what include order a translation unit ends up with.

    Nothing in OpenMW wants the Win32 DrawState function.
*/
#ifdef DrawState
#undef DrawState
#endif
/*
    End of tes3mp addition
*/

namespace MWMechanics
{

    enum class DrawState
    {
        Nothing = 0,
        Weapon = 1,
        Spell = 2
    };
}

#endif

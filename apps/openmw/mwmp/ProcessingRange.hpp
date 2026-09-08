#ifndef OPENMW_MWMP_PROCESSINGRANGE_H
#define OPENMW_MWMP_PROCESSINGRANGE_H

/*
    Actor processing range for multiplayer.

    TES3MP needs AI processing to have effectively no distance limit, at least until a
    better authority system exists for LocalActors: an actor that stops being processed
    stops being simulated, and its owning client stops sending updates for it, so other
    players see it freeze.

    0.47 achieved this by patching the hard-coded cap in Actors:

        // static const float maxProcessingRange = 7168.f;
        static const float maxProcessingRange = 8192.f * 50;

    0.51 turned that constant into the "actors processing range" setting, which looks
    like it should make the patch unnecessary -- except the setting is CLAMPED to
    [3584, 7168] by makeClampSanitizerInt in components/settings/categories/game.hpp.
    Configuring it is therefore not an option: the sanitizer silently reduces anything
    larger back to 7168.

    So the override still has to live in code, but it is now a single function called
    from the sites that gate AI processing, rather than a patched constant. That is the
    cheaper shape for future ports -- see PORTING.md on turning change (major) hooks
    into thin call-outs.

    If OpenMW ever raises or removes that clamp, delete this and use the setting.
*/

#include <components/settings/values.hpp>

namespace mwmp
{
    /*
        The range within which actors are simulated.

        Deliberately far beyond OpenMW's clamp. The upstream cap exists because large
        values make some single-player quests harder or impossible (OpenMW bug #1876);
        that trade is accepted in multiplayer, where a frozen actor is worse.
    */
    inline int getActorsProcessingRange()
    {
        return 8192 * 50;
    }
}

#endif

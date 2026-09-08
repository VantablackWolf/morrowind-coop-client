#ifndef OPENMW_MWMP_WINAPICONFLICTS_H
#define OPENMW_MWMP_WINAPICONFLICTS_H

/*
    Undo Win32 macros that collide with OpenMW identifiers.

    RakNet's headers pull in <windows.h> on Windows, so every tes3mp translation
    unit sees the Win32 preprocessor namespace -- which upstream OpenMW does not.

    <winuser.h> defines DrawState as DrawStateA/DrawStateW depending on UNICODE.
    OpenMW 0.47 sidestepped that by naming its enum DrawState_; 0.51 renamed it to
    an "enum class DrawState", so MWMechanics::DrawState now expands to
    MWMechanics::DrawStateW and fails to name a type. It is a preprocessor
    collision, not an API change, and the fix belongs on the tes3mp side because
    tes3mp is what dragged <windows.h> in.

    Include this after any header that may reach <windows.h>, and before using the
    affected OpenMW types.
*/

#ifdef DrawState
#undef DrawState
#endif

// Same family, same cause -- these are the Win32 macros most likely to be hit
// next as OpenMW keeps modernising its enum names.
#ifdef GetObject
#undef GetObject
#endif

#ifdef PlaySound
#undef PlaySound
#endif

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

#endif

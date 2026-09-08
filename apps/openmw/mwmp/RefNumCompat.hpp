#ifndef OPENMW_MWMP_REFNUMCOMPAT_H
#define OPENMW_MWMP_REFNUMCOMPAT_H

/*
    The reference number tes3mp puts on the wire.

    tes3mp identifies an object by exactly one of two numbers: a refNum, meaning
    "reference N of a content file", or an mpNum, meaning "the Nth object this
    server created at runtime". Both clients derive a refNum from their own copy
    of the same content files, so it means the same thing on every machine.

    0.51 broke that assumption. MWWorld::CellRef::getOrAssignRefNum hands a RefNum
    to any reference that lacks one, so that WorldModel's Ptr registry has a key
    for it, and the engine's own comment records how those are distinguished:

        // Generated RefNums have negative mContentFile

    A generated RefNum is allocated in local order, so the same object gets a
    different one on every client. Sent as identity it is worse than useless: the
    receiving client cannot match it to anything, and cannot tell that it failed
    to.

    What that looked like in practice: the second player to enter a cell saw every
    NPC in it walk on the spot. The first player had authority and was broadcasting
    an actor under a locally generated refNum ("535-0") that existed on no other
    machine, while the twelve real actors ("352868-0" upward) matched fine. The
    second player held all twelve, matched none of the incoming updates, and had no
    error to show for it.

    So: a refNum goes on the wire only when it comes from a content file. Anything
    else is reported as 0, which is already the protocol's way of saying "not a
    content-file object" and leaves mpNum to identify it.
*/

#include <components/esm/formid.hpp>

#include "../mwworld/cellref.hpp"
#include "../mwworld/ptr.hpp"

namespace mwmp
{
    namespace RefNumCompat
    {
        inline unsigned int toWire(const ESM::RefNum& refNum)
        {
            // hasContentFile() is false both for generated RefNums (negative
            // mContentFile) and for unset ones (-1), which is what we want.
            return refNum.hasContentFile() ? refNum.mIndex : 0;
        }

        inline unsigned int toWire(const MWWorld::CellRef& cellRef) { return toWire(cellRef.getRefNum()); }

        inline unsigned int toWire(const MWWorld::ConstPtr& ptr) { return toWire(ptr.getCellRef().getRefNum()); }
    }
}

#endif

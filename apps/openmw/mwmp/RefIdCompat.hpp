#ifndef OPENMW_MWMP_REFIDCOMPAT_H
#define OPENMW_MWMP_REFIDCOMPAT_H

/*
    TES3MP refId translation layer (OpenMW 0.47 -> 0.51 port)

    TES3MP's network protocol carries record identifiers as plain std::string,
    which is exactly what OpenMW 0.47 used internally. OpenMW 0.49 replaced that
    with the strong ESM::RefId type, a variant over six kinds:

        Empty, StringRefId, FormId, GeneratedRefId, IndexRefId, ESM3ExteriorCellRefId

    Rather than change the wire format (which would break every existing 0.8.1
    server, client and Lua server script), we convert at the engine boundary.
    ESM::RefId::serializeText() emits a bare lowercased string for StringRefId
    and a prefixed form ("FormId:", "Generated:", "Index:", "Esm3ExteriorCell:")
    for the others, so ordinary Morrowind record ids stay byte-compatible with
    the 0.47 wire format and the newer kinds survive the round trip.

    The two directions are NOT symmetric. Read the notes on fromWire*() before
    using them -- picking the wrong one fails silently rather than loudly.
*/

#include <string>
#include <string_view>

#include <components/esm/refid.hpp>
#include <components/esm/serializerefid.hpp>

namespace mwmp
{
    namespace RefIdCompat
    {
        /*
            Engine -> wire.

            Lossless for all six RefId kinds. Note that StringRefId is emitted
            lowercased (OpenMW interns record ids case-insensitively). TES3MP
            0.8.1 sent whatever case the record used, so any Lua server script
            doing a case-sensitive comparison on a refId will need updating --
            Morrowind record ids are case-insensitive, so comparisons should be
            lowercased on the server side anyway.
        */
        inline std::string toWire(const ESM::RefId& id) { return id.serializeText(); }

        /*
            True if the wire string denotes one of the non-string RefId kinds.

            A hand-authored Morrowind record id could in principle begin with one
            of these prefixes; nothing in the ESM format forbids it. It has never
            been observed in practice, and OpenMW's own save format has the same
            ambiguity, so we accept it.
        */
        inline bool isTypedRefId(std::string_view wire)
        {
            return wire.starts_with(ESM::formIdRefIdPrefix) || wire.starts_with(ESM::generatedRefIdPrefix)
                || wire.starts_with(ESM::indexRefIdPrefix) || wire.starts_with(ESM::esm3ExteriorCellRefIdPrefix);
        }

        /*
            Wire -> engine, for looking up a record that MUST already exist.

            Delegates to ESM::RefId::deserializeText(), which resolves StringRefId
            via StringRefId::deserializeExisting() -- that returns an EMPTY RefId
            if the string has never been interned. Use this only on paths where a
            missing record legitimately means "not found", e.g. resolving the
            target of an incoming packet against loaded content.
        */
        inline ESM::RefId fromWireExisting(std::string_view wire) { return ESM::RefId::deserializeText(wire); }

        /*
            Wire -> engine, for a record that MAY NOT exist yet.

            TES3MP synchronises custom records (see the 0.8.0 changelog): a packet
            can carry the refId of a record the receiving client is about to
            create. fromWireExisting() would yield an empty RefId there and the
            packet would quietly do nothing, so interning via RefId::stringRefId()
            is required instead.

            Use this for every custom-record creation path, and for any refId that
            originates from a server Lua script rather than from loaded content.
        */
        inline ESM::RefId fromWireCreate(std::string_view wire)
        {
            if (wire.empty())
                return ESM::RefId();
            if (isTypedRefId(wire))
                return ESM::RefId::deserializeText(wire);
            return ESM::RefId::stringRefId(wire);
        }
    }
}

#endif

#ifndef GAME_MWMECHANICS_AIACTIVATE_H
#define GAME_MWMECHANICS_AIACTIVATE_H

#include "typedaipackage.hpp"
<<<<<<< HEAD

/*
    Start of tes3mp addition

    Include additional headers for multiplayer purposes
*/
#include "../mwworld/ptr.hpp"
/*
    End of tes3mp addition
*/

=======
#include <components/esm/refid.hpp>
>>>>>>> omw51
#include <string>
#include <string_view>

namespace ESM
{
    namespace AiSequence
    {
        struct AiActivate;
    }
}

namespace MWMechanics
{
    /// \brief Causes actor to walk to activatable object and activate it
    /** Will activate when close to object **/
    class AiActivate final : public TypedAiPackage<AiActivate>
    {
    public:
        /// Constructor
        /** \param objectId Reference to object to activate **/
        explicit AiActivate(const ESM::RefId& objectId, bool repeat);

<<<<<<< HEAD
            /*
                Start of tes3mp addition

                Make it possible to initialize an AiActivate package with a specific Ptr
                as the target, allowing for more fine-tuned activation of objects
            */
            AiActivate(MWWorld::Ptr object);
            /*
                End of tes3mp addition
            */

            explicit AiActivate(const ESM::AiSequence::AiActivate* activate);
=======
        explicit AiActivate(const ESM::AiSequence::AiActivate* activate);
>>>>>>> omw51

        bool execute(const MWWorld::Ptr& actor, CharacterController& characterController, AiState& state,
            float duration) override;

        static constexpr AiPackageTypeId getTypeId() { return AiPackageTypeId::Activate; }

        void writeState(ESM::AiSequence::AiSequence& sequence) const override;

<<<<<<< HEAD
        private:
            const std::string mObjectId;

            /*
                Start of tes3mp addition

                Track the object associated with this AiActivate package
            */
            MWWorld::Ptr mObjectPtr;
            /*
                End of tes3mp addition
            */
=======
    private:
        const ESM::RefId mObjectId;
>>>>>>> omw51
    };
}
#endif // GAME_MWMECHANICS_AIACTIVATE_H

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_GROUNDED_HPP_

#include "bitset.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class ApplicableActionGenerator<WrappedApplicableActionGeneratorTag<DefaultApplicableActionGeneratorTag, GroundedTag>>
   : public ApplicableActionGeneratorBase<ApplicableActionGenerator<WrappedApplicableActionGeneratorTag<DefaultApplicableActionGeneratorTag, GroundedTag>>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<State<GroundedTag>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class ApplicableActionGeneratorBase;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_GROUNDED_HPP_

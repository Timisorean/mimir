#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_

#include "state.hpp"
#include "grounded/state_view.hpp"
#include "lifted/state_view.hpp"
#include "type_traits.hpp"

#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class.
*/
template<typename Derived>
class ApplicableActionGeneratorBase : public UncopyableMixin<ApplicableActionGeneratorBase<Derived>> {
private:
    using P = typename TypeTraits<Derived>::PlanningMode;

    ApplicableActionGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(View<State<P>> state, GroundActionList& out_applicable_actions) {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }
};


/**
 * Implementation class.
*/
template<IsPlanningMode P>
class ApplicableActionGenerator : public ApplicableActionGeneratorBase<ApplicableActionGenerator<P>> {
private:
    // Implement Config independent functionality.
};


/**
 * Type traits.
*/
template<IsPlanningMode P>
struct TypeTraits<ApplicableActionGenerator<P>> {
    using PlanningMode = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_

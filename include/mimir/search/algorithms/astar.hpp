#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "template.hpp"

#include "../heuristics.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsAAGTag AG = DefaultAAGTag, IsSSGTag SG = DefaultSSGTag>
struct AStarTag : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsAAGTag AG, IsSSGTag SG>
class Algorithm<AStarTag<P, H, AG, SG>> : public AlgorithmBase<Algorithm<AStarTag<P, H, AG, SG>>> {
private:
    Heuristic<WrappedHeuristicTag<H, P>> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class AlgorithmBase;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStarTag<P, H, AG, SG>>>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsAAGTag AG, IsSSGTag SG>
struct TypeTraits<Algorithm<AStarTag<P, H, AG, SG>>> {
    using PlanningModeTag = P;
    using HeuristicTag = H;
    using AAGTag = AG;
    using SSGTag = SG;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

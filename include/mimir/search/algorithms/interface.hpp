#ifndef MIMIR_SEARCH_ALGORITHMS_INTERFACE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"

namespace mimir
{

enum SearchStatus
{
    IN_PROGRESS,
    OUT_OF_TIME,
    OUT_OF_MEMORY,
    FAILED,
    EXHAUSTED,
    SOLVED
};

/**
 * Interface class.
 */
class IAlgorithm
{
public:
    virtual ~IAlgorithm() = default;

    virtual SearchStatus find_solution(GroundActionList& out_plan) = 0;
};

}
#endif
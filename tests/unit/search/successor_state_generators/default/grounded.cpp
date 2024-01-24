#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/successor_state_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchSSGDefaultGroundedTest) {
    // Instantiate grounded version
    auto grounded_state_repository = SSG<WrappedSSGTag<DefaultSSGTag, GroundedTag>>();
    auto problem = static_cast<Problem>(nullptr);
    const auto initial_state = grounded_state_repository.get_or_create_initial_state(problem);
}

}

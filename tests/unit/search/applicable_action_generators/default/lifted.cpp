#include <mimir/search/applicable_action_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchAAGsDefaultLiftedTest) {
    // Instantiate lifted version
    auto lifted_successor_generator = DefaultAAG<LiftedTag>();
}

}

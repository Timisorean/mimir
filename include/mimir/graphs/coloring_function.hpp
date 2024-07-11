/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_GRAPHS_COLORING_FUNCTION_HPP_
#define MIMIR_GRAPHS_COLORING_FUNCTION_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/graphs/coloring.hpp"
#include "mimir/search/state.hpp"

#include <set>
#include <unordered_map>
#include <vector>

namespace mimir
{

class ProblemColorFunction
{
private:
    Problem m_problem;

    std::unordered_map<std::string, Color> m_name_to_color;
    std::unordered_map<Color, std::string> m_color_to_name;

    void initialize_predicates();

public:
    explicit ProblemColorFunction(Problem problem);

    /// @brief get color of object.
    Color get_color(Object object) const;

    /// @brief Get unique color of state atom.
    template<PredicateCategory P>
    Color get_color(GroundAtom<P> atom, size_t pos) const;

    /// @brief Get unique color of dynamic goal literal.
    template<DynamicPredicateCategory P>
    Color get_color(State state, GroundLiteral<P> literal, size_t pos, bool mark_true_goal_literal = false) const;

    /// @brief Get unique color of static goal literal.
    Color get_color(State state, GroundLiteral<Static> literal, size_t pos, bool mark_true_goal_literal = false) const;

    /// @brief Get name of color.
    const std::string& get_color_name(Color color) const;

    /**
     * Getters
     */
    Problem get_problem() const;
    const std::unordered_map<std::string, Color>& get_name_to_color() const;
    const std::unordered_map<Color, std::string>& get_color_to_name() const;
};

/**
 * Implementations
 */

template<PredicateCategory P>
Color ProblemColorFunction::get_color(GroundAtom<P> atom, size_t pos) const
{
    return m_name_to_color.at(atom->get_predicate()->get_name() + ":" + std::to_string(pos));
}

template<DynamicPredicateCategory P>
Color ProblemColorFunction::get_color(State state, GroundLiteral<P> literal, size_t pos, bool mark_true_goal_literal) const
{
    bool is_satisfied_in_goal = state.literal_holds(literal);
    return m_name_to_color.at(literal->get_atom()->get_predicate()->get_name() + ":g"
                              + (mark_true_goal_literal ? (is_satisfied_in_goal ? ":true" : ":false") : "") + ":" + std::to_string(pos));
}

}
#endif
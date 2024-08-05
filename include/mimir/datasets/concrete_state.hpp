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

#ifndef MIMIR_DATASETS_CONCRETE_STATE_HPP_
#define MIMIR_DATASETS_CONCRETE_STATE_HPP_

#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{
class ConcreteState : public BaseVertex<ConcreteState>
{
public:
    ConcreteState(StateIndex index, State state);

    State get_state() const;

private:
    State m_state;

    bool is_equal_impl(const BaseVertex<ConcreteState>& other) const;
    size_t hash_impl() const;

    friend class BaseVertex<ConcreteState>;
};

static_assert(IsVertex<ConcreteState>);

using ConcreteStateList = std::vector<ConcreteState>;

}

#endif

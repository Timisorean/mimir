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

#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "cista/containers/tuple.h"
#include "cista/serialization.h"  // Serialization functions
#include "cista/storage/byte_buffer_vector.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

enum SearchNodeStatus
{
    NEW = 0,
    OPEN = 1,
    CLOSED = 2,
    DEAD_END = 3,
};

template<typename... SearchNodeProperties>
using SearchNode = cista::tuple<SearchNodeStatus,  //
                                StateIndex,
                                GroundActionIndex,
                                SearchNodeProperties...>;
/**
 * Getters.
 */

template<typename... SearchNodeProperties>
SearchNodeStatus get_status(const SearchNode<SearchNodeProperties...>& node)
{
    return cista::get<0>(node);
}

template<typename... SearchNodeProperties>
StateIndex get_parent_state(const SearchNode<SearchNodeProperties...>& node)
{
    return cista::get<1>(node);
}

template<typename... SearchNodeProperties>
GroundActionIndex get_creating_action(const SearchNode<SearchNodeProperties...>& node)
{
    return cista::get<2>(node);
}

template<size_t I, typename... SearchNodeProperties>
auto get_property(const SearchNode<SearchNodeProperties...>& node)
{
    static_assert(I < sizeof...(SearchNodeProperties));
    return cista::get<I + 3>(node);
}

/**
 * Setters
 */

template<typename... SearchNodeProperties>
void set_status(SearchNode<SearchNodeProperties...>& node, SearchNodeStatus status)
{
    cista::get<0>(node) = status;
}

template<typename... SearchNodeProperties>
void set_parent_state(SearchNode<SearchNodeProperties...>& node, StateIndex state)
{
    cista::get<1>(node) = state;
}

template<typename... SearchNodeProperties>
void set_creating_action(SearchNode<SearchNodeProperties...>& node, GroundActionIndex action)
{
    cista::get<2>(node) = action;
}

template<size_t I, typename... SearchNodeProperties>
void set_property(SearchNode<SearchNodeProperties...>& node, const typename std::tuple_element<I, std::tuple<SearchNodeProperties...>>::type& value)
{
    static_assert(I < sizeof...(SearchNodeProperties));
    cista::get<I + 3>(node) = value;
}

/**
 * Utility
 */

/// @brief Compute the plan consisting of ground actions by collecting the creating actions
///        and reversing them.
/// @param search_nodes The collection of all search nodes.
/// @param search_node The search node from which to start backtracking.
/// @param[out] out_plan The sequence of ground actions that leads from the initial state to
///                      the to the state underlying the search node.
template<typename SearchNodeType>
void set_plan(const cista::storage::ByteBufferVector<SearchNodeType>& search_nodes,  //
              const GroundActionList& ground_actions,
              const SearchNodeType& search_node,
              GroundActionList& out_plan)
{
    out_plan.clear();
    const SearchNodeType* cur_search_node = &search_node;

    while (get_parent_state(*cur_search_node) != std::numeric_limits<StateIndex>::max())
    {
        assert(get_creating_action(*cur_search_node) != std::numeric_limits<GroundActionIndex>::max());

        out_plan.push_back(ground_actions.at(get_creating_action(*cur_search_node)));

        cur_search_node = &search_nodes.at(get_parent_state(*cur_search_node));
    }

    std::reverse(out_plan.begin(), out_plan.end());
}

}

#endif
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

#include "mimir/search/algorithms/brfs.hpp"

#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

BrFSAlgorithm::BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator) :
    BrFSAlgorithm(applicable_action_generator,
                  std::make_shared<StateRepository>(applicable_action_generator),
                  std::make_shared<DefaultBrFSAlgorithmEventHandler>())
{
}

BrFSAlgorithm::BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                             std::shared_ptr<StateRepository> successor_state_generator,
                             std::shared_ptr<IBrFSAlgorithmEventHandler> event_handler) :
    m_aag(std::move(applicable_action_generator)),
    m_ssg(std::move(successor_state_generator)),
    m_event_handler(std::move(event_handler)),
    m_search_nodes(FlatSearchNodeVector<uint32_t>(
        SearchNodeBuilder<uint32_t>(SearchNodeStatus::NEW, std::optional<State>(std::nullopt), std::optional<GroundAction>(std::nullopt), (uint32_t) 0)
            .get_flatmemory_builder()))
{
}

SearchStatus BrFSAlgorithm::find_solution(GroundActionList& out_plan) { return find_solution(m_ssg->get_or_create_initial_state(), out_plan); }

SearchStatus BrFSAlgorithm::find_solution(State start_state, GroundActionList& out_plan)
{
    std::optional<State> unused_out_state = std::nullopt;
    return find_solution(start_state, out_plan, unused_out_state);
}

SearchStatus BrFSAlgorithm::find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state)
{
    return find_solution(start_state, std::make_unique<ProblemGoal>(m_aag->get_problem()), std::make_unique<DuplicateStatePruning>(), out_plan, out_goal_state);
}

SearchStatus BrFSAlgorithm::find_solution(State start_state,
                                          std::unique_ptr<IGoalStrategy>&& goal_strategy,
                                          std::unique_ptr<IPruningStrategy>&& pruning_strategy,
                                          GroundActionList& out_plan,
                                          std::optional<State>& out_goal_state)
{
    // Clear data structures
    m_search_nodes.clear();
    m_queue.clear();

    const auto problem = m_aag->get_problem();
    const auto& pddl_factories = *m_aag->get_pddl_factories();
    m_event_handler->on_start_search(start_state, problem, pddl_factories);

    auto start_search_node = SearchNode<uint32_t>(this->m_search_nodes[start_state.get_index()]);
    start_search_node.set_status(SearchNodeStatus::OPEN);
    start_search_node.set_property<0>(0);

    if (!goal_strategy->test_static_goal())
    {
        m_event_handler->on_unsolvable();

        return SearchStatus::UNSOLVABLE;
    }

    auto applicable_actions = GroundActionList {};

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        return SearchStatus::FAILED;
    }

    m_queue.emplace_back(start_state);

    auto g_value = uint32_t { 0 };

    while (!m_queue.empty())
    {
        const auto state = m_queue.front();
        m_queue.pop_front();

        // We need this before goal test for correct statistics reporting.
        const auto flat_search_node = this->m_search_nodes[state.get_index()];
        auto search_node = SearchNode<uint32_t>(flat_search_node);
        const auto const_search_node = ConstSearchNode<uint32_t>(flat_search_node);

        if (const_search_node.get_property<0>() > g_value)
        {
            g_value = const_search_node.get_property<0>();
            m_aag->on_finish_search_layer();
            m_event_handler->on_finish_g_layer();
        }

        if (goal_strategy->test_dynamic_goal(state))
        {
            set_plan(this->m_search_nodes, const_search_node, out_plan);
            out_goal_state = state;
            m_event_handler->on_end_search();
            if (!m_event_handler->is_quiet())
            {
                m_aag->on_end_search();
            }
            m_event_handler->on_solved(out_plan);

            return SearchStatus::SOLVED;
        }

        m_event_handler->on_expand_state(state, const_search_node, problem, pddl_factories);

        this->m_aag->generate_applicable_actions(state, applicable_actions);

        for (const auto& action : applicable_actions)
        {
            /* Open state. */
            const auto successor_state = this->m_ssg->get_or_create_successor_state(state, action);
            const auto flat_successor_search_node = this->m_search_nodes[successor_state.get_index()];
            auto successor_search_node = SearchNode<uint32_t>(flat_successor_search_node);
            const auto const_successor_search_node = ConstSearchNode<uint32_t>(flat_successor_search_node);

            m_event_handler->on_generate_state(successor_state, const_successor_search_node, problem, pddl_factories);

            bool is_new_successor_state = (const_successor_search_node.get_status() == SearchNodeStatus::NEW);
            if (pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                m_event_handler->on_prune_state(successor_state, problem, pddl_factories);
                continue;
            }

            successor_search_node.set_status(SearchNodeStatus::OPEN);
            successor_search_node.set_parent_state(state);
            successor_search_node.set_creating_action(action);
            successor_search_node.set_property<0>(const_search_node.get_property<0>() + 1);

            m_queue.emplace_back(successor_state);
        }

        /* Close state. */
        search_node.set_status(SearchNodeStatus::CLOSED);
    }

    m_event_handler->on_end_search();
    m_event_handler->on_exhausted();

    return SearchStatus::EXHAUSTED;
}

}

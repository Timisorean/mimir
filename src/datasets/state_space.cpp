/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/datasets/state_space.hpp"

#include "mimir/algorithms/BS_thread_pool.hpp"
#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace mimir
{

StateSpace::StateSpace(bool use_unit_cost_one,
                       std::shared_ptr<PDDLParser> parser,
                       std::shared_ptr<IAAG> aag,
                       std::shared_ptr<SuccessorStateGenerator> ssg,
                       StateList states,
                       StateMap<StateIndex> state_to_index,
                       StateIndex initial_state,
                       StateIndexSet goal_states,
                       StateIndexSet deadend_states,
                       TransitionList transitions,
                       std::vector<TransitionIndexList<Forward>> forward_transitions,
                       std::vector<TransitionIndexList<Backward>> backward_transitions,
                       std::vector<double> goal_distances) :
    m_use_unit_cost_one(use_unit_cost_one),
    m_parser(std::move(parser)),
    m_aag(std::move(aag)),
    m_ssg(std::move(ssg)),
    m_states(std::move(states)),
    m_state_to_index(std::move(state_to_index)),
    m_initial_state(std::move(initial_state)),
    m_goal_states(std::move(goal_states)),
    m_deadend_states(std::move(deadend_states)),
    m_transitions(std::move(transitions)),
    m_forward_transitions(std::move(forward_transitions)),
    m_backward_transitions(std::move(backward_transitions)),
    m_goal_distances(std::move(goal_distances)),
    m_states_by_goal_distance()
{
    for (size_t state_id = 0; state_id < m_states.size(); ++state_id)
    {
        m_states_by_goal_distance[m_goal_distances.at(state_id)].push_back(state_id);
    }
}

std::optional<StateSpace> StateSpace::create(const fs::path& domain_filepath,
                                             const fs::path& problem_filepath,
                                             bool use_unit_cost_one,
                                             bool remove_if_unsolvable,
                                             uint32_t max_num_states,
                                             uint32_t timeout_ms)
{
    auto parser = std::make_shared<PDDLParser>(domain_filepath, problem_filepath);
    const auto problem = parser->get_problem();
    auto aag = std::make_shared<GroundedAAG>(problem, parser->get_factories());
    auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
    return StateSpace::create(std::move(parser), std::move(aag), std::move(ssg), use_unit_cost_one, remove_if_unsolvable, max_num_states, timeout_ms);
}

std::optional<StateSpace> StateSpace::create(std::shared_ptr<PDDLParser> parser,
                                             std::shared_ptr<IAAG> aag,
                                             std::shared_ptr<SuccessorStateGenerator> ssg,
                                             bool use_unit_cost_one,
                                             bool remove_if_unsolvable,
                                             uint32_t max_num_states,
                                             uint32_t timeout_ms)
{
    auto stop_watch = StopWatch(timeout_ms);

    const auto problem = parser->get_problem();
    auto initial_state = ssg->get_or_create_initial_state();

    if (remove_if_unsolvable && !problem->static_goal_holds())
    {
        // Unsolvable
        return std::nullopt;
    }

    auto lifo_queue = std::deque<State>();
    lifo_queue.push_back(initial_state);

    const auto initial_state_index = 0;
    auto states = StateList { initial_state };
    auto state_to_index = StateMap<StateIndex> {};
    state_to_index[initial_state] = initial_state_index;
    auto transitions = TransitionList {};
    auto forward_transitions = std::vector<TransitionIndexList<Forward>>(1);
    auto backward_transitions = std::vector<TransitionIndexList<Backward>>(1);
    auto goal_states = StateIndexSet {};

    auto applicable_actions = GroundActionList {};
    stop_watch.start();
    while (!lifo_queue.empty() && !stop_watch.has_finished())
    {
        const auto state = lifo_queue.back();
        const auto state_index = state_to_index.at(state);
        lifo_queue.pop_back();
        if (state.literals_hold(problem->get_goal_condition<Fluent>()) && state.literals_hold(problem->get_goal_condition<Derived>()))
        {
            goal_states.insert(state_index);
        }

        aag->generate_applicable_actions(state, applicable_actions);
        for (const auto& action : applicable_actions)
        {
            const auto successor_state = ssg->get_or_create_successor_state(state, action);
            const auto it = state_to_index.find(successor_state);
            const bool exists = (it != state_to_index.end());
            if (exists)
            {
                const auto successor_state_index = it->second;
                const auto transition_index = transitions.size();
                transitions.emplace_back(successor_state_index, state_index, action);
                forward_transitions.at(state_index).indices.push_back(transition_index);
                backward_transitions.at(successor_state_index).indices.push_back(transition_index);
                continue;
            }

            const auto successor_state_index = states.size();
            if (successor_state_index >= max_num_states)
            {
                // Ran out of state resources
                return std::nullopt;
            }

            forward_transitions.resize(states.size() + 1);
            backward_transitions.resize(states.size() + 1);

            const auto transition_index = transitions.size();
            transitions.emplace_back(successor_state_index, state_index, action);
            forward_transitions.at(state_index).indices.push_back(transition_index);
            backward_transitions.at(successor_state_index).indices.push_back(transition_index);

            states.push_back(successor_state);
            state_to_index[successor_state] = successor_state_index;
            lifo_queue.push_back(successor_state);
        }
    }

    if (stop_watch.has_finished())
    {
        // Ran out of time
        return std::nullopt;
    }

    if (remove_if_unsolvable && goal_states.empty())
    {
        // Skip: unsolvable
        return std::nullopt;
    }

    auto goal_distances = mimir::compute_shortest_distances_from_states(states.size(),
                                                                        StateIndexList { goal_states.begin(), goal_states.end() },
                                                                        transitions,
                                                                        backward_transitions,
                                                                        use_unit_cost_one);

    auto deadend_states = StateIndexSet {};
    for (StateIndex state_id = 0; state_id < states.size(); ++state_id)
    {
        if (goal_distances.at(state_id) == std::numeric_limits<double>::max())
        {
            deadend_states.insert(state_id);
        }
    }

    // Must explicitly call constructor since it is private
    return StateSpace(use_unit_cost_one,
                      std::move(parser),
                      std::move(aag),
                      std::move(ssg),
                      std::move(states),
                      std::move(state_to_index),
                      initial_state_index,
                      std::move(goal_states),
                      std::move(deadend_states),
                      std::move(transitions),
                      std::move(forward_transitions),
                      std::move(backward_transitions),
                      std::move(goal_distances));
}

StateSpaceList StateSpace::create(const fs::path& domain_filepath,
                                  const std::vector<fs::path>& problem_filepaths,
                                  bool use_unit_cost_one,
                                  bool remove_if_unsolvable,
                                  bool sort_ascending_by_num_states,
                                  uint32_t max_num_states,
                                  uint32_t timeout_ms,
                                  uint32_t num_threads)
{
    auto memories = std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = std::make_shared<PDDLParser>(domain_filepath, problem_filepath);
        auto aag = std::make_shared<GroundedAAG>(parser->get_problem(), parser->get_factories());
        auto ssg = std::make_shared<SuccessorStateGenerator>(aag);
        memories.emplace_back(std::move(parser), std::move(aag), std::move(ssg));
    }

    return StateSpace::create(memories, use_unit_cost_one, remove_if_unsolvable, sort_ascending_by_num_states, max_num_states, timeout_ms, num_threads);
}

std::vector<StateSpace>
StateSpace::create(const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
                   bool use_unit_cost_one,
                   bool remove_if_unsolvable,
                   bool sort_ascending_by_num_states,
                   uint32_t max_num_states,
                   uint32_t timeout_ms,
                   uint32_t num_threads)
{
    auto state_spaces = StateSpaceList {};
    auto pool = BS::thread_pool(num_threads);
    auto futures = std::vector<std::future<std::optional<StateSpace>>> {};

    for (const auto& [parser, aag, ssg] : memories)
    {
        futures.push_back(
            pool.submit_task([parser, aag, ssg, use_unit_cost_one, remove_if_unsolvable, max_num_states, timeout_ms]
                             { return StateSpace::create(parser, aag, ssg, use_unit_cost_one, remove_if_unsolvable, max_num_states, timeout_ms); }));
    }

    for (auto& future : futures)
    {
        auto state_space = future.get();
        if (state_space.has_value())
        {
            state_spaces.push_back(std::move(state_space.value()));
        }
    }

    if (sort_ascending_by_num_states)
    {
        std::sort(state_spaces.begin(), state_spaces.end(), [](const auto& l, const auto& r) { return l.get_num_states() < r.get_num_states(); });
    }

    return state_spaces;
}

/**
 * Extended functionality
 */

std::vector<double> StateSpace::compute_shortest_distances_from_states(const StateIndexList& states, const bool forward) const
{
    throw std::runtime_error("Not implemented");
}

std::vector<std::vector<double>> StateSpace::compute_pairwise_shortest_state_distances(const bool forward) const
{
    throw std::runtime_error("Not implemented");
}

/**
 *  Getters
 */

/* Meta data */
bool StateSpace::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

/* Memory */
const std::shared_ptr<PDDLParser>& StateSpace::get_pddl_parser() const { return m_parser; }

const std::shared_ptr<IAAG>& StateSpace::get_aag() const { return m_aag; }

const std::shared_ptr<SuccessorStateGenerator>& StateSpace::get_ssg() const { return m_ssg; }

/* States */
const StateList& StateSpace::get_states() const { return m_states; }

StateIndex StateSpace::get_state_index(State state) const { return m_state_to_index.at(state); }

StateIndex StateSpace::get_initial_state() const { return m_initial_state; }

const StateIndexSet& StateSpace::get_goal_states() const { return m_goal_states; }

const StateIndexSet& StateSpace::get_deadend_states() const { return m_deadend_states; }

size_t StateSpace::get_num_states() const { return get_states().size(); }

size_t StateSpace::get_num_goal_states() const { return get_goal_states().size(); }

size_t StateSpace::get_num_deadend_states() const { return get_deadend_states().size(); }

bool StateSpace::is_goal_state(StateIndex state) const { return get_goal_states().count(state); }

bool StateSpace::is_deadend_state(StateIndex state) const { return get_deadend_states().count(state); }

bool StateSpace::is_alive_state(StateIndex state) const { return !(get_goal_states().count(state) || get_deadend_states().count(state)); }

/* Transitions */
size_t StateSpace::get_num_transitions() const { return m_transitions.size(); }

const TransitionList& StateSpace::get_transitions() const { return m_transitions; }

const std::vector<TransitionIndexList<Forward>>& StateSpace::get_forward_transition_adjacency_lists() const { return m_forward_transitions; }

const std::vector<TransitionIndexList<Backward>>& StateSpace::get_backward_transition_adjacency_lists() const { return m_backward_transitions; }

/* Distances */
const std::vector<double>& StateSpace::get_goal_distances() const { return m_goal_distances; }

double StateSpace::get_goal_distance(State state) const { return m_goal_distances.at(get_state_index(state)); }

double StateSpace::get_max_goal_distance() const { return *std::max_element(m_goal_distances.begin(), m_goal_distances.end()); }

/* Additional */
StateIndex StateSpace::sample_state_with_goal_distance(double goal_distance) const
{
    const auto& states = m_states_by_goal_distance.at(goal_distance);
    const auto index = std::rand() % static_cast<int>(states.size());
    return states.at(index);
}

}

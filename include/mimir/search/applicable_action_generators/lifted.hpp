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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_

#include "mimir/formalism/action.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted/axiom_evaluator.hpp"
#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/state.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
using GroundFunctionToValue = std::unordered_map<GroundFunction, double>;

/// @brief `LiftedApplicableActionGenerator` implements lifted applicable action generation
/// using maximum clique enumeration by Stahlberg (ECAI2023).
/// Source: https://mrlab.ai/papers/stahlberg-ecai2023.pdf
class LiftedApplicableActionGenerator : public IApplicableActionGenerator
{
private:
    Problem m_problem;

    // Memory
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_event_handler;

    AxiomEvaluator m_axiom_evaluator;

    std::unordered_map<Action, ConditionGrounder<State>> m_action_precondition_grounders;
    std::unordered_map<Action, std::vector<consistency_graph::StaticConsistencyGraph>> m_action_universal_effects;

    FlatActionSet m_flat_actions;
    GroundActionList m_actions_by_index;
    GroundActionBuilder m_action_builder;
    std::unordered_map<Action, GroundingTable<GroundAction>> m_action_groundings;

    GroundFunctionToValue m_ground_function_value_costs;

    /// @brief Ground the precondition of an action and return a view onto it.
    GroundAction ground_action_precondition(Action action, const ObjectList& binding);

public:
    /// @brief Simplest construction
    LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> ref_pddl_factories);

    /// @brief Complete construction
    LiftedApplicableActionGenerator(Problem problem,
                                    std::shared_ptr<PDDLFactories> ref_pddl_factories,
                                    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> event_handler);

    // Uncopyable
    LiftedApplicableActionGenerator(const LiftedApplicableActionGenerator& other) = delete;
    LiftedApplicableActionGenerator& operator=(const LiftedApplicableActionGenerator& other) = delete;
    // Unmovable
    LiftedApplicableActionGenerator(LiftedApplicableActionGenerator&& other) = delete;
    LiftedApplicableActionGenerator& operator=(LiftedApplicableActionGenerator&& other) = delete;

    void generate_applicable_actions(State state, GroundActionList& out_applicable_actions) override;

    void generate_and_apply_axioms(const FlatBitset& fluent_state_atoms, FlatBitset& ref_derived_state_atoms) override;

    void on_finish_search_layer() const override;

    void on_end_search() const override;

    Problem get_problem() const override;

    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const override;

    /// @brief Return the axiom partitioning.
    const std::vector<AxiomPartition>& get_axiom_partitioning() const;

    /// @brief Ground an axiom and return a view onto it.
    GroundAxiom ground_axiom(Axiom axiom, ObjectList&& binding);

    /// @brief Ground an action and return a view onto it.
    GroundAction ground_action(Action action, ObjectList&& binding);

    const GroundActionList& get_ground_actions() const override;

    GroundAction get_ground_action(size_t action_index) const override;

    const GroundAxiomList& get_ground_axioms() const override;

    GroundAxiom get_ground_axiom(size_t axiom_index) const override;

    size_t get_num_ground_actions() const override;

    size_t get_num_ground_axioms() const override;

    friend std::ostream& operator<<(std::ostream& out, const LiftedApplicableActionGenerator& lifted_aag);
};

std::ostream& operator<<(std::ostream& out, const LiftedApplicableActionGenerator& lifted_aag);

}  // namespace mimir

#endif

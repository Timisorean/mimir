/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

/**
 * Let V be the set of all variables and free(V) all free variables.
 * Let v in V be a quantified variables that occurs free in other parts,
 * e.g., forall(v, phi) and psi with v in free(psi).
 * Then do the following renaming:
 *       forall(v_i, phi) and psi with v_j in free(psi).
 *
 * First, rename variables v1,...,vk to v1_1,...,vk_k to make them all unique
 * Second, for quantified variable vi_i rename it to vi_i_j where j is
 * the amount of times that the variable is being quantified over.
 *
 * Implementation detail, when opening a new scope, we need to replace variable renamings
 *
 */
class RenameQuantifiedVariablesTranslator : public BaseTranslator<RenameQuantifiedVariablesTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<RenameQuantifiedVariablesTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    // Collect all variables in preparation phase.
    std::unordered_set<loki::Variable> m_variables;
    // Keep track of the number of times that each variable was quantified during translation phase
    std::unordered_map<loki::Variable, size_t> m_num_quantifications;

    class Scope
    {
    private:
        std::unordered_map<loki::Variable, loki::Variable> m_renaming;

        const Scope* m_parent_scope;

    public:
        Scope(std::unordered_map<loki::Variable, loki::Variable> renaming, const Scope* parent_scope = nullptr) :
            m_renaming(std::move(renaming)),
            m_parent_scope(parent_scope)
        {
        }

        const std::unordered_map<loki::Variable, loki::Variable>& get_renaming() const { return m_renaming; }
    };

    class ScopeStack
    {
    private:
        std::deque<std::unique_ptr<Scope>> m_stack;

    public:
        /**
         * Open the first scope
         */
        const Scope&
        open_scope(const loki::VariableList& variables, std::unordered_map<loki::Variable, size_t>& num_quantifications, loki::PDDLFactories& pddl_factories)
        {
            assert(m_stack.empty());

            std::unordered_map<loki::Variable, loki::Variable> renamings;
            for (const auto& variable : variables)
            {
                num_quantifications.emplace(variable, 0);

                const auto renamed_variable =
                    pddl_factories.get_or_create_variable(variable->get_name() + "_" + std::to_string(variable->get_identifier()) + "_" + std::to_string(0));
                renamings.emplace(variable, renamed_variable);
            }

            m_stack.push_back(std::make_unique<Scope>(std::move(renamings)));

            return get();
        }

        /**
         * Open successive scope.
         */
        const Scope&
        open_scope(const loki::ParameterList& parameters, std::unordered_map<loki::Variable, size_t>& num_quantifications, loki::PDDLFactories& pddl_factories)
        {
            assert(!m_stack.empty());

            const auto& parent_renamings = get().get_renaming();

            std::unordered_map<loki::Variable, loki::Variable> renamings = parent_renamings;

            for (const auto& parameter : parameters)
            {
                // Increment number of quantifications of the variable.
                const auto renamed_variable = pddl_factories.get_or_create_variable(parameter->get_variable()->get_name() + "_"
                                                                                    + std::to_string(parameter->get_variable()->get_identifier()) + "_"
                                                                                    + std::to_string(++num_quantifications.at(parameter->get_variable())));

                renamings[parameter->get_variable()] = renamed_variable;
            }

            m_stack.push_back(std::make_unique<Scope>(std::move(renamings), &get()));

            return get();
        }

        void close_scope()
        {
            assert(!m_stack.empty());
            m_stack.pop_back();
        }

        const Scope& get() const { return *m_stack.back(); }
    };

    ScopeStack m_scopes;

    void prepare_impl(const loki::VariableImpl& variable);

    loki::Variable translate_impl(const loki::VariableImpl& variable);
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition);
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect);
    loki::Action translate_impl(const loki::ActionImpl& action);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif
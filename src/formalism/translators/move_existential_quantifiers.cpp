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

#include "mimir/formalism/translators/move_existential_quantifiers.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

loki::Condition MoveExistentialQuantifiersTranslator::translate_impl(const loki::ConditionAndImpl& condition)
{
    const auto& translated_nested_conditions = this->translate(condition.get_conditions());

    auto parameters = loki::ParameterList {};
    auto parts = loki::ConditionList {};
    for (const auto translated_nested_condition : translated_nested_conditions)
    {
        if (const auto translated_nested_condition_exists = std::get_if<loki::ConditionExistsImpl>(translated_nested_condition))
        {
            parameters.insert(parameters.end(),
                              translated_nested_condition_exists->get_parameters().begin(),
                              translated_nested_condition_exists->get_parameters().end());

            parts.push_back(translated_nested_condition_exists->get_condition());
        }
        else
        {
            parts.push_back(translated_nested_condition);
        }
    }

    const auto parts_conjunction = this->m_pddl_factories.get_or_create_condition_and(parts);

    if (parameters.empty())
    {
        return parts_conjunction;
    }

    return this->translate(*this->m_pddl_factories.get_or_create_condition_exists(parameters, parts_conjunction));
}

loki::Condition MoveExistentialQuantifiersTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    return flatten(std::get<loki::ConditionExistsImpl>(*this->m_pddl_factories.get_or_create_condition_exists(this->translate(condition.get_parameters()),
                                                                                                              this->translate(*condition.get_condition()))),
                   this->m_pddl_factories);
}

loki::Problem MoveExistentialQuantifiersTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

MoveExistentialQuantifiersTranslator::MoveExistentialQuantifiersTranslator(loki::PDDLFactories& pddl_factories) : BaseCachedRecurseTranslator(pddl_factories) {}

}
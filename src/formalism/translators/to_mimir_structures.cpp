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

#include "mimir/formalism/translators/to_mimir_structures.hpp"

#include "mimir/formalism/translators/utils.hpp"

#include <numeric>

namespace mimir
{
void ToMimirStructures::prepare(const loki::RequirementsImpl& requirements) {}
void ToMimirStructures::prepare(const loki::TypeImpl& type) { prepare(type.get_bases()); }
void ToMimirStructures::prepare(const loki::ObjectImpl& object) { prepare(object.get_bases()); }
void ToMimirStructures::prepare(const loki::VariableImpl& variable) {}
void ToMimirStructures::prepare(const loki::TermObjectImpl& term) { prepare(*term.get_object()); }
void ToMimirStructures::prepare(const loki::TermVariableImpl& term) { prepare(*term.get_variable()); }
void ToMimirStructures::prepare(const loki::TermImpl& term)
{
    std::visit([this](auto&& arg) { return this->prepare(arg); }, term);
}
void ToMimirStructures::prepare(const loki::ParameterImpl& parameter) { prepare(*parameter.get_variable()); }
void ToMimirStructures::prepare(const loki::PredicateImpl& predicate) { prepare(predicate.get_parameters()); }
void ToMimirStructures::prepare(const loki::AtomImpl& atom)
{
    prepare(*atom.get_predicate());
    prepare(atom.get_terms());
}
void ToMimirStructures::prepare(const loki::LiteralImpl& literal) { prepare(*literal.get_atom()); }
void ToMimirStructures::prepare(const loki::NumericFluentImpl& numeric_fluent) { prepare(*numeric_fluent.get_function()); }
void ToMimirStructures::prepare(const loki::ConditionLiteralImpl& condition) { prepare(*condition.get_literal()); }
void ToMimirStructures::prepare(const loki::ConditionAndImpl& condition) { prepare(condition.get_conditions()); }
void ToMimirStructures::prepare(const loki::ConditionOrImpl& condition) { prepare(condition.get_conditions()); }
void ToMimirStructures::prepare(const loki::ConditionNotImpl& condition) { prepare(*condition.get_condition()); }
void ToMimirStructures::prepare(const loki::ConditionImplyImpl& condition)
{
    prepare(*condition.get_condition_left());
    prepare(*condition.get_condition_right());
}
void ToMimirStructures::prepare(const loki::ConditionExistsImpl& condition)
{
    prepare(condition.get_parameters());
    prepare(*condition.get_condition());
}
void ToMimirStructures::prepare(const loki::ConditionForallImpl& condition)
{
    prepare(condition.get_parameters());
    prepare(*condition.get_condition());
}
void ToMimirStructures::prepare(const loki::ConditionImpl& condition)
{
    std::visit([this](auto&& arg) { return this->prepare(arg); }, condition);
}
void ToMimirStructures::prepare(const loki::EffectImpl& effect)
{
    /**
     * Find predicates affected by an effect.
     */
    auto effect_ptr = &effect;

    // 1. Prepare conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAndImpl>(effect_ptr))
    {
        for (const auto& nested_effect : effect_and->get_effects())
        {
            auto tmp_effect = nested_effect;

            // 2. Prepare universal part
            if (const auto& tmp_effect_forall = std::get_if<loki::EffectConditionalForallImpl>(tmp_effect))
            {
                prepare(tmp_effect_forall->get_parameters());

                tmp_effect = tmp_effect_forall->get_effect();
            }

            // 3. Prepare conditional part
            if (const auto& tmp_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(tmp_effect))
            {
                if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(tmp_effect_when->get_condition()))
                {
                    for (const auto& part : condition_and->get_conditions())
                    {
                        if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
                        {
                            prepare(*condition_literal->get_literal());
                        }
                        else
                        {
                            std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                            throw std::logic_error("Expected literal in conjunctive condition.");
                        }
                    }
                }
                else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(tmp_effect_when->get_condition()))
                {
                    prepare(*condition_literal->get_literal());
                }

                tmp_effect = tmp_effect_when->get_effect();
            }

            // 4. Parse simple effect
            if (const auto& effect_literal = std::get_if<loki::EffectLiteralImpl>(tmp_effect))
            {
                prepare(*effect_literal->get_literal());

                // Found predicate affected by an effect
                m_fluent_predicates.insert(effect_literal->get_literal()->get_atom()->get_predicate());
            }
            else if (const auto& effect_numeric = std::get_if<loki::EffectNumericImpl>(tmp_effect))
            {
                assert(effect_numeric->get_assign_operator() == loki::AssignOperatorEnum::INCREASE);
                assert(effect_numeric->get_function()->get_function_skeleton()->get_name() == "total-cost");
                prepare(*effect_numeric->get_function_expression());
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

                throw std::logic_error("Expected simple effect.");
            }
        }
        return;
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *effect_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive effect.");
}
void ToMimirStructures::prepare(const loki::FunctionExpressionNumberImpl& function_expression) {}
void ToMimirStructures::prepare(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    prepare(*function_expression.get_left_function_expression());
    prepare(*function_expression.get_right_function_expression());
}
void ToMimirStructures::prepare(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    this->prepare(function_expression.get_function_expressions());
}
void ToMimirStructures::prepare(const loki::FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
void ToMimirStructures::prepare(const loki::FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
void ToMimirStructures::prepare(const loki::FunctionExpressionImpl& function_expression)
{
    std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression);
}
void ToMimirStructures::prepare(const loki::FunctionSkeletonImpl& function_skeleton)
{
    prepare(function_skeleton.get_parameters());
    prepare(*function_skeleton.get_type());
}
void ToMimirStructures::prepare(const loki::FunctionImpl& function)
{
    prepare(*function.get_function_skeleton());
    prepare(function.get_terms());
}
void ToMimirStructures::prepare(const loki::ActionImpl& action)
{
    prepare(action.get_parameters());
    if (action.get_condition().has_value())
    {
        prepare(*action.get_condition().value());
    }
    if (action.get_effect().has_value())
    {
        prepare(*action.get_effect().value());
    }
}
void ToMimirStructures::prepare(const loki::AxiomImpl& axiom)
{
    prepare(*axiom.get_condition());
    prepare(*axiom.get_literal());
}
void ToMimirStructures::prepare(const loki::DomainImpl& domain)
{
    prepare(*domain.get_requirements());
    prepare(domain.get_types());
    prepare(domain.get_constants());
    prepare(domain.get_predicates());
    prepare(domain.get_derived_predicates());
    prepare(domain.get_functions());
    prepare(domain.get_actions());
    prepare(domain.get_axioms());
}
void ToMimirStructures::prepare(const loki::OptimizationMetricImpl& metric) { prepare(*metric.get_function_expression()); }
void ToMimirStructures::prepare(const loki::ProblemImpl& problem)
{
    prepare(*problem.get_domain());
    prepare(*problem.get_requirements());
    prepare(problem.get_objects());
    prepare(problem.get_derived_predicates());
    prepare(problem.get_initial_literals());
    prepare(problem.get_numeric_fluents());
    if (problem.get_goal_condition().has_value())
    {
        prepare(*problem.get_goal_condition().value());
    }
    if (problem.get_optimization_metric().has_value())
    {
        prepare(*problem.get_optimization_metric().value());
    }
    prepare(problem.get_axioms());
}

/**
 * Common
 */

Requirements ToMimirStructures::translate_common(const loki::RequirementsImpl& requirements)
{
    return m_pddl_factories.get_or_create_requirements(requirements.get_requirements());
}

Variable ToMimirStructures::translate_common(const loki::VariableImpl& variable)
{
    const auto parameter_index = m_variable_to_parameter_index.at(&variable);

    // Encode parameter index in variable name for visibility
    const auto variable_name = variable.get_name() + "_" + std::to_string(parameter_index);

    return m_pddl_factories.get_or_create_variable(variable_name, parameter_index);
}

Object ToMimirStructures::translate_common(const loki::ObjectImpl& object)
{
    if (!object.get_bases().empty())
    {
        throw std::logic_error("Expected types to be empty.");
    }
    return m_pddl_factories.get_or_create_object(object.get_name());
}

Parameter ToMimirStructures::translate_common(const loki::ParameterImpl& parameter)
{
    if (!parameter.get_bases().empty())
    {
        throw std::logic_error("Expected types to be empty.");
    }
    return m_pddl_factories.get_or_create_parameter(translate_common(*parameter.get_variable()));
}

Predicate ToMimirStructures::translate_common(const loki::PredicateImpl& predicate)
{
    auto result = m_pddl_factories.get_or_create_predicate(predicate.get_name(), translate_common(predicate.get_parameters()));
    if (result->get_name() == "=")
    {
        m_equal_predicate = result;
    }
    return result;
}

/**
 * Lifted
 */

Term ToMimirStructures::translate_lifted(const loki::TermVariableImpl& term)
{
    return m_pddl_factories.get_or_create_term_variable(translate_common(*term.get_variable()));
}

Term ToMimirStructures::translate_lifted(const loki::TermObjectImpl& term)
{
    return m_pddl_factories.get_or_create_term_object(translate_common(*term.get_object()));
}

Term ToMimirStructures::translate_lifted(const loki::TermImpl& term)
{
    return std::visit([this](auto&& arg) { return this->translate_lifted(arg); }, term);
}

Atom ToMimirStructures::translate_lifted(const loki::AtomImpl& atom)
{
    return m_pddl_factories.get_or_create_atom(translate_common(*atom.get_predicate()), translate_lifted(atom.get_terms()));
}

Literal ToMimirStructures::translate_lifted(const loki::LiteralImpl& literal)
{
    return m_pddl_factories.get_or_create_literal(literal.is_negated(), translate_lifted(*literal.get_atom()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionNumberImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_number(function_expression.get_number());
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                              translate_lifted(*function_expression.get_left_function_expression()),
                                                                              translate_lifted(*function_expression.get_right_function_expression()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                             translate_lifted(function_expression.get_function_expressions()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionMinusImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_minus(translate_lifted(*function_expression.get_function_expression()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_function(translate_lifted(*function_expression.get_function()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionImpl& function_expression)
{
    return std::visit([this](auto&& arg) { return this->translate_lifted(arg); }, function_expression);
}

FunctionSkeleton ToMimirStructures::translate_lifted(const loki::FunctionSkeletonImpl& function_skeleton)
{
    return m_pddl_factories.get_or_create_function_skeleton(function_skeleton.get_name(), translate_common(function_skeleton.get_parameters()));
}

Function ToMimirStructures::translate_lifted(const loki::FunctionImpl& function)
{
    return m_pddl_factories.get_or_create_function(translate_lifted(*function.get_function_skeleton()), translate_lifted(function.get_terms()));
}

std::tuple<ParameterList, LiteralList, LiteralList, LiteralList> ToMimirStructures::translate_lifted(const loki::ConditionImpl& condition)
{
    auto condition_ptr = &condition;

    auto parameters = ParameterList {};
    if (const auto condition_exists = std::get_if<loki::ConditionExistsImpl>(condition_ptr))
    {
        parameters = translate_common(condition_exists->get_parameters());

        condition_ptr = condition_exists->get_condition();
    }

    const auto func_insert_fluents = [](const loki::Literal literal,
                                        const Literal& translated_literal,
                                        const std::unordered_set<loki::Predicate>& fluent_predicates,
                                        LiteralList& ref_literals,
                                        LiteralList& ref_static_literals,
                                        LiteralList& ref_fluent_literals)
    {
        ref_literals.push_back(translated_literal);
        if (fluent_predicates.count(literal->get_atom()->get_predicate()))
        {
            ref_fluent_literals.push_back(translated_literal);
        }
        else
        {
            ref_static_literals.push_back(translated_literal);
        }
    };

    if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(condition_ptr))
    {
        auto literals = LiteralList {};
        auto static_literals = LiteralList {};
        auto fluent_literals = LiteralList {};
        for (const auto& part : condition_and->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
            {
                const auto translated_literal = translate_lifted(*condition_literal->get_literal());

                func_insert_fluents(condition_literal->get_literal(), translated_literal, m_fluent_predicates, literals, static_literals, fluent_literals);
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return std::make_tuple(parameters, literals, static_literals, fluent_literals);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(condition_ptr))
    {
        auto literals = LiteralList {};
        auto static_literals = LiteralList {};
        auto fluent_literals = LiteralList {};

        const auto translated_literal = translate_lifted(*condition_literal->get_literal());

        func_insert_fluents(condition_literal->get_literal(), translated_literal, m_fluent_predicates, literals, static_literals, fluent_literals);

        return std::make_tuple(parameters, literals, static_literals, fluent_literals);
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

std::pair<EffectList, FunctionExpression> ToMimirStructures::translate_lifted(const loki::EffectImpl& effect)
{
    auto effect_ptr = &effect;

    // 1. Parse conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAndImpl>(effect_ptr))
    {
        auto result_effects = EffectList {};
        auto result_function_expressions = FunctionExpressionList {};
        for (const auto& nested_effect : effect_and->get_effects())
        {
            auto tmp_effect = nested_effect;

            // 2. Parse universal part
            auto parameters = ParameterList {};
            if (const auto& tmp_effect_forall = std::get_if<loki::EffectConditionalForallImpl>(tmp_effect))
            {
                parameters = translate_common(tmp_effect_forall->get_parameters());

                tmp_effect = tmp_effect_forall->get_effect();
            }

            // 3. Parse conditional part
            auto literals = LiteralList {};
            auto static_literals = LiteralList {};
            auto fluent_literals = LiteralList {};
            if (const auto& tmp_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(tmp_effect))
            {
                const auto [parameters_, literals_, static_literals, fluent_literals_] = translate_lifted(*tmp_effect_when->get_condition());

                if (!parameters_.empty())
                {
                    throw std::logic_error("Unexpected parameter in effect condition.");
                }

                tmp_effect = tmp_effect_when->get_effect();
            }

            // 4. Parse simple effect
            if (const auto& effect_literal = std::get_if<loki::EffectLiteralImpl>(tmp_effect))
            {
                const auto translated_effect = translate_lifted(*effect_literal->get_literal());

                result_effects.push_back(
                    m_pddl_factories.get_or_create_simple_effect(parameters, literals, static_literals, fluent_literals, translated_effect));
            }
            else if (const auto& effect_numeric = std::get_if<loki::EffectNumericImpl>(tmp_effect))
            {
                assert(effect_numeric->get_assign_operator() == loki::AssignOperatorEnum::INCREASE);
                assert(effect_numeric->get_function()->get_function_skeleton()->get_name() == "total-cost");

                result_function_expressions.push_back(this->translate_lifted(*effect_numeric->get_function_expression()));
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

                throw std::logic_error("Expected simple effect.");
            }
        }

        // If more than one action cost effects are given then we take their sum,
        // If one action cost is given then we take it,
        // and otherwise, we take cost 1.
        auto cost_function_expression =
            (result_function_expressions.empty()) ?
                this->m_pddl_factories.get_or_create_function_expression_number(1) :
            result_function_expressions.size() > 1 ?
                this->m_pddl_factories.get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum::PLUS, result_function_expressions) :
                result_function_expressions.front();

        return std::make_pair(result_effects, cost_function_expression);
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *effect_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive effect.");
}

Action ToMimirStructures::translate_lifted(const loki::ActionImpl& action)
{
    // Remove existential quantifier at the root and collecting its parameters
    auto parameters = translate_common(action.get_parameters());
    auto literals = LiteralList {};
    auto static_literals = LiteralList {};
    auto fluent_literals = LiteralList {};
    if (action.get_condition().has_value())
    {
        const auto [parameters_, literals_, static_literals_, fluent_literals_] = translate_lifted(*action.get_condition().value());
        literals = literals_;
        static_literals = static_literals_;
        fluent_literals = fluent_literals_;
        parameters.insert(parameters.end(), parameters_.begin(), parameters_.end());
    }

    // Default effects
    auto effects = EffectList {};
    auto function_expression = m_pddl_factories.get_or_create_function_expression_number(1);

    if (action.get_effect().has_value())
    {
        const auto [effects_, function_expression_] = translate_lifted(*action.get_effect().value());
        effects = effects_;
        function_expression = function_expression_;
    }

    return m_pddl_factories.get_or_create_action(action.get_name(), parameters, literals, static_literals, fluent_literals, effects, function_expression);
}

Axiom ToMimirStructures::translate_lifted(const loki::AxiomImpl& axiom)
{
    // Turn predicate arguments into parameters
    auto parameters = translate_common(axiom.get_literal()->get_atom()->get_predicate()->get_parameters());

    // Turn quantifier variables into parameters
    const auto [additional_parameters, literals, static_literals, fluent_literals] = translate_lifted(*axiom.get_condition());
    parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());

    // Turn free variables into parameters
    for (const auto& free_variable : collect_free_variables(*axiom.get_condition()))
    {
        parameters.push_back(m_pddl_factories.get_or_create_parameter(translate_common(*free_variable)));
    }
    parameters.shrink_to_fit();

    return m_pddl_factories.get_or_create_axiom(parameters, translate_lifted(*axiom.get_literal()), literals, static_literals, fluent_literals);
}

Domain ToMimirStructures::translate_lifted(const loki::DomainImpl& domain)
{
    const auto requirements = translate_common(*domain.get_requirements());
    const auto constants = translate_common(domain.get_constants());
    auto predicates = translate_common(domain.get_predicates());
    auto static_predicates = PredicateList {};
    for (const auto& predicate : domain.get_predicates())
    {
        if (!m_fluent_predicates.count(predicate))
        {
            static_predicates.push_back(translate_common(*predicate));
        }
    }
    const auto fluent_predicates = translate_common(loki::PredicateList(m_fluent_predicates.begin(), m_fluent_predicates.end()));
    const auto derived_predicates = translate_common(domain.get_derived_predicates());
    const auto functions = translate_lifted(domain.get_functions());
    const auto actions = translate_lifted(domain.get_actions());
    const auto axioms = translate_lifted(domain.get_axioms());

    // Add equal predicate that was hidden from predicate section
    // This must occur after translating all domain contents
    if (m_equal_predicate)
    {
        predicates.push_back(m_equal_predicate);
        static_predicates.push_back(m_equal_predicate);
    }

    return m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                 requirements,
                                                 constants,
                                                 predicates,
                                                 static_predicates,
                                                 fluent_predicates,
                                                 derived_predicates,
                                                 functions,
                                                 actions,
                                                 axioms);
}

/**
 * Grounded
 */

Object ToMimirStructures::translate_grounded(const loki::TermImpl& term)
{
    if (const auto term_object = std::get_if<loki::TermObjectImpl>(&term))
    {
        return translate_common(*term_object->get_object());
    }

    throw std::logic_error("Expected ground term.");
}

GroundAtom ToMimirStructures::translate_grounded(const loki::AtomImpl& atom)
{
    return m_pddl_factories.get_or_create_ground_atom(translate_common(*atom.get_predicate()), translate_grounded(atom.get_terms()));
}

GroundLiteral ToMimirStructures::translate_grounded(const loki::LiteralImpl& literal)
{
    return m_pddl_factories.get_or_create_ground_literal(literal.is_negated(), translate_grounded(*literal.get_atom()));
}

NumericFluent ToMimirStructures::translate_grounded(const loki::NumericFluentImpl& numeric_fluent)
{
    return m_pddl_factories.get_or_create_numeric_fluent(translate_grounded(*numeric_fluent.get_function()), numeric_fluent.get_number());
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionNumberImpl& function_expression)
{
    return m_pddl_factories.get_or_create_ground_function_expression_number(function_expression.get_number());
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    return m_pddl_factories.get_or_create_ground_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                                     translate_grounded(*function_expression.get_left_function_expression()),
                                                                                     translate_grounded(*function_expression.get_right_function_expression()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    return m_pddl_factories.get_or_create_ground_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                    translate_grounded(function_expression.get_function_expressions()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionMinusImpl& function_expression)
{
    return m_pddl_factories.get_or_create_ground_function_expression_minus(translate_grounded(*function_expression.get_function_expression()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    return m_pddl_factories.get_or_create_ground_function_expression_function(translate_grounded(*function_expression.get_function()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionImpl& function_expression)
{
    return std::visit([this](auto&& arg) { return this->translate_grounded(arg); }, function_expression);
}

GroundFunction ToMimirStructures::translate_grounded(const loki::FunctionImpl& function)
{
    return m_pddl_factories.get_or_create_ground_function(translate_lifted(*function.get_function_skeleton()), translate_grounded(function.get_terms()));
}

GroundLiteralList ToMimirStructures::translate_grounded(const loki::ConditionImpl& condition)
{
    auto condition_ptr = &condition;

    if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(condition_ptr))
    {
        auto literals = GroundLiteralList {};
        for (const auto& part : condition_and->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
            {
                literals.push_back(translate_grounded(*condition_literal->get_literal()));
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return literals;
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(condition_ptr))
    {
        return GroundLiteralList { translate_grounded(*condition_literal->get_literal()) };
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

OptimizationMetric ToMimirStructures::translate_grounded(const loki::OptimizationMetricImpl& optimization_metric)
{
    return m_pddl_factories.get_or_create_optimization_metric(optimization_metric.get_optimization_metric(),
                                                              translate_grounded(*optimization_metric.get_function_expression()));
}

Problem ToMimirStructures::translate_grounded(const loki::ProblemImpl& problem)
{
    // Add constants to objects in problem.
    const auto constants = translate_common(problem.get_domain()->get_constants());
    auto objects = translate_common(problem.get_objects());
    objects.insert(objects.end(), constants.begin(), constants.end());

    auto goal_literals = GroundLiteralList {};
    if (problem.get_goal_condition().has_value())
    {
        goal_literals = translate_grounded(*problem.get_goal_condition().value());
    }

    const auto translated_domain = translate_lifted(*problem.get_domain());

    auto initial_literals = translate_grounded(problem.get_initial_literals());
    // Add equal atoms, e.g., (= object1 object1)
    // This must occur after parsing the domain
    if (m_equal_predicate)
    {
        for (const auto& object : objects)
        {
            initial_literals.push_back(
                m_pddl_factories.get_or_create_ground_literal(false,
                                                              m_pddl_factories.get_or_create_ground_atom(m_equal_predicate, ObjectList { object, object })));
        }
    }
    // Derive static and fluent initial literals
    auto static_initial_literals = GroundLiteralList {};
    auto fluent_initial_literals = GroundLiteralList {};
    const auto static_predicates = PredicateSet(translated_domain->get_static_predicates().begin(), translated_domain->get_static_predicates().end());
    for (const auto& literal : initial_literals)
    {
        if (static_predicates.count(literal->get_atom()->get_predicate()))
        {
            static_initial_literals.push_back(literal);
        }
        else
        {
            fluent_initial_literals.push_back(literal);
        }
    }

    return m_pddl_factories.get_or_create_problem(translated_domain,
                                                  problem.get_name(),
                                                  translate_common(*problem.get_requirements()),
                                                  objects,
                                                  translate_common(problem.get_derived_predicates()),
                                                  initial_literals,
                                                  static_initial_literals,
                                                  fluent_initial_literals,
                                                  translate_grounded(problem.get_numeric_fluents()),
                                                  goal_literals,
                                                  (problem.get_optimization_metric().has_value() ?
                                                       std::optional<OptimizationMetric>(translate_grounded(*problem.get_optimization_metric().value())) :
                                                       std::nullopt),
                                                  translate_lifted(problem.get_axioms()));
}

Problem ToMimirStructures::run(const loki::ProblemImpl& problem)
{
    prepare(problem);
    return translate_grounded(problem);
}

ToMimirStructures::ToMimirStructures(PDDLFactories& pddl_factories) :
    m_pddl_factories(pddl_factories),
    m_fluent_predicates(),
    m_equal_predicate(nullptr),
    m_variable_to_parameter_index()
{
}

}
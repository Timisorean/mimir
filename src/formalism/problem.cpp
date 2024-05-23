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

#include "mimir/formalism/problem.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"

#include <cassert>
#include <iostream>
#include <loki/loki.hpp>

using namespace std;

namespace mimir
{
ProblemImpl::ProblemImpl(int identifier,
                         Domain domain,
                         std::string name,
                         Requirements requirements,
                         ObjectList objects,
                         PredicateList derived_predicates,
                         GroundLiteralList static_initial_literals,
                         GroundLiteralList fluent_initial_literals,
                         NumericFluentList numeric_fluents,
                         GroundLiteralList goal_condition,
                         std::optional<OptimizationMetric> optimization_metric,
                         AxiomList axioms) :
    Base(identifier),
    m_domain(std::move(domain)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_objects(std::move(objects)),
    m_derived_predicates(std::move(derived_predicates)),
    m_static_initial_literals(std::move(static_initial_literals)),
    m_fluent_initial_literals(std::move(fluent_initial_literals)),
    m_numeric_fluents(std::move(numeric_fluents)),
    m_goal_condition(std::move(goal_condition)),
    m_optimization_metric(std::move(optimization_metric)),
    m_axioms(std::move(axioms))
{
    assert(are_disjoint(m_static_initial_literals, m_fluent_initial_literals));
    assert(is_all_unique(m_objects));
    assert(is_all_unique(m_derived_predicates));
    assert(is_all_unique(m_static_initial_literals));
    assert(is_all_unique(m_fluent_initial_literals));
    assert(is_all_unique(m_numeric_fluents));
    assert(is_all_unique(m_goal_condition));
    assert(is_all_unique(m_axioms));

    // Initialize static atom bitsets
    for (const auto& literal : m_static_initial_literals)
    {
        if (literal->is_negated())
        {
            m_static_initial_negative_atoms_builder.set(literal->get_atom()->get_identifier());
        }
        else
        {
            m_static_initial_positive_atoms_builder.set(literal->get_atom()->get_identifier());
        }
    }
    m_static_initial_positive_atoms_builder.finish();
    m_static_initial_negative_atoms_builder.finish();
    // Ensure that buffer is correctly written
    assert(m_static_initial_positive_atoms_builder == get_static_initial_positive_atoms_bitset());
    assert(m_static_initial_negative_atoms_builder == get_static_initial_negative_atoms_bitset());

    std::cout << "Static initial literals: " << m_static_initial_literals << std::endl;
    std::cout << "Fluent initial literals: " << m_fluent_initial_literals << std::endl;
}

bool ProblemImpl::is_structurally_equivalent_to_impl(const ProblemImpl& other) const
{
    if (this != &other)
    {
        return (m_domain == other.m_domain) && (m_name == other.m_name) && (m_requirements == other.m_requirements)
               && (loki::get_sorted_vector(m_objects) == loki::get_sorted_vector(other.m_objects))
               && (loki::get_sorted_vector(m_derived_predicates) == loki::get_sorted_vector(other.m_derived_predicates))
               && (loki::get_sorted_vector(m_static_initial_literals)) == loki::get_sorted_vector(other.m_static_initial_literals)
               && (loki::get_sorted_vector(m_fluent_initial_literals)) == loki::get_sorted_vector(other.m_fluent_initial_literals)
               && (loki::get_sorted_vector(m_goal_condition)) == loki::get_sorted_vector(other.m_goal_condition)
               && (m_optimization_metric == other.m_optimization_metric) && (loki::get_sorted_vector(m_axioms) == loki::get_sorted_vector(other.m_axioms));
    }
    return true;
}

size_t ProblemImpl::hash_impl() const
{
    size_t optimization_hash = (m_optimization_metric.has_value()) ? loki::hash_combine(m_optimization_metric) : 0;
    return loki::hash_combine(m_domain,
                              m_name,
                              m_requirements,
                              loki::hash_container(loki::get_sorted_vector(m_objects)),
                              loki::hash_container(loki::get_sorted_vector(m_derived_predicates)),
                              loki::hash_container(loki::get_sorted_vector(m_static_initial_literals)),
                              loki::hash_container(loki::get_sorted_vector(m_fluent_initial_literals)),
                              loki::hash_container(loki::get_sorted_vector(m_goal_condition)),
                              optimization_hash,
                              loki::hash_container(loki::get_sorted_vector(m_axioms)));
}

void ProblemImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << string(options.indent, ' ') << "(define (problem " << m_name << ")\n";
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << string(nested_options.indent, ' ') << "(:domain " << m_domain->get_name() << ")\n";
    if (!m_requirements->get_requirements().empty())
    {
        out << string(nested_options.indent, ' ');
        m_requirements->str(out, nested_options);
        out << "\n";
    }

    if (!m_objects.empty())
    {
        out << string(nested_options.indent, ' ') << "(:objects ";
        for (size_t i = 0; i < m_objects.size(); ++i)
        {
            if (i != 0)
                out << " ";
            out << *m_objects[i];
        }
        out << ")\n";
    }

    if (!m_derived_predicates.empty())
    {
        out << string(nested_options.indent, ' ') << "(:derived-predicates ";
        for (size_t i = 0; i < m_derived_predicates.size(); ++i)
        {
            if (i != 0)
                out << " ";
            m_derived_predicates[i]->str(out, nested_options);
        }
        out << ")\n";
    }

    if (!(m_static_initial_literals.empty() && m_fluent_initial_literals.empty() && m_numeric_fluents.empty()))
    {
        out << string(nested_options.indent, ' ') << "(:init ";
        for (size_t i = 0; i < m_static_initial_literals.size(); ++i)
        {
            if (i != 0)
                out << " ";
            m_static_initial_literals[i]->str(out, nested_options);
        }
        for (size_t i = 0; i < m_fluent_initial_literals.size(); ++i)
        {
            out << " ";
            m_fluent_initial_literals[i]->str(out, nested_options);
        }
        for (size_t i = 0; i < m_numeric_fluents.size(); ++i)
        {
            out << " ";
            m_numeric_fluents[i]->str(out, nested_options);
        }
    }
    out << ")\n";

    if (!m_goal_condition.empty())
    {
        out << string(nested_options.indent, ' ') << "(:goal ";
        out << "(and ";
        for (size_t i = 0; i < m_goal_condition.size(); ++i)
        {
            if (i != 0)
            {
                out << " ";
            }
            out << *m_goal_condition[i];
        }
        out << ")\n";
    }

    if (m_optimization_metric.has_value())
    {
        out << string(nested_options.indent, ' ') << "(:metric ";
        m_optimization_metric.value()->str(out, nested_options);
        out << ")\n";
    }

    for (const auto& axiom : m_axioms)
    {
        axiom->str(out, nested_options);
    }

    out << string(options.indent, ' ') << ")";
}

const Domain& ProblemImpl::get_domain() const { return m_domain; }

const std::string& ProblemImpl::get_name() const { return m_name; }

const Requirements& ProblemImpl::get_requirements() const { return m_requirements; }

const ObjectList& ProblemImpl::get_objects() const { return m_objects; }

const PredicateList& ProblemImpl::get_derived_predicates() const { return m_derived_predicates; }

const GroundLiteralList& ProblemImpl::get_static_initial_literals() const { return m_static_initial_literals; }

FlatBitset ProblemImpl::get_static_initial_positive_atoms_bitset() const { return FlatBitset(m_static_initial_positive_atoms_builder.buffer().data()); }

FlatBitset ProblemImpl::get_static_initial_negative_atoms_bitset() const { return FlatBitset(m_static_initial_negative_atoms_builder.buffer().data()); }

const GroundLiteralList& ProblemImpl::get_fluent_initial_literals() const { return m_fluent_initial_literals; }

const NumericFluentList& ProblemImpl::get_numeric_fluents() const { return m_numeric_fluents; }

const GroundLiteralList& ProblemImpl::get_goal_condition() const { return m_goal_condition; }

const std::optional<OptimizationMetric>& ProblemImpl::get_optimization_metric() const { return m_optimization_metric; }

const AxiomList& ProblemImpl::get_axioms() const { return m_axioms; }

}

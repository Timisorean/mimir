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

#include "mimir/formalism/metric.hpp"

#include "formatter.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"

#include <cassert>

using namespace std;

namespace mimir
{
OptimizationMetricImpl::OptimizationMetricImpl(size_t index, loki::OptimizationMetricEnum optimization_metric, GroundFunctionExpression function_expression) :
    m_index(index),
    m_optimization_metric(optimization_metric),
    m_function_expression(std::move(function_expression))
{
}

std::string OptimizationMetricImpl::str() const
{
    auto out = std::stringstream();
    out << *this;
    return out.str();
}

size_t OptimizationMetricImpl::get_index() const { return m_index; }

loki::OptimizationMetricEnum OptimizationMetricImpl::get_optimization_metric() const { return m_optimization_metric; }

const GroundFunctionExpression& OptimizationMetricImpl::get_function_expression() const { return m_function_expression; }

std::ostream& operator<<(std::ostream& out, const OptimizationMetricImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

}

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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_BASE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_BASE_HPP_

#include "mimir/languages/description_logics/constructor_interface.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * Concept
 */

template<IsConceptOrRole D, typename Derived_>
class ConstructorEvaluatorBase : public ConstructorImpl<D>
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    Denotation<D> evaluate(EvaluationContext& context) const override
    {
        // Try to access cached result
        auto denotation = context.get_denotation_repository<D>().get_if(this, context.get_state_index());
        if (denotation.has_value())
        {
            return denotation.value();
        }

        // Compute the result in the derived class
        self().evaluate_impl(context);

        // Store and return result;
        return context.get_denotation_repository<D>().insert(this, context.get_state_index(), context.get_denotation_builder<D>());
    };

    bool accept(const grammar::Visitor<D>& visitor) const override { return self().accept_impl(visitor); }
};

}

#endif

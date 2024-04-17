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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TO_EFFECT_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TO_EFFECT_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

/**
 * Translate to effect normal form:
 * https://gki.informatik.uni-freiburg.de/teaching/ss05/aip/aip02.pdf
 */
class ToENFTranslator : public BaseTranslator<ToENFTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<ToENFTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    /**
     * 1. e and (phi > e)          =>  e                                                // ok
     * 2. e1 and (e2 and e3)       =>  e1 and e2 and e3                                 // ok
     */
    loki::Effect translate_impl(const loki::EffectAndImpl& effect);
    /**
     * 3. forall(vars1, forall(vars2, e))    =>  forall(vars1+vars2, e)                  // flatten forall
     * 4. forall(vars, (e1 and e2))          => forall(vars, e1) and forall(vars, e2)    // move conjunction over forall
     */
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect);
    /**
     * 5. phi > (psi > e)    =>  (phi and psi) > e                                       // flatten when
     * 6. phi > (e1 and e2)  =>  (phi > e1) and (phi > e2)                               // move conjunction over when
     * 7. phi > forall(vars, e)  => forall(vars, phi > e)
     * 8. exists(vars, phi) > e  => forall(vars, phi > e)
     */
    loki::Effect translate_impl(const loki::EffectConditionalWhenImpl& effect);
    /**
     * Flatten conjunctions.
     *
     * 9. phi and (psi and chi)  =>  phi and psi and chi
     */
    loki::Condition translate_impl(const loki::ConditionAndImpl& condition);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit ToENFTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif

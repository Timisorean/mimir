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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_VISITOR_INTERFACE_HPP_

#include "mimir/formalism/predicate.hpp"

namespace mimir::dl
{

/**
 * Forward declarations
 */

template<PredicateCategory P>
class ConceptPredicate;
class ConceptAnd;

/**
 * Visitors
 */

class ConceptVisitor
{
public:
    virtual ~ConceptVisitor() {}

    // Default implementations always return false

    /* Concepts */
    virtual bool accept(const ConceptPredicate<Static>& constructor) const { return false; }
    virtual bool accept(const ConceptPredicate<Fluent>& constructor) const { return false; }
    virtual bool accept(const ConceptPredicate<Derived>& constructor) const { return false; }
    virtual bool accept(const ConceptAnd& constructor) const { return false; }
};

class RoleVisitor
{
public:
    virtual ~RoleVisitor() {}

    // Default implementations always return false

    /* Roles */
};

}

#endif

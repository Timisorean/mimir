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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTORS_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"
#include "mimir/languages/description_logics/grammar_constructors_interface.hpp"

namespace mimir::dl
{
template<typename T>
class ConstructorRepository;

namespace grammar
{
/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public Constructor<dl::Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const ConceptPredicateState& other) const;
    size_t hash() const;

    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<dl::Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const ConceptPredicateGoal& other) const;
    size_t hash() const;

    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

class ConceptAnd : public Constructor<dl::Concept>
{
private:
    size_t m_id;
    const Concept* m_concept_left;
    const Concept* m_concept_right;

    ConceptAnd(size_t id, const Concept& concept_left, const Concept& concept_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const ConceptAnd& other) const;
    size_t hash() const;

    bool test_match(const dl::Concept& constructor) const override;

    const Concept& get_concept_left() const;
    const Concept& get_concept_right() const;

    size_t get_id() const override;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateState : public Constructor<dl::Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const RolePredicateState& other) const;
    size_t hash() const;

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

template<PredicateCategory P>
class RolePredicateGoal : public Constructor<dl::Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const RolePredicateGoal& other) const;
    size_t hash() const;

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

class RoleAnd : public Constructor<dl::Role>
{
private:
    size_t m_id;
    const Role* m_role_left;
    const Role* m_role_right;

    RoleAnd(size_t id, const Role& role_left, const Role& role_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const RoleAnd& other) const;
    size_t hash() const;

    bool test_match(const dl::Role& constructor) const override;

    const Role& get_role_left() const;
    const Role& get_role_right() const;

    size_t get_id() const override;
};

}
}

#endif

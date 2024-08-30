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

#ifndef MIMIR_SEARCH_AXIOM_HPP_
#define MIMIR_SEARCH_AXIOM_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/flat_types.hpp"

#include <cstdint>
#include <ostream>
#include <tuple>

namespace mimir
{

/**
 * Flatmemory types
 */

struct FlatDerivedEffect
{
    bool is_negated;
    size_t atom_id;

    bool operator==(const FlatDerivedEffect& other) const;
};

using FlatAxiom = cista::tuple<GroundAxiomIndex,  //
                               uint32_t,          // AxiomIndex
                               FlatIndexList,     // ObjectIndices
                               FlatStripsActionPrecondition,
                               FlatStripsActionEffect,
                               FlatDerivedEffect>;

using FlatAxiomVector = cista::offset::vector<FlatAxiom>;
}

template<>
struct cista::storage::DerefStdHasher<mimir::FlatAxiom>
{
    size_t operator()(const mimir::FlatAxiom* ptr) const;
};

template<>
struct cista::storage::DerefStdEqualTo<mimir::FlatAxiom>
{
    bool operator()(const mimir::FlatAxiom* lhs, const mimir::FlatAxiom* rhs) const;
};

namespace mimir
{
using FlatAxiomSet = cista::storage::UnorderedSet<FlatAxiom>;

/**
 * GroundAxiomBuilder
 */
class GroundAxiomBuilder
{
private:
    FlatAxiom m_builder;

public:
    FlatAxiom& get_data();
    const FlatAxiom& get_data() const;

    GroundAxiomIndex& get_index();
    uint32_t& get_axiom();
    FlatIndexList& get_objects();
    /* STRIPS part */
    FlatStripsActionPrecondition& get_strips_precondition();
    FlatStripsActionEffect& get_strips_effect();
    /* Simple effect */
    FlatDerivedEffect& get_derived_effect();
};

/**
 * GroundAxiom
 */
class GroundAxiom
{
private:
    const FlatAxiom* m_view;

public:
    /// @brief Create a view on a Axiom.
    explicit GroundAxiom(const FlatAxiom* view);

    GroundAxiomIndex get_index() const;
    uint32_t get_axiom() const;
    const FlatIndexList& get_objects() const;

    /* STRIPS part */
    const FlatStripsActionPrecondition& get_strips_precondition() const;
    const FlatStripsActionEffect& get_strips_effect() const;
    /* Effect*/
    const FlatDerivedEffect& get_derived_effect() const;

    bool is_applicable(const FlatBitset& state_fluent_atoms, const FlatBitset& state_derived_atoms, const FlatBitset& static_positive_atoms) const;

    bool is_statically_applicable(const FlatBitset& static_positive_bitset) const;
};

extern bool operator==(GroundAxiom lhs, GroundAxiom rhs);

}

template<>
struct std::hash<mimir::GroundAxiom>
{
    size_t operator()(mimir::GroundAxiom element) const;
};

template<>
struct std::equal_to<mimir::GroundAxiom>
{
    size_t operator()(mimir::GroundAxiom lhs, mimir::GroundAxiom rhs) const;
};

namespace mimir
{

/**
 * Mimir types
 */

using GroundAxiomList = std::vector<GroundAxiom>;
using GroundAxiomSet = std::unordered_set<GroundAxiom>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<FlatDerivedEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAxiom, const PDDLFactories&>& data);
}

#endif

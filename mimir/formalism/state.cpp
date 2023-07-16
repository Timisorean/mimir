/*
 * Copyright (C) 2023 Simon Stahlberg
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


#include "state.hpp"

#include "help_functions.hpp"
#include "problem.hpp"

#include <algorithm>
#include <boost/functional/hash.hpp>
#include <cassert>

namespace formalism
{
    StateImpl::StateImpl(const std::vector<uint32_t>& dynamic_bitset, const formalism::ProblemDescription& problem) :
        ranks_(dynamic_bitset),
        problem_(problem),
        hash_(0)
    {
        boost::hash_combine(hash_, (std::size_t) problem_.get());
        boost::hash_combine(hash_, hash_vector(ranks_));
    }

    StateImpl::StateImpl() : ranks_(), problem_(nullptr), hash_(0) {}

    StateImpl::StateImpl(const formalism::AtomList& atoms, const formalism::ProblemDescription& problem) : ranks_(), problem_(problem), hash_(0)
    {
        for (const auto& atom : atoms)
        {
            ranks_.emplace_back(problem->get_rank(atom));
        }

        std::sort(ranks_.begin(), ranks_.end());

        boost::hash_combine(hash_, (std::size_t) problem_.get());
        boost::hash_combine(hash_, hash_vector(ranks_));
    }

    StateImpl::StateImpl(const formalism::AtomSet& atoms, const formalism::ProblemDescription& problem) : ranks_(), problem_(problem), hash_(0)
    {
        for (const auto& atom : atoms)
        {
            ranks_.emplace_back(problem->get_rank(atom));
        }

        boost::hash_combine(hash_, (std::size_t) problem_.get());
        boost::hash_combine(hash_, hash_vector(ranks_));
    }

    State create_state() { return std::make_shared<formalism::StateImpl>(); }

    State create_state(const formalism::AtomList& atoms, formalism::ProblemDescription problem)
    {
        return std::make_shared<formalism::StateImpl>(atoms, problem);
    }

    State create_state(const formalism::AtomSet& atoms, formalism::ProblemDescription problem)
    {
        return std::make_shared<formalism::StateImpl>(atoms, problem);
    }

    inline bool StateImpl::operator<(const StateImpl& other) const
    {
        if (problem_ != other.problem_)
        {
            return problem_ < other.problem_;
        }

        return ranks_ < other.ranks_;
    }

    inline bool StateImpl::operator==(const StateImpl& other) const
    {
        if (hash_ != other.hash_)
        {
            return false;
        }

        if (problem_.get() != other.problem_.get())
        {
            return false;
        }

        return ranks_ == other.ranks_;
    }

    inline bool StateImpl::operator!=(const StateImpl& other) const { return !(this->operator==(other)); }

    formalism::AtomList StateImpl::get_atoms() const
    {
        formalism::AtomList atoms;

        for (const auto rank : ranks_)
        {
            atoms.emplace_back(problem_->get_atom(rank));
        }

        return atoms;
    }

    formalism::AtomList StateImpl::get_static_atoms() const
    {
        formalism::AtomList atoms;

        for (const auto rank : ranks_)
        {
            if (problem_->is_static(rank))
            {
                atoms.emplace_back(problem_->get_atom(rank));
            }
        }

        return atoms;
    }

    formalism::AtomList StateImpl::get_dynamic_atoms() const
    {
        formalism::AtomList atoms;

        for (const auto rank : ranks_)
        {
            if (problem_->is_dynamic(rank))
            {
                atoms.emplace_back(problem_->get_atom(rank));
            }
        }

        return atoms;
    }

    std::vector<uint32_t> StateImpl::get_ranks() const { return ranks_; }

    std::vector<uint32_t> StateImpl::get_static_ranks() const
    {
        std::vector<uint32_t> static_atoms;
        std::copy_if(ranks_.begin(), ranks_.end(), std::back_insert_iterator(static_atoms), [this](uint32_t rank) { return problem_->is_static(rank); });
        return static_atoms;
    }

    std::vector<uint32_t> StateImpl::get_dynamic_ranks() const
    {
        std::vector<uint32_t> dynamic_atoms;
        std::copy_if(ranks_.begin(), ranks_.end(), std::back_insert_iterator(dynamic_atoms), [this](uint32_t rank) { return problem_->is_dynamic(rank); });
        return dynamic_atoms;
    }

    formalism::ProblemDescription StateImpl::get_problem() const { return problem_; }

    std::map<formalism::Predicate, formalism::AtomList> StateImpl::get_atoms_grouped_by_predicate() const
    {
        std::map<formalism::Predicate, formalism::AtomList> grouped_atoms;

        for (const auto& atom : get_atoms())
        {
            const auto& predicate = atom->predicate;

            if (grouped_atoms.find(predicate) == grouped_atoms.end())
            {
                grouped_atoms.insert(std::make_pair(predicate, formalism::AtomList()));
            }

            auto& atom_list = grouped_atoms.at(predicate);
            atom_list.push_back(atom);
        }

        return grouped_atoms;
    }

    std::pair<std::map<uint32_t, std::vector<uint32_t>>, std::map<uint32_t, std::pair<std::string, uint32_t>>>
    StateImpl::pack_object_ids_by_predicate_id(bool include_types, bool include_goal) const
    {
        std::map<uint32_t, std::vector<uint32_t>> packed_ids;
        std::map<uint32_t, std::pair<std::string, uint32_t>> id_to_name_arity;

        for (const auto& atom : get_atoms())
        {
            const auto& predicate = atom->predicate;

            if (packed_ids.find(predicate->id) == packed_ids.end())
            {
                packed_ids.insert(std::make_pair(predicate->id, std::vector<uint32_t>()));
                id_to_name_arity.insert(std::make_pair(predicate->id, std::make_pair(predicate->name, predicate->arity)));
            }

            auto& atom_list = packed_ids.at(predicate->id);

            for (const auto& object : atom->arguments)
            {
                atom_list.push_back(object->id);
            }
        }

        auto num_predicates = problem_->domain->predicates.size();

        if (include_types)
        {
            std::map<formalism::Type, uint32_t> type_ids;
            const auto& types = problem_->domain->types;

            for (const auto& type : types)
            {
                type_ids.insert(std::make_pair(type, num_predicates + type_ids.size()));
            }

            for (const auto& object : problem_->objects)
            {
                const auto object_id = object->id;
                auto type = object->type;

                while (type != nullptr)
                {
                    const auto predicate_id = type_ids[type];

                    if (packed_ids.find(predicate_id) == packed_ids.end())
                    {
                        packed_ids.insert(std::make_pair(predicate_id, std::vector<uint32_t>()));
                        id_to_name_arity.insert(std::make_pair(predicate_id, std::make_pair(type->name + "_type", 1)));
                    }

                    auto& atom_list = packed_ids.at(predicate_id);
                    atom_list.push_back(object_id);
                    type = type->base;
                }
            }

            num_predicates += types.size();
        }

        if (include_goal)
        {
            for (const auto& literal : problem_->goal)
            {
                if (literal->negated)
                {
                    throw std::invalid_argument("negated literal in the goal");
                }

                const auto atom = literal->atom;
                const auto predicate_id = num_predicates + atom->predicate->id;

                if (packed_ids.find(predicate_id) == packed_ids.end())
                {
                    packed_ids.insert(std::make_pair(predicate_id, std::vector<uint32_t>()));
                    id_to_name_arity.insert(std::make_pair(predicate_id, std::make_pair(atom->predicate->name + "_goal", atom->predicate->arity)));
                }

                auto& atom_list = packed_ids.at(predicate_id);

                for (const auto& object : atom->arguments)
                {
                    atom_list.push_back(object->id);
                }
            }
        }

        return std::make_pair(packed_ids, id_to_name_arity);
    }

    bool is_in_state(uint32_t rank, const formalism::State& state)
    {
        const auto it = std::lower_bound(state->ranks_.cbegin(), state->ranks_.cend(), rank);
        return (it != state->ranks_.cend() && *it == rank);
    }

    bool is_in_state(const formalism::Atom& atom, const formalism::State& state) { return is_in_state(state->get_problem()->get_rank(atom), state); }

    bool is_applicable(const formalism::Action& action, const formalism::State& state)
    {
        if (static_cast<int32_t>(action->get_arguments().size()) != action->schema->arity)
        {
            throw std::runtime_error("is_applicable: action is not ground");
        }

        const auto& ranks = state->ranks_;
        const auto& neg_ranks = action->negative_precondition_ranks_;
        const auto& pos_ranks = action->positive_precondition_ranks_;

        const auto neg_precond = std::find_first_of(ranks.begin(), ranks.end(), neg_ranks.begin(), neg_ranks.end()) == ranks.end();

        if (!neg_precond)
        {
            return false;
        }

        const auto pos_precond =
            std::all_of(pos_ranks.begin(), pos_ranks.end(), [&](int elem) { return std::binary_search(ranks.begin(), ranks.end(), elem); });

        return pos_precond;
    }

    formalism::State apply(const formalism::Action& action, const formalism::State& state)
    {
        const auto& ranks = state->ranks_;
        const auto reserve_size = ranks.size() + action->positive_effect_ranks_.size();

        std::vector<uint32_t> ranks_delete;
        ranks_delete.reserve(reserve_size);

        std::set_difference(ranks.begin(),
                            ranks.end(),
                            action->negative_effect_ranks_.begin(),
                            action->negative_effect_ranks_.end(),
                            std::back_inserter(ranks_delete));

        std::vector<uint32_t> ranks_add;
        ranks_add.reserve(reserve_size);

        std::set_union(ranks_delete.begin(),
                       ranks_delete.end(),
                       action->positive_effect_ranks_.begin(),
                       action->positive_effect_ranks_.end(),
                       std::back_inserter(ranks_add));

        return std::make_shared<formalism::StateImpl>(std::move(ranks_add), state->problem_);
    }

    bool atoms_hold(const AtomList& atoms, const formalism::State& state)
    {
        for (const auto& atom : atoms)
        {
            if (!formalism::is_in_state(atom, state))
            {
                return false;
            }
        }

        return true;
    }

    bool literal_holds(const formalism::Literal& literal, const formalism::State& state)
    {
        return formalism::is_in_state(literal->atom, state) != literal->negated;
    }

    bool literals_hold(const LiteralList& literal_list, const formalism::State& state, std::size_t min_arity)
    {
        for (const auto& literal : literal_list)
        {
            const auto predicate_arity = literal->atom->predicate->arity;

            if ((predicate_arity >= min_arity) && !formalism::literal_holds(literal, state))
            {
                return false;
            }
        }

        return true;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::State& state)
    {
        os << state->get_atoms();
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::StateList& states)
    {
        print_vector(os, states);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::State>::operator()(const formalism::State& state) const { return !state ? 0 : state->hash(); }

    bool less<formalism::State>::operator()(const formalism::State& left_state, const formalism::State& right_state) const
    {
        return *left_state < *right_state;
    }

    bool equal_to<formalism::State>::operator()(const formalism::State& left_state, const formalism::State& right_state) const
    {
        if (left_state.get() == right_state.get())
        {
            return true;
        }

        if (!left_state || !right_state)
        {
            return false;
        }

        return *left_state == *right_state;
    }
}  // namespace std

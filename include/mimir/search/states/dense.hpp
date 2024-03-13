#ifndef MIMIR_SEARCH_STATES_DENSE_HPP_
#define MIMIR_SEARCH_STATES_DENSE_HPP_

#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/search/states/interface.hpp"
#include "mimir/search/types.hpp"

#include <flatmemory/flatmemory.hpp>

namespace mimir
{
/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
class DenseStateTag : public StateTag
{
};

/**
 * Flatmemory types
 */
using DenseStateLayout = flatmemory::Tuple<uint32_t, BitsetLayout>;
using DenseStateBuilder = flatmemory::Builder<DenseStateLayout>;
using ConstDenseStateView = flatmemory::ConstView<DenseStateLayout>;

struct ConstDenseStateViewHash
{
    size_t operator()(const ConstDenseStateView& view) const
    {
        const auto bitset_view = view.get<1>();
        return bitset_view.hash();
    }
};

struct ConstDenseStateViewEqual
{
    bool operator()(const ConstDenseStateView& view_left, const ConstDenseStateView& view_right) const
    {
        const auto bitset_view_left = view_left.get<1>();
        const auto bitset_view_right = view_left.get<1>();
        return bitset_view_left == bitset_view_right;
    }
};

using DenseStateSet = flatmemory::UnorderedSet<DenseStateLayout, ConstDenseStateViewHash, ConstDenseStateViewEqual>;

/**
 * Implementation class
 */
template<>
class Builder<StateDispatcher<DenseStateTag>> :
    public IBuilder<Builder<StateDispatcher<DenseStateTag>>>,
    public IStateBuilder<Builder<StateDispatcher<DenseStateTag>>>
{
private:
    DenseStateBuilder m_builder;

    /* Implement IBuilder interface */
    template<typename>
    friend class IBuilder;

    [[nodiscard]] DenseStateBuilder& get_flatmemory_builder_impl() { return m_builder; }
    [[nodiscard]] const DenseStateBuilder& get_flatmemory_builder_impl() const { return m_builder; }

    /* Implement IStateBuilder interface */
    template<typename>
    friend class IStateBuilder;

    [[nodiscard]] uint32_t& get_id_impl() { return m_builder.get<0>(); }

public:
    [[nodiscard]] BitsetBuilder& get_atoms_bitset() { return m_builder.get<1>(); }
};

/**
 * Implementation class
 *
 * Reads the memory layout generated by the lifted state builder.
 */
template<>
class ConstView<StateDispatcher<DenseStateTag>> :
    public IConstView<ConstView<StateDispatcher<DenseStateTag>>>,
    public IStateView<ConstView<StateDispatcher<DenseStateTag>>>
{
private:
    ConstDenseStateView m_view;

    /* Implement IView interface */
    template<typename>
    friend class IConstView;

    [[nodiscard]] bool are_equal_impl(const ConstView& other) const { return get_atoms_bitset() == other.get_atoms_bitset(); }

    [[nodiscard]] size_t hash_impl() const { return get_atoms_bitset().hash(); }

    /* Implement IStateView interface */
    template<typename>
    friend class IStateView;

    [[nodiscard]] uint32_t get_id_impl() const { return m_view.get<0>(); }

public:
    explicit ConstView(ConstDenseStateView view) : m_view(view) {}

    [[nodiscard]] ConstBitsetView get_atoms_bitset() const { return m_view.get<1>(); }

    bool contains(const GroundAtom& ground_atom) const { return get_atoms_bitset().get(ground_atom->get_identifier()); }

    bool literal_holds(const GroundLiteral& literal) const { return literal->is_negated() != contains(literal->get_atom()); }

    bool literals_hold(const GroundLiteralList& literals) const
    {
        for (const auto& literal : literals)
        {
            if (!literal_holds(literal))
            {
                return false;
            }
        }

        return true;
    }
};

/**
 * Mimir types
 */
using DenseStateBuilderProxy = Builder<StateDispatcher<DenseStateTag>>;

template<IsPlanningModeTag P>
using ConstDenseStateViewProxy = ConstView<StateDispatcher<DenseStateTag>>;

}

#endif

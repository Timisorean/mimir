#ifndef MIMIR_SEARCH_ACTIONS_DENSE_HPP_
#define MIMIR_SEARCH_ACTIONS_DENSE_HPP_

#include "mimir/search/actions/interface.hpp"
#include "mimir/search/builder.hpp"
#include "mimir/search/states/dense.hpp"
#include "mimir/search/type_traits.hpp"
#include "mimir/search/types.hpp"
#include "mimir/search/view_const.hpp"

namespace mimir
{
/**
 * Flatmemory types
 */
using FlatDenseActionLayout = flatmemory::Tuple<uint32_t,
                                                int32_t,
                                                Action,
                                                FlatObjectListLayout,
                                                FlatBitsetLayout,
                                                FlatBitsetLayout,
                                                FlatBitsetLayout,
                                                FlatBitsetLayout,
                                                FlatBitsetVectorLayout,
                                                FlatBitsetVectorLayout,
                                                FlatInt32tVectorLayout>;
using FlatDenseActionBuilder = flatmemory::Builder<FlatDenseActionLayout>;
using FlatDenseAction = flatmemory::ConstView<FlatDenseActionLayout>;
using FlatDenseActionVector = flatmemory::VariableSizedTypeVector<FlatDenseActionLayout>;

struct FlatDenseActionHash
{
    size_t operator()(const FlatDenseAction& view) const
    {
        const auto action = view.get<2>();
        const auto objects = view.get<3>();
        return loki::hash_combine(action, objects.hash());
    }
};

struct FlatDenseActionEqual
{
    bool operator()(const FlatDenseAction& view_left, const FlatDenseAction& view_right) const
    {
        const auto action_left = view_left.get<2>();
        const auto objects_left = view_left.get<3>();
        const auto action_right = view_right.get<2>();
        const auto objects_right = view_right.get<3>();
        return (action_left == action_right) && (objects_left == objects_right);
    }
};

using FlatDenseActionSet = flatmemory::UnorderedSet<FlatDenseActionLayout, FlatDenseActionHash, FlatDenseActionEqual>;

/**
 * Implementation class
 */
template<>
class Builder<ActionDispatcher<DenseStateTag>> :
    public IBuilder<Builder<ActionDispatcher<DenseStateTag>>>,
    public IActionBuilder<Builder<ActionDispatcher<DenseStateTag>>>
{
private:
    FlatDenseActionBuilder m_builder;

    /* Implement IBuilder interface */
    friend class IBuilder<Builder<ActionDispatcher<DenseStateTag>>>;

    [[nodiscard]] FlatDenseActionBuilder& get_flatmemory_builder_impl() { return m_builder; }
    [[nodiscard]] const FlatDenseActionBuilder& get_flatmemory_builder_impl() const { return m_builder; }

    /* Implement IActionBuilder interface */
    friend class IActionBuilder<Builder<ActionDispatcher<DenseStateTag>>>;

public:
    /// @brief Modify the data, call finish, then copy the buffer to a container and use its returned view.
    [[nodiscard]] uint32_t& get_id() { return m_builder.get<0>(); }
    [[nodiscard]] int32_t& get_cost() { return m_builder.get<1>(); }
    [[nodiscard]] Action& get_action() { return m_builder.get<2>(); }
    [[nodiscard]] FlatObjectListBuilder& get_objects() { return m_builder.get<3>(); }
    /* Precondition */
    [[nodiscard]] FlatBitsetBuilder& get_applicability_positive_precondition_bitset() { return m_builder.get<4>(); }
    [[nodiscard]] FlatBitsetBuilder& get_applicability_negative_precondition_bitset() { return m_builder.get<5>(); }
    /* Simple effects */
    [[nodiscard]] FlatBitsetBuilder& get_unconditional_positive_effect_bitset() { return m_builder.get<6>(); }
    [[nodiscard]] FlatBitsetBuilder& get_unconditional_negative_effect_bitset() { return m_builder.get<7>(); }
    /* Conditional effects */
    [[nodiscard]] FlatBitsetVectorBuilder& get_conditional_positive_precondition_bitsets() { return m_builder.get<8>(); }
    [[nodiscard]] FlatBitsetVectorBuilder& get_conditional_negative_precondition_bitsets() { return m_builder.get<9>(); }
    // We use positive numbers for add and negative numbers for delete effects
    [[nodiscard]] FlatInt32tVectorBuilder& get_conditional_effects() { return m_builder.get<10>(); }
};

/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
 */
template<>
class ConstView<ActionDispatcher<DenseStateTag>> :
    public IConstView<ConstView<ActionDispatcher<DenseStateTag>>>,
    public IActionView<ConstView<ActionDispatcher<DenseStateTag>>>
{
private:
    using DenseState = ConstView<StateDispatcher<DenseStateTag>>;

    FlatDenseAction m_view;

    /* Implement IView interface: */
    friend class IConstView<ConstView<ActionDispatcher<DenseStateTag>>>;

    /// @brief Compute equality based on the lifted action and the objects assigned to the parameters.
    [[nodiscard]] bool are_equal_impl(const ConstView& other) const { return get_action() == other.get_action() && get_objects() == other.get_objects(); }

    /// @brief Compute hash based on the lifted action and the objects assigned to the parameters.
    [[nodiscard]] size_t hash_impl() const { return loki::hash_combine(get_action(), get_objects().hash()); }

    /* Implement IActionView interface */
    friend class IActionView<ConstView<ActionDispatcher<DenseStateTag>>>;

public:
    /// @brief Create a view on a DefaultAction.
    explicit ConstView(FlatDenseAction view) : m_view(view) {}

    [[nodiscard]] uint32_t get_id() const { return m_view.get<0>(); }
    [[nodiscard]] int32_t get_cost() const { return m_view.get<1>(); }
    [[nodiscard]] Action get_action() const { return m_view.get<2>(); }
    [[nodiscard]] FlatObjectList get_objects() const { return m_view.get<3>(); }
    /* Precondition */
    [[nodiscard]] FlatBitset get_applicability_positive_precondition_bitset() const { return m_view.get<4>(); }
    [[nodiscard]] FlatBitset get_applicability_negative_precondition_bitset() const { return m_view.get<5>(); }
    /* Simple effects */
    [[nodiscard]] FlatBitset get_unconditional_positive_effect_bitset() const { return m_view.get<6>(); };
    [[nodiscard]] FlatBitset get_unconditional_negative_effect_bitset() const { return m_view.get<7>(); };
    /* Conditional effects */
    [[nodiscard]] FlatBitsetVector get_conditional_positive_precondition_bitsets() const { return m_view.get<8>(); }
    [[nodiscard]] FlatBitsetVector get_conditional_negative_precondition_bitsets() const { return m_view.get<9>(); }
    [[nodiscard]] FlatInt32tVector get_conditional_effects() const { return m_view.get<10>(); }

    [[nodiscard]] bool is_applicable(DenseState state) const
    {
        const auto state_bitset = state.get_atoms_bitset();
        return state_bitset.is_superseteq(get_applicability_positive_precondition_bitset())
               && state_bitset.are_disjoint(get_applicability_negative_precondition_bitset());
    }
};

/**
 * Mimir types
 */
using DenseActionBuilder = Builder<ActionDispatcher<DenseStateTag>>;
using DenseAction = ConstView<ActionDispatcher<DenseStateTag>>;
using DenseActionList = std::vector<DenseAction>;

}

#endif

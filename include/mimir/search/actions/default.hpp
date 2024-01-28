#ifndef MIMIR_SEARCH_ACTIONS_DEFAULT_HPP_
#define MIMIR_SEARCH_ACTIONS_DEFAULT_HPP_

#include "template.hpp"

#include "../../buffer/flatbuffers/action-default_generated.h"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class DefaultActionTag : public ActionBaseTag {};


/**
 * Dispatcher class.
 *
 * Define the required template arguments of your implementation.
*/
template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct is_action_dispatcher<ActionDispatcher<A, P, S>> : std::true_type {};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Builder<ActionDispatcher<DefaultActionTag, P, S>>> {
    using PlanningModeTag = P;
    using StateTag = S;
};

template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<View<ActionDispatcher<DefaultActionTag, P, S>>> {
    using PlanningModeTag = P;
    using StateTag = S;
};


/**
 * Implementation class
*/
template<IsPlanningModeTag P, IsStateTag S>
class Builder<ActionDispatcher<DefaultActionTag, P, S>>
    : public BuilderBase<Builder<ActionDispatcher<DefaultActionTag, P, S>>>
    , public ActionBuilderBase<Builder<ActionDispatcher<DefaultActionTag, P, S>>> {
private:
    flatbuffers::FlatBufferBuilder m_flatbuffers_builder;

    // The bitset data
    BitsetBuilder<uint64_t> m_applicability_positive_precondition_bitset;
    BitsetBuilder<uint64_t> m_applicability_negative_precondition_bitset;
    BitsetBuilder<uint64_t> m_unconditional_positive_effect_bitset;
    BitsetBuilder<uint64_t> m_unconditional_negative_effect_bitset;

    /* Implement BuilderBase interface */
    template<typename>
    friend class BuilderBase;

    void finish_impl() {
        auto builder = DefaultActionFlatBuilder(this->m_flatbuffers_builder);
        this->m_flatbuffers_builder.FinishSizePrefixed(builder.Finish());
    }

    void clear_impl() {
        m_flatbuffers_builder.Clear();
    }

    [[nodiscard]] uint8_t* get_buffer_pointer_impl() { return m_flatbuffers_builder.GetBufferPointer(); }
    [[nodiscard]] const uint8_t* get_buffer_pointer_impl() const { return m_flatbuffers_builder.GetBufferPointer(); }
    [[nodiscard]] uint32_t get_size_impl() const { return *reinterpret_cast<const flatbuffers::uoffset_t*>(this->get_buffer_pointer()) + sizeof(flatbuffers::uoffset_t); }

    /* Implement ActionBuilderBase interface */
    template<typename>
    friend class ActionBuilderBase;
};



/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
*/
template<IsPlanningModeTag P, IsStateTag S>
class View<ActionDispatcher<DefaultActionTag, P, S>>
    : public ViewBase<View<ActionDispatcher<DefaultActionTag, P, S>>>
    , public ActionViewBase<View<ActionDispatcher<DefaultActionTag, P, S>>> {
private:
    const DefaultActionFlat* m_flatbuffers_view;


    /* Implement ViewBase interface: */
    template<typename>
    friend class ViewBase;

    /* Implement ActionViewBase interface */
    template<typename>
    friend class ActionViewBase;

public:
    /// @brief Create a view on a DefaultAction.
    explicit View(uint8_t* data)
        : ViewBase<View<ActionDispatcher<DefaultActionTag, P, S>>>(data)
        , m_flatbuffers_view(data ? GetSizePrefixedDefaultActionFlat(reinterpret_cast<void*>(data)) : nullptr) { }

    std::string str_impl() const { return "some_action"; }

    BitsetView get_applicability_positive_precondition_bitset_impl() { return BitsetView(m_flatbuffers_view->applicability_positive_precondition_bitset()); }
    BitsetView get_applicability_negative_precondition_bitset_impl() { return BitsetView(m_flatbuffers_view->applicability_negative_precondition_bitset()); }
    BitsetView get_unconditional_positive_effect_bitset_impl() { return BitsetView(m_flatbuffers_view->unconditional_positive_effect_bitset()); };
    BitsetView get_unconditional_negative_effect_bitset_impl() { return BitsetView(m_flatbuffers_view->unconditional_negative_effect_bitset()); };
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ACTIONS_DEFAULT_HPP_

#ifndef MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_

#include "config.hpp"
#include "search_node.hpp"
#include "search_node_builder.hpp"
#include "lifted/state_view.hpp"
#include "grounded/state_view.hpp"
#include "type_traits.hpp"

#include "../buffer/view_base.hpp"
#include "../buffer/byte_stream_utils.hpp"
#include "../formalism/problem/declarations.hpp"



namespace mimir {

/**
 * Interface class
*/
template<typename Derived>
requires HasPlanningModeTag<Derived>
class SearchNodeViewBase {
private:
    using P = typename TypeTraits<Derived>::PlanningMode;

    SearchNodeViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /* Mutable getters. */
    [[nodiscard]] SearchNodeStatus& get_status() { return self().get_status_impl(); }
    [[nodiscard]] g_value_type& get_g_value() { return self().get_g_value_impl(); }
    [[nodiscard]] View<State<P>> get_parent_state() { return self().get_parent_state_impl(); }
    [[nodiscard]] GroundAction get_ground_action() { return self().get_ground_action_impl(); }

    /* Immutable getters. */
    [[nodiscard]] const SearchNodeStatus& get_status() const { return self().get_status_impl(); }
    [[nodiscard]] const g_value_type& get_g_value() const { return self().get_g_value_impl(); }
    [[nodiscard]] const View<State<P>> get_parent_state() const { return self().get_parent_state_impl(); }
    [[nodiscard]] const GroundAction get_ground_action() const { return self().get_ground_action_impl(); }
};


/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
*/
template<IsPlanningModeTag P>
class View<SearchNode<P>> : public ViewBase<View<SearchNode<P>>>, public SearchNodeViewBase<View<SearchNode<P>>> {
private:
    static constexpr size_t s_status_offset =       sizeof(data_size_type);
    static constexpr size_t s_g_value_offset =      sizeof(data_size_type) + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = sizeof(data_size_type) + sizeof(SearchNodeStatus) + sizeof(g_value_type);
    static constexpr size_t s_ground_action =       sizeof(data_size_type) + sizeof(SearchNodeStatus) + sizeof(g_value_type) + sizeof(View<State<P>>);

    /* Implement ViewBase interface: */
    [[nodiscard]] size_t get_offset_to_representative_data_impl() const { return 0; }

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] SearchNodeStatus& get_status_impl() {
        return read_value<SearchNodeStatus>(this->get_data() + s_status_offset);
    }

    [[nodiscard]] g_value_type& get_g_value_impl() {
        return read_value<g_value_type>(this->get_data() + s_g_value_offset);
    }

    [[nodiscard]] View<State<P>> get_parent_state_impl() {
        return read_value<View<State<P>>>(this->get_data() + s_parent_state_offset);
    }

    [[nodiscard]] GroundAction get_ground_action_impl() {
        return read_pointer<const GroundActionImpl>(this->get_data() + s_ground_action);
    }

    friend class SearchNodeViewBase<View<SearchNode<P>>>;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<SearchNode<P>>>(data) { }
};


/**
 * Type traits
*/
template<IsPlanningModeTag P>
struct TypeTraits<View<SearchNode<P>>> {
    using PlanningMode = P;
};


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_

#ifndef MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_

#include "../state_view.hpp"


namespace mimir
{

/**
 * No ID class since we have all tags.
*/


/**
 * Implementation class
 *
 * Reads the memory layout generated by the grounded state builder.
*/
template<>
class View<State<Grounded>> : public ViewBase<View<State<Grounded>>>, public StateViewBase<View<State<Grounded>>> {
private:
    static constexpr size_t s_id_offset = sizeof(data_size_type);
    static constexpr size_t s_data_offset = sizeof(data_size_type) + sizeof(state_id_type);

    /* Implement ViewBase interface */
    [[nodiscard]] size_t get_offset_to_representative_data_impl() const { return s_data_offset; }
    friend class ViewBase<View<State<Grounded>>>;

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] state_id_type get_id_impl() const {
        return read_value<state_id_type>(this->get_data() + s_id_offset);
    }

    friend class StateViewBase<View<State<Grounded>>>;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<State<Grounded>>>(data) { }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_

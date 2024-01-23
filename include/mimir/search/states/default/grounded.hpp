#ifndef MIMIR_SEARCH_STATES_DEFAULT_GROUNDED_HPP_
#define MIMIR_SEARCH_STATES_DEFAULT_GROUNDED_HPP_

#include "../default.hpp"


namespace mimir
{

/**
 * Implementation class
 *
 * The grounded state builder extends the builder base memory layout as follows:
 *  __________________________________
 * |                |          |      |
 * | data_size_type | state_id | TODO |
 * |________________|__________|______|
 *
 *
*/
template<>
class Builder<DefaultStateTag<GroundedTag>> : public BuilderBase<Builder<DefaultStateTag<GroundedTag>>>, public StateBuilderBase<Builder<DefaultStateTag<GroundedTag>>> {
    state_id_type m_id;

    /* Implement BuilderBase interface */
    data_size_type calculate_size_impl() const {
        return sizeof(state_id_type);
    }

    void finish_impl() {
        this->m_buffer.write(m_id);
    }

    friend class BuilderBase<Builder<DefaultStateTag<GroundedTag>>>;

    /* Implement StateBuilderBase interface */
    void set_id_impl(state_id_type id) { m_id = id; }

    friend class StateBuilderBase<Builder<DefaultStateTag<GroundedTag>>>;
};


/**
 * Implementation class
 *
 * Reads the memory layout generated by the grounded state builder.
*/
template<>
class View<DefaultStateTag<GroundedTag>> : public ViewBase<View<DefaultStateTag<GroundedTag>>>, public StateViewBase<View<DefaultStateTag<GroundedTag>>> {
private:
    static constexpr size_t s_id_offset =  sizeof(data_size_type);
    static constexpr size_t s_data_offset = sizeof(data_size_type) + sizeof(state_id_type);

    /* Implement ViewBase interface */
    [[nodiscard]] size_t get_offset_to_representative_data_impl() const { return s_data_offset; }

    friend class ViewBase<View<DefaultStateTag<GroundedTag>>>;

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] state_id_type get_id_impl() const {
        return read_value<state_id_type>(this->get_data() + s_id_offset);
    }

    friend class StateViewBase<View<DefaultStateTag<GroundedTag>>>;

public:
    explicit View(char* data) : ViewBase<View<DefaultStateTag<GroundedTag>>>(data) { }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_STATES_DEFAULT_LIFTED_HPP_

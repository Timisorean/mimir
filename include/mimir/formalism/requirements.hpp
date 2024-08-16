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

#ifndef MIMIR_FORMALISM_REQUIREMENTS_HPP_
#define MIMIR_FORMALISM_REQUIREMENTS_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class RequirementsImpl
{
private:
    size_t m_index;
    loki::RequirementEnumSet m_requirements;

    // Below: add additional members if needed and initialize them in the constructor

    RequirementsImpl(size_t index, loki::RequirementEnumSet requirements);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RequirementsImpl(const RequirementsImpl& other) = delete;
    RequirementsImpl& operator=(const RequirementsImpl& other) = delete;
    RequirementsImpl(RequirementsImpl&& other) = default;
    RequirementsImpl& operator=(RequirementsImpl&& other) = default;

    bool test(loki::RequirementEnum requirement) const;

    std::string str() const;

    size_t get_index() const;
    const loki::RequirementEnumSet& get_requirements() const;
};

extern std::ostream& operator<<(std::ostream& out, const RequirementsImpl& element);

}

#endif

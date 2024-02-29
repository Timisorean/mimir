/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_DOMAIN_PARSER_ACTION_HPP_
#define MIMIR_FORMALISM_DOMAIN_PARSER_ACTION_HPP_

#include "../../common/types.hpp"
#include "../action.hpp"
#include "../declarations.hpp"

namespace mimir
{
extern Action parse(loki::pddl::Action action, PDDLFactories& factories);
extern ActionList parse(loki::pddl::ActionList action_list, PDDLFactories& factories);
}

#endif

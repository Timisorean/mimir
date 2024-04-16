/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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

#include "function_skeleton.hpp"

#include "mimir/formalism/factories.hpp"
#include "parameter.hpp"

namespace mimir
{
FunctionSkeleton parse(loki::FunctionSkeleton function_skeleton, PDDLFactories& factories)
{
    return factories.get_or_create_function_skeleton(function_skeleton->get_name(), parse(function_skeleton->get_parameters(), factories));
}

FunctionSkeletonList parse(loki::FunctionSkeletonList function_skeleton_list, PDDLFactories& factories)
{
    auto result_function_skeleton_list = FunctionSkeletonList();
    for (const auto& function_skeleton : function_skeleton_list)
    {
        result_function_skeleton_list.push_back(parse(function_skeleton, factories));
    }
    return result_function_skeleton_list;
}
}

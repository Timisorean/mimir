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

#include <mimir/formalism/domain/parsers/function_skeleton.hpp>

#include <mimir/formalism/domain/parsers/parameter.hpp>
#include <mimir/formalism/domain/parsers/type.hpp>


namespace mimir 
{
    FunctionSkeleton parse(loki::pddl::FunctionSkeleton function_skeleton, PDDLFactories& factories)
    {
        std::cout << "FunctionSkeleton parse" << std::endl;
        return factories.function_skeletons.get_or_create<FunctionSkeletonImpl>(
            function_skeleton->get_name(), 
            parse(function_skeleton->get_parameters(), factories), 
            parse(function_skeleton->get_type(), factories));
    }

    FunctionSkeletonList parse(loki::pddl::FunctionSkeletonList function_skeleton_list, PDDLFactories& factories) 
    {
        std::cout << "FunctionSkeletons parse" << std::endl;
        auto result_function_skeleton_list = FunctionSkeletonList();
        for (const auto& function_skeleton : function_skeleton_list) {
            result_function_skeleton_list.push_back(parse(function_skeleton, factories));
        }
        return result_function_skeleton_list;
    }
}

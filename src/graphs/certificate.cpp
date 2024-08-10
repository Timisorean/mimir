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

#include "mimir/graphs/certificate.hpp"

#include "mimir/common/hash_utils.hpp"

namespace mimir
{

Certificate::Certificate(size_t num_vertices, size_t num_edges, std::string nauty_certificate, ColorList canonical_initial_coloring) :
    m_num_vertices(num_vertices),
    m_num_edges(num_edges),
    m_nauty_certificate(std::move(nauty_certificate)),
    m_canonical_initial_coloring(std::move(canonical_initial_coloring))
{
}

bool Certificate::operator==(const Certificate& other) const
{
    if (this != &other)
    {
        return (m_num_vertices == other.m_num_vertices) && (m_num_edges == other.m_num_edges)
               && (m_canonical_initial_coloring == other.m_canonical_initial_coloring) && (m_nauty_certificate == other.m_nauty_certificate);
    }
    return true;
}

size_t Certificate::hash() const { return hash_combine(m_num_vertices, m_num_edges, m_nauty_certificate, hash_container(m_canonical_initial_coloring)); }

size_t Certificate::get_num_vertices() const { return m_num_vertices; }

size_t Certificate::get_num_edges() const { return m_num_edges; }

const std::string& Certificate::get_nauty_certificate() const { return m_nauty_certificate; }

const ColorList& Certificate::get_canonical_initial_coloring() const { return m_canonical_initial_coloring; }

}

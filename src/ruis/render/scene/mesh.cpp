/*
carcockpit - Car cockpit example GUI project

Copyright (C) 2024 Gagistech Oy <gagisechoy@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

#include "mesh.hpp"

using namespace ruis::render;

mesh::mesh(const std::vector<utki::shared_ref<primitive>>& primitives, const std::string& name) :
	primitives(primitives),
	name(name)
{}

void mesh::render()
{
	for (const auto& primitive : primitives) {
		primitive.get().render();
	}
}

primitive::primitive(utki::shared_ref<ruis::render::vertex_array> vao, utki::shared_ref<material> material_) :
	vao(vao),
	material_(material_)
{}

void primitive::render()
{
	// material_.get().shader.get()->render(m, vao.get());
}
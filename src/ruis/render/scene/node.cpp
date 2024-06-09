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

#include "node.hpp"

using namespace ruis::render;

node::node(std::shared_ptr<mesh> mesh_, const std::string name, const trs& transformation) :
	mesh_(mesh_),
	name(name),
	transformation(transformation)
{}

const ruis::mat4& node::get_transformation_matrix()
{
	transformation_matrix.set_identity();
	transformation_matrix.translate(transformation.translation);
	transformation_matrix.rotate(transformation.rotation);
	transformation_matrix.scale(transformation.scale);

	return transformation_matrix;
}

// void node::render(const scene& s)
// {
// 	if (mesh_ != nullptr)
// 		mesh_->render();
// 	for (const auto& child : children) {
// 		child.get().render(s);
// 	}
// }
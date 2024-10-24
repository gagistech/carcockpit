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

node::node(
	std::shared_ptr<mesh> mesh_v, //
	std::string name,
	const trs_transformation& transformation
) :
	mesh_v(std::move(mesh_v)),
	name(std::move(name)),
	transformation(transformation)
{}

ruis::mat4 node::get_transformation_matrix() const
{
	ruis::mat4 transformation_matrix;

	transformation_matrix.set_identity();
	transformation_matrix.translate(transformation.translation);
	transformation_matrix.rotate(transformation.rotation);
	transformation_matrix.scale(transformation.scale);

	return transformation_matrix;
}

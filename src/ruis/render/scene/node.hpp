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

#pragma once

#include <variant>

#include <papki/file.hpp>
#include <r4/quaternion.hpp>
#include <r4/vector.hpp>
#include <ruis/config.hpp>

#include "mesh.hpp"
#include "scene.hpp"

namespace ruis::render {

struct trs {
	ruis::vec3 translation;
	ruis::quaternion rotation;
	ruis::vec3 scale;
};

constexpr trs transformation_identity{
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 1.0f}
};

class scene;

class node
{
	ruis::mat4 transformation_matrix;

public:
	std::shared_ptr<mesh> mesh_v; // can be null, empty (intermediate) node
	std::string name;
	trs transformation;

	std::vector<utki::shared_ref<node>> children;

	const ruis::mat4& get_transformation_matrix();
	// std::variant<ruis::mat4, trs> transformation;

	node(std::shared_ptr<mesh> mesh_v, std::string name, const trs& transformation = transformation_identity);

	// void render(const scene& s);
};

} // namespace ruis::render

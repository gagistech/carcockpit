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

namespace ruis::render {

class node
{
public:
	std::string name;

	// TODO: add extras as JSON?

	utki::shared_ref<mesh> mesh_v;

	std::vector<utki::shared_ref<node>> children;

	struct trs {
		ruis::vec3 translation;
		ruis::quaternion rotation;
		ruis::vec3 scale;
	};

	std::variant<ruis::mat4, trs> transformation;
};

} // namespace ruis::render
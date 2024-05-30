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

#include <ruis/context.hpp>
#include <ruis/render/renderer.hpp>

#include "node.hpp"

namespace ruis::render {

class scene
{
public:
	// utki::shared_ref<ruis::context> context_;
	std::vector<utki::shared_ref<node>> nodes;
	// std::vector<utki::shared_ref<mesh>> meshes; // mesh order is important on loading stage
	// scene(utki::shared_ref<ruis::context> c);
	scene();
};

} // namespace ruis::render
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

#include "scene.hpp"

#include <chrono>

using namespace ruis::render;

void scene::update(uint32_t dt)
{
	time += dt;
	[[maybe_unused]] float ft = static_cast<float>(time) / std::milli::den;
	[[maybe_unused]] float fdt = static_cast<float>(dt) / std::milli::den;
}

ruis::mat4 camera::get_projection_matrix(ruis::real aspect_ratio)
{
	ruis::mat4 projection;
	projection.set_identity();
	projection.set_perspective(fovy, aspect_ratio, near, far);
	return projection;
}

ruis::mat4 camera::get_view_matrix()
{
	ruis::mat4 view;
	view.set_identity();
	view.set_look_at(pos, target, up);
	return view;
}

ruis::vec3 camera::to_view_coords(ruis::vec3 vec)
{
	return get_view_matrix() * vec;
}

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

// #include "shaders/shader_adv.hpp"
// #include "shaders/shader_phong.hpp"
// #include "shaders/shader_skybox.hpp"

#include "node.hpp"

namespace ruis::render {

class node;
class camera;
class light;

class scene
{
public:
	std::string name;

	std::vector<utki::shared_ref<node>> nodes;
	std::vector<utki::shared_ref<camera>> cameras;
	std::vector<utki::shared_ref<light>> lights;

	std::shared_ptr<camera> active_camera;
	scene();
	void update(uint32_t dt);
	uint32_t time = 0;
	// void render(ruis::render::renderer& r);
};

class camera
{
public:
	ruis::vec3 pos;
	ruis::vec3 target;
	ruis::vec3 up{0, 1, 0};

	ruis::real fovy;
	ruis::real near{0.1};
	ruis::real far{100};

	ruis::mat4 get_projection_matrix(ruis::real aspect_ratio);
	ruis::mat4 get_view_matrix();

	ruis::vec3 to_view_coords(ruis::vec3 vec);
};

class light
{
public:
	ruis::vec4 pos; // vec4, because w = 0 means light is at infinite distance
	ruis::vec3 intensity;
};

} // namespace ruis::render
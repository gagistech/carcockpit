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

#include <ruis/render/shader.hpp>
#include <ruis/render/texture_2d.hpp>
#include <ruis/render/vertex_array.hpp>

// #include <ruis/res/texture_2d.hpp>

namespace ruis::render {
class material
{
public:
	std::string name;
	std::shared_ptr<ruis::render::texture_2d> tex_diffuse;
	std::shared_ptr<ruis::render::texture_2d> tex_normal;
	std::shared_ptr<ruis::render::texture_2d> tex_arm; // ambient occlusion, roughness, metalness
};

class primitive
{
public:
	utki::shared_ref<ruis::render::vertex_array> vao;
	utki::shared_ref<material> material_;
	primitive(utki::shared_ref<ruis::render::vertex_array> vao, utki::shared_ref<material> material_);
	// void render();
};

class mesh
{
public:
	std::vector<utki::shared_ref<primitive>> primitives;
	std::string name;
	mesh(const std::vector<utki::shared_ref<primitive>>& primitives, const std::string& name);

	void render();
};

} // namespace ruis::render
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
#include <ruis/res/texture_2d.hpp>
#include <ruis/res/texture_cube.hpp>

#include "node.hpp"
#include "scene.hpp"

namespace ruis::render {

class node;
class camera;
class light;

class scene_renderer
{
protected:
	std::shared_ptr<ruis::render::vertex_array> fullscreen_quad_vao;
	std::shared_ptr<ruis::render::scene> scene_v;
	std::shared_ptr<ruis::render::camera> external_camera;
	utki::shared_ref<ruis::context> context_v;
	ruis::mat4 view_matrix;
	ruis::mat4 projection_matrix;
	ruis::render::light main_light;
	ruis::real scene_scaling_factor{1};

	std::shared_ptr<ruis::res::texture_2d> texture_default_white;
	std::shared_ptr<ruis::res::texture_2d> texture_default_black;
	std::shared_ptr<ruis::res::texture_2d> texture_default_normal;
	std::shared_ptr<ruis::res::texture_cube> texture_default_environment_cube;
	std::shared_ptr<ruis::res::texture_cube> texture_environment_cube;
	void render_node(const node& n, const ruis::mat4& parent_tree_model_matrix);
	void render_environment();
	void prepare_fullscreen_quad_vao();

public:
	scene_renderer(utki::shared_ref<ruis::context> c);
	void render(const ruis::vec2& dims, const ruis::mat4& viewport_matrix);
	void set_scene(std::shared_ptr<ruis::render::scene> scene_v);
	void set_scene_scaling_factor(ruis::real scene_scaling_factor);
	void set_environment_cube(std::shared_ptr<ruis::res::texture_cube> texture_environment_cube);
	void set_external_camera(std::shared_ptr<ruis::render::camera> cam);
};

} // namespace ruis::render
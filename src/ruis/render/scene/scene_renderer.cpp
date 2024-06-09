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

#include "scene_renderer.hpp"

#include <chrono>

#include "../../../carcockpit/application.hpp"

using namespace ruis::render;

scene_renderer::scene_renderer(utki::shared_ref<ruis::context> c) :
	context_v(c)
{
	texture_default_black = context_v.get().loader.load<ruis::res::texture_2d>("texture_default_black").to_shared_ptr();
	texture_default_white = context_v.get().loader.load<ruis::res::texture_2d>("texture_default_white").to_shared_ptr();
	texture_default_normal =
		context_v.get().loader.load<ruis::res::texture_2d>("texture_default_normal").to_shared_ptr();
}

void scene_renderer::set_scene(std::shared_ptr<ruis::render::scene> scene_v)
{
	this->scene_v = scene_v;
}

void scene_renderer::render()
{
	if (!scene_v)
		return;

	auto cam = scene_v.get()->active_camera;

	if (!cam)
		return;

	projection_matrix = cam->get_projection_matrix(2.0); // TODO: pass resolution to this render(), and maybe
														 // framebuffer
	view_matrix = cam->get_view_matrix();

	if (scene_v.get()->lights.size() > 0) // scene has at least 1 light
	{
		main_light = scene_v.get()->lights[0].get();
	} else {
		main_light.pos = {2, 4, -2, 1};
		main_light.intensity = {1, 1, 1};
		main_light.intensity *= 1.4;
	}

	ruis::mat4 root_model_matrix;
	root_model_matrix.set_identity();

	for (const auto& node_ : scene_v->nodes) {
		this->render_node(node_, root_model_matrix);
	}
}

void scene_renderer::render_node(utki::shared_ref<node> n, ruis::mat4 parent_model_matrix)
{
	parent_model_matrix *= n.get().get_transformation_matrix();

	ruis::mat4 modelview = view_matrix * parent_model_matrix;
	ruis::mat4 mvp = projection_matrix * modelview;

	ruis::vec4 light_pos_view_coords = view_matrix * main_light.pos; // light position in view (camera) coords
	// render the node itself

	if (n.get().mesh_) {
		for (const auto& primitive : n.get().mesh_->primitives) {
			auto& phong = carcockpit::application::inst().shader_phong_v;

			// TODO: primitive.get().material_ use later somehow, choose shader and textures here, set material-specific
			// uniforms

			phong.render(
				primitive.get().vao.get(),
				mvp,
				modelview,
				texture_default_white->tex(),
				light_pos_view_coords,
				main_light.intensity
			);
		}
	}
	// render children
	for (const auto& node_ : n.get().children) {
		this->render_node(node_, parent_model_matrix);
	}
}

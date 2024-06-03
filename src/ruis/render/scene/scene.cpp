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

#include "../../../carcockpit/application.hpp"

using namespace ruis::render;

scene::scene() {}

void scene::render(ruis::render::renderer& r)
{
	// TODO: render from here
	[[maybe_unused]] auto& phong = carcockpit::application::inst().shader_phong_v;
}

void scene_renderer::render(utki::shared_ref<node> n, ruis::mat4 parent_model)
{
	ruis::mat4 node_model(parent_model);
	node_model *= n.get().get_transformation_matrix();

	this->render_node(n, node_model);

	for (auto c : n.get().children) {
		this->render(c, node_model);
	}
}

scene_renderer_regular::scene_renderer_regular(ruis::render::renderer& r) :
	scene_renderer(r)
{
	// skybox_shader = std::make_shared<shader_skybox>();
	// phong_shader = std::make_shared<shader_phong>();
	// advanced_shader = std::make_shared<shader_adv>();
}

void scene_renderer_regular::render_node(utki::shared_ref<node> n, ruis::mat4 model)
{
	// ruis::mat4 final_matrix = projection * model;
	// phong_shader.get()->
}
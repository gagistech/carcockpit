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

#include "scene_renderer.hxx"

#include <chrono>

#include "../../../carcockpit/application.hpp"

using namespace ruis::render;

scene_renderer::scene_renderer(utki::shared_ref<ruis::context> c) :
	context_v(std::move(c))
{
	texture_default_black = context_v.get().loader.load<ruis::res::texture_2d>("texture_default_black").to_shared_ptr();
	texture_default_white = context_v.get().loader.load<ruis::res::texture_2d>("texture_default_white").to_shared_ptr();
	texture_default_normal =
		context_v.get().loader.load<ruis::res::texture_2d>("texture_default_normal").to_shared_ptr();
	texture_default_environment_cube =
		context_v.get().loader.load<ruis::res::texture_cube>("tex_cube_env_hata").to_shared_ptr();

	prepare_fullscreen_quad_vao();
}

void scene_renderer::set_scene_scaling_factor(ruis::real scene_scaling_factor)
{
	this->scene_scaling_factor = scene_scaling_factor;
}

void scene_renderer::set_scene(std::shared_ptr<ruis::render::scene> scene_v)
{
	this->scene_v = scene_v;
}

void scene_renderer::set_environment_cube(std::shared_ptr<ruis::res::texture_cube> texture_environment_cube)
{
	this->texture_environment_cube = texture_environment_cube;
}

void scene_renderer::set_external_camera(std::shared_ptr<ruis::render::camera> cam)
{
	external_camera = cam;
}

void scene_renderer::render(const ruis::rect& dims, const ruis::mat4& viewport_matrix)
{
	if (!scene_v)
		return;

	auto cam = external_camera ? external_camera : scene_v.get()->active_camera;

	if (!cam)
		return;

	float aspect = dims.d.x() / dims.d.y();
	projection_matrix = viewport_matrix * cam->get_projection_matrix(aspect);

	view_matrix = cam->get_view_matrix();

	constexpr ruis::vec4 default_light_position{2, 4, -2, 1};
	constexpr ruis::vec3 default_light_intensity{2, 2, 2};

	if (scene_v.get()->lights.size() > 0) // scene has at least 1 light
	{
		main_light = scene_v.get()->lights[0].get();
	} else {
		main_light.pos = default_light_position;
		main_light.intensity = default_light_intensity;
	}

	{
		auto& r = this->context_v.get().renderer.get();
		bool depth = r.is_depth_enabled();
		r.enable_depth(false);
		utki::scope_exit scope_exit([&r, depth]() {
			r.enable_depth(depth);
		});
		render_environment();
	}

	ruis::mat4 root_model_matrix;
	root_model_matrix.set_identity();
	root_model_matrix.scale(scene_scaling_factor, scene_scaling_factor, scene_scaling_factor);

	for (const auto& node_v : scene_v->nodes) {
		this->render_node(node_v, root_model_matrix);
	}
}

void scene_renderer::prepare_fullscreen_quad_vao()
{
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	std::array<ruis::vec2, 4> pos = {
		{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}
	};

	auto pos_vbo = this->context_v.get().renderer.get().factory->create_vertex_buffer(utki::make_span(pos));

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	std::array<uint16_t, 36> indices = {
		{0, 2, 1, 1, 2, 3}
	};

	auto indices_vbo = this->context_v.get().renderer.get().factory->create_index_buffer(utki::make_span(indices));

	this->fullscreen_quad_vao =
		context_v.get()
			.renderer.get()
			.factory->create_vertex_array({pos_vbo}, indices_vbo, ruis::render::vertex_array::mode::triangles)
			.to_shared_ptr();
}

void scene_renderer::render_environment()
{
	auto& skybox = carcockpit::application::inst().shader_skybox_v;
	skybox.render(
		*fullscreen_quad_vao.get(),
		view_matrix,
		projection_matrix,
		texture_environment_cube ? texture_environment_cube->tex() : texture_default_environment_cube->tex()
	);
}

void scene_renderer::render_node(utki::shared_ref<node> n, ruis::mat4 parent_model_matrix)
{
	parent_model_matrix *= n.get().get_transformation_matrix();

	[[maybe_unused]] ruis::mat4 modelview_matrix = view_matrix * parent_model_matrix;
	[[maybe_unused]] ruis::mat4 mvp_matrix = projection_matrix * modelview_matrix;

	[[maybe_unused]] ruis::vec4 light_pos_view_coords =
		view_matrix * main_light.pos; // light position in view (camera) coords
	// render the node itself

	if (n.get().mesh_v) {
		for (const auto& primitive : n.get().mesh_v->primitives) {
			[[maybe_unused]] auto& phong = carcockpit::application::inst().shader_phong_v;
			[[maybe_unused]] auto& advanced = carcockpit::application::inst().shader_adv_v;

			// choose shader and textures here, set material-specific uniforms

			auto tex_diffuse = primitive.get().material_v.get().tex_diffuse;
			auto tex_normal = primitive.get().material_v.get().tex_normal;
			auto tex_arm = primitive.get().material_v.get().tex_arm;

			advanced.render(
				primitive.get().vao.get(),
				mvp_matrix,
				modelview_matrix,
				projection_matrix,
				tex_diffuse ? *tex_diffuse.get() : texture_default_white->tex(),
				tex_normal ? *tex_normal.get() : texture_default_normal->tex(),
				tex_arm ? *tex_arm.get() : texture_default_white->tex(),
				texture_environment_cube ? texture_environment_cube->tex() : texture_default_environment_cube->tex(),
				light_pos_view_coords,
				main_light.intensity
			);

			// phong.render(
			// 	primitive.get().vao.get(),
			// 	mvp_matrix,
			// 	modelview_matrix,
			// 	tex_diffuse ? *tex_diffuse.get() : texture_default_white->tex(),
			// 	light_pos_view_coords,
			// 	main_light.intensity
			// );
		}
	}
	// render children
	for (const auto& node_v : n.get().children) {
		this->render_node(node_v, parent_model_matrix);
	}
}

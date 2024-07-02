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

#include "gltf_viewer_widget.hpp"

#include <ratio>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <papki/fs_file.hpp>
#include <ruis/render/opengles/texture_2d.hpp>
#include <ruis/res/texture_cube.hpp>

#include "../ruis/render/scene/gltf_loader.hpp"

#include "application.hpp"

using namespace carcockpit;
using namespace ruis::render;

gltf_viewer_widget::gltf_viewer_widget(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)}),
	ruis::fraction_widget(this->context, {})
{
	// ruis::render::gltf_loader l(*this->context.get().renderer.get().factory, true);
	// auto scene = l.load(papki::fs_file("../res/samples_gltf/parent_and_children.glb"));

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	std::array<ruis::vector3, 36> cube_pos = {
		{ruis::vector3(-0, -0, 0), ruis::vector3(0, -0, 0),   ruis::vector3(-0, 0, 0),   ruis::vector3(0, -0, 0),
		 ruis::vector3(0, 0, 0),   ruis::vector3(-0, 0, 0),   ruis::vector3(0, -0, 0),   ruis::vector3(0, -0, -0),
		 ruis::vector3(0, 0, 0),   ruis::vector3(0, -0, -0),  ruis::vector3(0, 0, -0),   ruis::vector3(0, 0, 0),
		 ruis::vector3(0, -0, -0), ruis::vector3(-0, -0, -0), ruis::vector3(0, 0, -0),   ruis::vector3(-0, -0, -0),
		 ruis::vector3(-0, 0, -0), ruis::vector3(0, 0, -0),   ruis::vector3(-0, -0, -0), ruis::vector3(-0, -0, 0),
		 ruis::vector3(-0, 0, -0), ruis::vector3(-0, -0, 0),  ruis::vector3(-0, 0, 0),   ruis::vector3(-0, 0, -0),
		 ruis::vector3(-0, 0, -0), ruis::vector3(-0, 0, 0),   ruis::vector3(0, 0, -0),   ruis::vector3(-0, 0, 0),
		 ruis::vector3(0, 0, 0),   ruis::vector3(0, 0, -0),   ruis::vector3(-0, -0, -0), ruis::vector3(0, -0, -0),
		 ruis::vector3(-0, -0, 0), ruis::vector3(-0, -0, 0),  ruis::vector3(0, -0, -0),  ruis::vector3(0, -0, 0)}
	};
	auto pos_vbo = this->context.get().renderer.get().factory->create_vertex_buffer(utki::make_span(cube_pos));
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	std::array<ruis::vector2, 36> cube_tex = {
		{ruis::vector2(0, 0), ruis::vector2(1, 0), ruis::vector2(0, 1), ruis::vector2(1, 0), ruis::vector2(1, 1),
		 ruis::vector2(0, 1), ruis::vector2(0, 0), ruis::vector2(1, 0), ruis::vector2(0, 1), ruis::vector2(1, 0),
		 ruis::vector2(1, 1), ruis::vector2(0, 1), ruis::vector2(0, 0), ruis::vector2(1, 0), ruis::vector2(0, 1),
		 ruis::vector2(1, 0), ruis::vector2(1, 1), ruis::vector2(0, 1), ruis::vector2(0, 0), ruis::vector2(1, 0),
		 ruis::vector2(0, 1), ruis::vector2(1, 0), ruis::vector2(1, 1), ruis::vector2(0, 1), ruis::vector2(0, 0),
		 ruis::vector2(1, 0), ruis::vector2(0, 1), ruis::vector2(1, 0), ruis::vector2(1, 1), ruis::vector2(0, 1),
		 ruis::vector2(0, 0), ruis::vector2(1, 0), ruis::vector2(0, 1), ruis::vector2(1, 0), ruis::vector2(1, 1),
		 ruis::vector2(0, 1)}
	};
	auto tex_vbo = this->context.get().renderer.get().factory->create_vertex_buffer(utki::make_span(cube_tex));
	// clang-format off
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	std::array<uint16_t, 36> indices = {{
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
	}};
	// clang-format on
	auto cube_indices = this->context.get().renderer.get().factory->create_index_buffer(utki::make_span(indices));

	this->cube_vao =
		this->context.get()
			.renderer.get()
			.factory->create_vertex_array({pos_vbo, tex_vbo}, cube_indices, ruis::render::vertex_array::mode::triangles)
			.to_shared_ptr();

	this->tex = this->context.get().loader.load<ruis::res::texture_2d>("tex_sample").to_shared_ptr();
	this->rot.set_identity();

	// this->tex_car_diffuse   =
	// this->context.get().loader.load<ruis::res::texture_2d>("tex_car_diffuse").to_shared_ptr();
	// this->tex_car_normal    =
	// this->context.get().loader.load<ruis::res::texture_2d>("tex_car_normal").to_shared_ptr();
	// this->tex_car_roughness =
	// this->context.get().loader.load<ruis::res::texture_2d>("tex_car_roughness").to_shared_ptr();

	this->tex_test = this->context.get().loader.load<ruis::res::texture_2d>("tex_test").to_shared_ptr();

	this->tex_car_diffuse = this->context.get().loader.load<ruis::res::texture_2d>("tex_car_diffuse1").to_shared_ptr();
	this->tex_car_normal = this->context.get().loader.load<ruis::res::texture_2d>("tex_car_normal1").to_shared_ptr();
	this->tex_car_roughness =
		this->context.get().loader.load<ruis::res::texture_2d>("tex_car_roughness1").to_shared_ptr();

	this->tex_rust_diffuse =
		this->context.get().loader.load<ruis::res::texture_2d>("tex_spray_diffuse").to_shared_ptr();
	this->tex_rust_normal = this->context.get().loader.load<ruis::res::texture_2d>("tex_spray_normal").to_shared_ptr();
	this->tex_rust_roughness = this->context.get().loader.load<ruis::res::texture_2d>("tex_spray_arm").to_shared_ptr();

	this->tex_cube_env_hata =
		this->context.get().loader.load<ruis::res::texture_cube>("tex_cube_env_hata").to_shared_ptr();

	std::shared_ptr<ModelOBJ> light_model_obj = std::make_shared<ModelOBJ>();
	std::shared_ptr<ModelOBJ> car_model_obj = std::make_shared<ModelOBJ>();
	std::shared_ptr<ModelOBJ> lamba_left_model_obj = std::make_shared<ModelOBJ>();
	std::shared_ptr<ModelOBJ> lamba_right_model_obj = std::make_shared<ModelOBJ>();
	// this->car_model_obj = std::make_shared<ModelOBJ>();
	light_model_obj->import("../res/car/monkey.obj");
	car_model_obj->import("../res/spray/spray.obj");
	// car_model_obj->import("res/test/bake.obj");
	// car_model_obj->import("res/lamba/lamba.obj");

	lamba_left_model_obj->import("../res/lamba/lamba_l.obj");
	lamba_right_model_obj->import("../res/lamba/lamba_r.obj");
	lamba_left_model_obj->buildVBOs();
	lamba_right_model_obj->buildVBOs();
	light_model_obj->buildVBOs();
	car_model_obj->buildVBOs();

	auto lcar_vbo_positions = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_left_model_obj->getPositionsBuffer())
	);
	auto lcar_vbo_texcoords = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_left_model_obj->getTextureCoordsBuffer())
	);
	auto lcar_vbo_normals = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_left_model_obj->getNormalsBuffer())
	);
	auto lcar_vbo_tangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_left_model_obj->getTangentsBuffer())
	);
	auto lcar_vbo_bitangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_left_model_obj->getBitangentsBuffer())
	);
	auto lcar_vbo_indices = this->context.get().renderer.get().factory->create_index_buffer(
		utki::make_span(lamba_left_model_obj->getShortIndexBuffer())
	);

	auto rcar_vbo_positions = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_right_model_obj->getPositionsBuffer())
	);
	auto rcar_vbo_texcoords = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_right_model_obj->getTextureCoordsBuffer())
	);
	auto rcar_vbo_normals = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_right_model_obj->getNormalsBuffer())
	);
	auto rcar_vbo_tangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_right_model_obj->getTangentsBuffer())
	);
	auto rcar_vbo_bitangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(lamba_right_model_obj->getBitangentsBuffer())
	);
	auto rcar_vbo_indices = this->context.get().renderer.get().factory->create_index_buffer(
		utki::make_span(lamba_right_model_obj->getShortIndexBuffer())
	);

	auto light_vbo_positions = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(light_model_obj->getPositionsBuffer())
	);
	auto light_vbo_texcoords = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(light_model_obj->getTextureCoordsBuffer())
	);
	auto light_vbo_normals = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(light_model_obj->getNormalsBuffer())
	);
	auto light_vbo_tangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(light_model_obj->getTangentsBuffer())
	);
	auto light_vbo_bitangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(light_model_obj->getBitangentsBuffer())
	);
	auto light_vbo_indices = this->context.get().renderer.get().factory->create_index_buffer(
		utki::make_span(light_model_obj->getShortIndexBuffer())
	);

	auto car_vbo_positions = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(car_model_obj->getPositionsBuffer())
	);
	auto car_vbo_texcoords = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(car_model_obj->getTextureCoordsBuffer())
	);
	auto car_vbo_normals = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(car_model_obj->getNormalsBuffer())
	);
	auto car_vbo_tangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(car_model_obj->getTangentsBuffer())
	);
	auto car_vbo_bitangents = this->context.get().renderer.get().factory->create_vertex_buffer(
		utki::make_span(car_model_obj->getBitangentsBuffer())
	);
	auto car_vbo_indices = this->context.get().renderer.get().factory->create_index_buffer(
		utki::make_span(car_model_obj->getShortIndexBuffer())
	);

	this->light_vao =
		this->context.get()
			.renderer.get()
			.factory
			->create_vertex_array(
				{light_vbo_positions, light_vbo_texcoords, light_vbo_normals, light_vbo_tangents, light_vbo_bitangents},
				light_vbo_indices,
				ruis::render::vertex_array::mode::triangles
			)
			.to_shared_ptr();

	this->car_vao =
		this->context.get()
			.renderer.get()
			.factory
			->create_vertex_array(
				{car_vbo_positions, car_vbo_texcoords, car_vbo_normals, car_vbo_tangents, car_vbo_bitangents},
				car_vbo_indices,
				ruis::render::vertex_array::mode::triangles
			)
			.to_shared_ptr();

	this->vao_lamba_l =
		this->context.get()
			.renderer.get()
			.factory
			->create_vertex_array(
				{lcar_vbo_positions, lcar_vbo_texcoords, lcar_vbo_normals, lcar_vbo_tangents, lcar_vbo_bitangents},
				lcar_vbo_indices,
				ruis::render::vertex_array::mode::triangles
			)
			.to_shared_ptr();

	this->vao_lamba_r =
		this->context.get()
			.renderer.get()
			.factory
			->create_vertex_array(
				{rcar_vbo_positions, rcar_vbo_texcoords, rcar_vbo_normals, rcar_vbo_tangents, rcar_vbo_bitangents},
				rcar_vbo_indices,
				ruis::render::vertex_array::mode::triangles
			)
			.to_shared_ptr();

	LOG([&](auto& o) {
		o << "<< SHADER KOM PILE >>" << std::endl;
	})

	this->skybox_s = std::make_shared<shader_skybox>();
	this->phong_s = std::make_shared<shader_phong>();
	this->advanced_s = std::make_shared<shader_adv>();

	int maxTextureSize[1];
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
	LOG([&](auto& o) {
		o << "Max texture size: " << *maxTextureSize << std::endl;
	})

	LOG([&](auto& o) {
		o << "<< LOAD GLTF >>" << std::endl;
	})

	ruis::render::gltf_loader l(*this->context.get().renderer.get().factory);
	// demoscene = l.load(papki::fs_file("../res/samples_gltf/parent_and_children.glb")).to_shared_ptr();
	//demoscene = l.load(papki::fs_file("../res/samples_gltf/camera.glb")).to_shared_ptr();
	demoscene = l.load(papki::fs_file("../res/samples_gltf/spray.glb")).to_shared_ptr();

	sc_renderer = std::make_shared<ruis::render::scene_renderer>(this->context);
	sc_renderer->set_scene(demoscene);
	camrip = std::make_shared<ruis::render::camera>();
	sc_renderer->set_external_camera(camrip);
}

void gltf_viewer_widget::update(uint32_t dt)
{
	demoscene->update(dt);

	this->fps_sec_counter += dt;
	this->time += dt;
	float ft = static_cast<float>(this->time) / std::milli::den;
	float fdt = static_cast<float>(dt) / std::milli::den;
	++this->fps;

	this->rot =
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		(ruis::quaternion()
			 .set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 2.0f * 3.1415926f * float(get_fraction()))) *
		(ruis::quaternion().set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 0.1f * ft));

	// if(camera_transition_ongoing)
	{
		camera_position += (camera_attractor - camera_position) * fdt / camera_transition_duration;
		ruis::vec3 remains = camera_attractor - camera_position;
		ruis::real l2 = remains.x() * remains.x() + remains.y() * remains.y() + remains.z() * remains.z();
		const ruis::real threshold = 0.00001;
		if (l2 < threshold) {
			// camera_transition_ongoing = false;
			camera_position = camera_attractor;
		}
	}

	if (this->fps_sec_counter >= std::milli::den) {
		std::cout << "fps = " << std::dec << fps << std::endl;
		this->fps_sec_counter = 0;
		this->fps = 0;
	}
	this->clear_cache();
}

void gltf_viewer_widget::toggleCamera(bool toggle)
{
	// camera_transition_ongoing = true;
	if (toggle)
		camera_attractor = camera_position_top;
	else
		camera_attractor = camera_position_front;
}

void gltf_viewer_widget::set_normal_mapping(bool toggle)
{
	if (advanced_s)
		advanced_s->set_normal_mapping(toggle);
}

bool gltf_viewer_widget::on_mouse_button(const ruis::mouse_button_event& e)
{
	std::cout << "Is Down = " << e.is_down << std::endl;

	if (e.button == ruis::mouse_button::wheel_up) {
		camera_attractor /= 1.07;
	} else if (e.button == ruis::mouse_button::wheel_down) {
		camera_attractor *= 1.07;
	} else if (e.button == ruis::mouse_button::left) {
		mouse_rotate = e.is_down;
		if (e.is_down) {
			mouse_changeview_start = e.pos;
			camera_changeview_start = camera_position;
		}
	}
	return true;
}

bool gltf_viewer_widget::on_mouse_move(const ruis::mouse_move_event& e)
{
	if (mouse_rotate) {
		ruis::vec2 diff = e.pos - mouse_changeview_start;
		ruis::vec4 diff4 = diff;

		// ruis::mat4 inv_view = get_view_matrix().inv();
		// diff4 = inv_view * diff4;

		ruis::quat q1, q2;
		q1.set_rotation(0, 1, 0, -diff4.x() / 200.0f);
		// camera_attractor.rotate(q);
		q2.set_rotation(1, 0, 0, -diff4.y() / 200.0f);
		// camera_attractor.rotate(q);

		ruis::vec3 cam2go = camera_changeview_start;
		cam2go.rotate(q1);
		cam2go.rotate(q2);

		camera_attractor = cam2go;
	}

	return false;
}

bool gltf_viewer_widget::on_key(const ruis::key_event& e)
{
	return false;
}

ruis::mat4 gltf_viewer_widget::get_view_matrix() const
{
	ruis::mat4 view;
	view.set_identity();
	view.set_look_at(camera_position, camera_target, ruis::vec3(0, 2, 0));
	return view;
}

void gltf_viewer_widget::render(const ruis::matrix4& matrix) const
{
	this->widget::render(matrix);

	// ruis::mat4 mvp(matrix);  // matr = mvp matrix
	// mvp.scale(this->rect().d / 2);
	// mvp.translate(1, 1);
	// mvp.scale(1, 1);
	// mvp.scale(1, -1, -1);
	// mvp.frustum(-1, 1, -1, 1, 1, 10);
	// mvp.translate(0, -1.6, -4);
	// mvp.rotate(this->rot);
	// mvp.scale(2, 2, 2);

	ruis::mat4 viewport;
	viewport.set_identity();
	//	viewport = matrix;
	//	viewport.scale(1.0f / this->rect().d[0], 1.0f / this->rect().d[1], 1.0f);

	ruis::mat4 modelview, model, view, projection, mvp;
	ruis::mat4 model_monkey, modelview_monkey, mvp_monkey;

	projection.set_perspective(3.1415926535f / 4.f, this->rect().d[0] / this->rect().d[1], 0.1f, 20.0f);
	projection *= viewport;

	view = get_view_matrix();

	model.set_identity();
	// model.rotate(this->rot);
	model.scale(0.4f, 0.4f, 0.4f);
	model.translate(0.f, -1.6f, 0.f);
	// model.scale(0.25f, 0.25f, 0.25f);
	// model.scale(10.0f, 10.0f, 10.0f);

	modelview = view * model; //     v * m
	mvp = projection * view * model; // p * v * m

	float fms = static_cast<float>(this->time) / std::milli::den;

	float xx = 3 * cosf(fms / 2);
	float zz = 3 * sinf(fms / 2);

	// xx = 1;
	// zz = -1;

	ruis::vec4 light_pos{xx, zz + 1, 3, 1.0f};
	// ruis::vec3 light_int{1.95f, 1.98f, 2.0f};
	ruis::vec3 light_int{1, 1, 1};
	light_int *= 2.4;

	model_monkey.set_identity();

	// model_monkey.scale(0.5, 0.5, 0.5);
	model_monkey.translate(light_pos[0], light_pos[1], light_pos[2]);
	model_monkey.scale(0.2, 0.2, 0.2);

	modelview_monkey = view * model_monkey;
	mvp_monkey = projection * view * model_monkey;

	[[maybe_unused]] ruis::vec4 light_pos_view = view * light_pos; // light position in view (camera) coords

	// glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_DEPTH_BUFFER_BIT);

	// advanced_s->render(*this->car_vao, mvp, modelview, projection,
	// this->tex_rust_diffuse->tex(),
	//  					this->tex_rust_normal->tex(),
	//  this->tex_rust_roughness->tex(), light_pos_view, light_int);

	// GLenum err;
	// while((err = glGetError()) != GL_NO_ERROR) {} // skip all uniform-related
	// errors (TODO: remove asap)

	// //phong_s->render(*this->vao_lamba_l, mvp, modelview,
	// this->tex_car_diffuse->tex(), light_pos_view, light_int);
	// phong_s->render(*this->vao_lamba_r, mvp, modelview, this->tex_car_diffuse->tex(), light_pos_view, light_int);
	// skybox_s->render(*this->car_vao,
	// 	mvp,
	// 	modelview,
	// 	this->tex_cube_env_hata->tex());

	// ruis::mat4 mtrx;

	// auto vaao1 = demoscene->nodes[0].get().mesh_.get()->primitives[0].get().vao.to_shared_ptr();
	// mtrx = demoscene->nodes[0].get().get_transformation_matrix();
	// application::inst()
	// 	.shader_phong_v.render(*vaao1, mvp * mtrx, modelview * mtrx, this->tex_test->tex(), light_pos_view, light_int);

	// auto vaao2 = demoscene->nodes[0].get().children[0].get().mesh_.get()->primitives[0].get().vao.to_shared_ptr();
	// mtrx *= demoscene->nodes[0].get().children[0].get().get_transformation_matrix();
	// application::inst()
	// 	.shader_phong_v.render(*vaao2, mvp * mtrx, modelview * mtrx, this->tex_test->tex(), light_pos_view, light_int);

	// phong_s->render(*this->light_vao, mvp_monkey, modelview_monkey, this->tex_test->tex(), light_pos_view,
	// light_int);

	camrip->pos = camera_position;
	camrip->target = camera_target;
	camrip->up = ruis::vec3(0, 2, 0);
	camrip->fovy = 3.1415926535f / 4.f;
	camrip->near = 0.1;
	camrip->far = 20.f;

	sc_renderer->render();

	// advanced_s->render(
	// 	*this->car_vao,
	// 	mvp,
	// 	modelview,
	// 	projection,
	// 	this->tex_rust_diffuse->tex(),
	// 	this->tex_rust_normal->tex(),
	// 	this->tex_rust_roughness->tex(),
	// 	this->tex_cube_env_hata->tex(),
	// 	light_pos_view,
	// 	light_int
	// );

	// //advanced_s->render(*this->vao_lamba_r, mvp, modelview, projection,
	// this->tex_car_diffuse->tex(),
	// // 					this->tex_car_normal->tex(),
	// this->tex_car_roughness->tex(), light_pos_view, light_int);

	glDisable(GL_DEPTH_TEST);
}

/// How to register mouse events outside app window ?
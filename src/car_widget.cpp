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

#include "car_widget.hpp"

#include <ratio>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <ruis/render/opengles/texture_2d.hpp>

using namespace carcockpit;

car_widget::car_widget(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)}),
	ruis::fraction_widget(this->context, {})
{
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

	this->tex = this->context.get().loader.load<ruis::res::texture>("tex_sample").to_shared_ptr();
	this->rot.set_identity();

	// this->tex_car_diffuse   =
	// this->context.get().loader.load<ruis::res::texture>("tex_car_diffuse").to_shared_ptr();
	// this->tex_car_normal    =
	// this->context.get().loader.load<ruis::res::texture>("tex_car_normal").to_shared_ptr();
	// this->tex_car_roughness =
	// this->context.get().loader.load<ruis::res::texture>("tex_car_roughness").to_shared_ptr();

	this->tex_test = this->context.get().loader.load<ruis::res::texture>("tex_test").to_shared_ptr();

	this->tex_car_diffuse = this->context.get().loader.load<ruis::res::texture>("tex_car_diffuse1").to_shared_ptr();
	this->tex_car_normal = this->context.get().loader.load<ruis::res::texture>("tex_car_normal1").to_shared_ptr();
	this->tex_car_roughness = this->context.get().loader.load<ruis::res::texture>("tex_car_roughness1").to_shared_ptr();

	this->tex_rust_diffuse = this->context.get().loader.load<ruis::res::texture>("tex_car_diffuse").to_shared_ptr();
	this->tex_rust_normal = this->context.get().loader.load<ruis::res::texture>("tex_car_normal").to_shared_ptr();
	this->tex_rust_roughness = this->context.get().loader.load<ruis::res::texture>("tex_car_roughness").to_shared_ptr();

	std::shared_ptr<ModelOBJ> light_model_obj = std::make_shared<ModelOBJ>();
	std::shared_ptr<ModelOBJ> car_model_obj = std::make_shared<ModelOBJ>();
	std::shared_ptr<ModelOBJ> lamba_left_model_obj = std::make_shared<ModelOBJ>();
	std::shared_ptr<ModelOBJ> lamba_right_model_obj = std::make_shared<ModelOBJ>();
	// this->car_model_obj = std::make_shared<ModelOBJ>();
	light_model_obj->import("res/car/monkey.obj");
	car_model_obj->import("res/car/car3d.obj");
	// car_model_obj->import("res/test/bake.obj");
	// car_model_obj->import("res/lamba/lamba.obj");

	lamba_left_model_obj->import("res/lamba/lamba_l.obj");
	lamba_right_model_obj->import("res/lamba/lamba_r.obj");
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

	this->phong_s = std::make_shared<shader_phong>();
	this->advanced_s = std::make_shared<shader_adv>();

	int maxTextureSize[1];
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
	LOG([&](auto& o) {
		o << "Max texture size: " << *maxTextureSize << std::endl;
	})
}

void car_widget::update(uint32_t dt)
{
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

void car_widget::toggleCamera(bool toggle)
{
	// camera_transition_ongoing = true;
	if (toggle)
		camera_attractor = camera_position_top;
	else
		camera_attractor = camera_position_front;
}

void car_widget::setNormalMapping(bool toggle)
{
	if (advanced_s)
		advanced_s->setNormalMapping(toggle);
}

void car_widget::render(const ruis::matrix4& matrix) const
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

	ruis::mat4 modelview, model, view, projection, mvp;
	ruis::mat4 model_monkey, modelview_monkey, mvp_monkey;
	ruis::vec3 pos = this->camera_position; //{3, 1.5, 3};
	projection.set_perspective(3.1415926535f / 3.f, this->rect().d[0] / this->rect().d[1], 0.1f, 20.0f);
	view.set_identity();
	view.set_look_at(pos, ruis::vec3(0, 1, 0), ruis::vec3(0, 2, 0));
	model.set_identity();
	model.rotate(this->rot);
	model.scale(0.25f, 0.25f, 0.25f);

	modelview = view * model; //     v * m
	mvp = projection * view * model; // p * v * m

	float fms = static_cast<float>(this->time) / std::milli::den;

	float xx = 3 * cosf(fms / 4);
	float zz = 3 * sinf(fms / 4);

	// xx = 1;
	// zz = -1;

	ruis::vec4 light_pos{xx, 3.5, zz, 1.0f};
	// ruis::vec3 light_int{1.95f, 1.98f, 2.0f};
	ruis::vec3 light_int{1.6, 1.6, 1.6};

	model_monkey.set_identity();

	// model_monkey.scale(0.5, 0.5, 0.5);
	model_monkey.translate(light_pos[0], light_pos[1], light_pos[2]);
	model_monkey.scale(0.2, 0.2, 0.2);

	modelview_monkey = view * model_monkey;
	mvp_monkey = projection * view * model_monkey;

	ruis::vec4 light_pos_view = view * light_pos; // light position in view (camera) coords

	// glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// advanced_s->render(*this->car_vao, mvp, modelview, projection,
	// this->tex_rust_diffuse->tex(),
	//  					this->tex_rust_normal->tex(),
	//  this->tex_rust_roughness->tex(), light_pos_view, light_int);

	// GLenum err;
	// while((err = glGetError()) != GL_NO_ERROR) {} // skip all uniform-related
	// errors (TODO: remove asap)

	// //phong_s->render(*this->vao_lamba_l, mvp, modelview,
	// this->tex_car_diffuse->tex(), light_pos_view, light_int);
	phong_s->render(*this->vao_lamba_r, mvp, modelview, this->tex_car_diffuse->tex(), light_pos_view, light_int);
	phong_s->render(*this->light_vao, mvp_monkey, modelview_monkey, this->tex_test->tex(), light_pos_view, light_int);

	advanced_s->render(
		*this->vao_lamba_l,
		mvp,
		modelview,
		projection,
		this->tex_car_diffuse->tex(),
		this->tex_car_normal->tex(),
		this->tex_car_roughness->tex(),
		light_pos_view,
		light_int
	);
	// //advanced_s->render(*this->vao_lamba_r, mvp, modelview, projection,
	// this->tex_car_diffuse->tex(),
	// // 					this->tex_car_normal->tex(),
	// this->tex_car_roughness->tex(), light_pos_view, light_int);

	glDisable(GL_DEPTH_TEST);
}

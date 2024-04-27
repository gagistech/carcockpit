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

// something
ruis::mat3 from_mat4(const ruis::mat4& mat)
{
	ruis::mat3 m;
	for(int i = 0; i < 3; ++i)
		for(int j = 0; j < 3; ++j)
			m[i][j] = mat[i][j];
	return m;
}

car_widget::car_widget(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)}),
	ruis::fraction_widget(this->context, {})
{
	// // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	// std::array<ruis::vector3, 36> cube_pos = {
	// 	{ruis::vector3(-0, -0, 0), ruis::vector3(0, -0, 0),   ruis::vector3(-0, 0, 0),   ruis::vector3(0, -0, 0),
	// 	 ruis::vector3(0, 0, 0),   ruis::vector3(-0, 0, 0),   ruis::vector3(0, -0, 0),   ruis::vector3(0, -0, -0),
	// 	 ruis::vector3(0, 0, 0),   ruis::vector3(0, -0, -0),  ruis::vector3(0, 0, -0),   ruis::vector3(0, 0, 0),
	// 	 ruis::vector3(0, -0, -0), ruis::vector3(-0, -0, -0), ruis::vector3(0, 0, -0),   ruis::vector3(-0, -0, -0),
	// 	 ruis::vector3(-0, 0, -0), ruis::vector3(0, 0, -0),   ruis::vector3(-0, -0, -0), ruis::vector3(-0, -0, 0),
	// 	 ruis::vector3(-0, 0, -0), ruis::vector3(-0, -0, 0),  ruis::vector3(-0, 0, 0),   ruis::vector3(-0, 0, -0),
	// 	 ruis::vector3(-0, 0, -0), ruis::vector3(-0, 0, 0),   ruis::vector3(0, 0, -0),   ruis::vector3(-0, 0, 0),
	// 	 ruis::vector3(0, 0, 0),   ruis::vector3(0, 0, -0),   ruis::vector3(-0, -0, -0), ruis::vector3(0, -0, -0),
	// 	 ruis::vector3(-0, -0, 0), ruis::vector3(-0, -0, 0),  ruis::vector3(0, -0, -0),  ruis::vector3(0, -0, 0)}
	// };
	
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	std::array<ruis::vector3, 36> cube_pos = {
		{ruis::vector3(-1, -1, 1), ruis::vector3(1, -1, 1),   ruis::vector3(-1, 1, 1),   ruis::vector3(1, -1, 1),
		 ruis::vector3(1, 1, 1),   ruis::vector3(-1, 1, 1),   ruis::vector3(1, -1, 1),   ruis::vector3(1, -1, -1),
		 ruis::vector3(1, 1, 1),   ruis::vector3(1, -1, -1),  ruis::vector3(1, 1, -1),   ruis::vector3(1, 1, 1),
		 ruis::vector3(1, -1, -1), ruis::vector3(-1, -1, -1), ruis::vector3(1, 1, -1),   ruis::vector3(-1, -1, -1),
		 ruis::vector3(-1, 1, -1), ruis::vector3(1, 1, -1),   ruis::vector3(-1, -1, -1), ruis::vector3(-1, -1, 1),
		 ruis::vector3(-1, 1, -1), ruis::vector3(-1, -1, 1),  ruis::vector3(-1, 1, 1),   ruis::vector3(-1, 1, -1),
		 ruis::vector3(-1, 1, -1), ruis::vector3(-1, 1, 1),   ruis::vector3(1, 1, -1),   ruis::vector3(-1, 1, 1),
		 ruis::vector3(1, 1, 1),   ruis::vector3(1, 1, -1),   ruis::vector3(-1, -1, -1), ruis::vector3(1, -1, -1),
		 ruis::vector3(-1, -1, 1), ruis::vector3(-1, -1, 1),  ruis::vector3(1, -1, -1),  ruis::vector3(1, -1, 1)}
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
			.factory->create_vertex_array({pos_vbo, tex_vbo}, cube_indices, ruis::vertex_array::mode::triangles)
			.to_shared_ptr();

	this->tex = this->context.get().loader.load<ruis::res::texture>("tex_sample").to_shared_ptr();
	this->rot.set_identity();


	this->tex_car_diffuse   = this->context.get().loader.load<ruis::res::texture>("tex_car_diffuse").to_shared_ptr();
	this->tex_car_normal    = this->context.get().loader.load<ruis::res::texture>("tex_car_normal").to_shared_ptr();
	this->tex_car_roughness = this->context.get().loader.load<ruis::res::texture>("tex_car_roughness").to_shared_ptr();

	std::cout << "car_W" << std::endl;

	this->car_model_obj = std::make_shared<ModelOBJ>();

	//car_model_obj->import("res/car/monkey.obj");
	car_model_obj->import("res/car/car3d.obj");
	car_model_obj->buildVBOs();

	auto car_vbo_positions = this->context.get().renderer.get().factory->create_vertex_buffer(utki::make_span(car_model_obj->getPositionsBuffer()));
	auto car_vbo_texcoords = this->context.get().renderer.get().factory->create_vertex_buffer(utki::make_span(car_model_obj->getTextureCoordsBuffer()));
	auto car_vbo_normals   = this->context.get().renderer.get().factory->create_vertex_buffer(utki::make_span(car_model_obj->getNormalsBuffer()));
	auto car_vbo_indices   = this->context.get().renderer.get().factory->create_index_buffer(utki::make_span(car_model_obj->getShortIndexBuffer()));

	this->car_vao =
		this->context.get()
			.renderer.get()
			.factory->create_vertex_array({car_vbo_positions, car_vbo_texcoords, car_vbo_normals}, car_vbo_indices, ruis::vertex_array::mode::triangles)
			.to_shared_ptr();

	LOG([&](auto& o) { o << "<< SHADER KOM PILE >>\n" << std::endl; })
	this->shader = std::make_shared<shader_car>();
}

void car_widget::update(uint32_t dt)
{
	this->fps_sec_counter += dt;
	this->time_sec += dt;
	++this->fps;
	this->rot =
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		(ruis::quaternion().set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 2.0f * 3.1415926f * float(get_fraction()))) *
		//ruis::quaternion().set_identity() *
		(ruis::quaternion().set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 0.1f * (float(this->time_sec) / std::milli::den)));
		//(ruis::quaternion().set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 0.1f * (float(this->time_sec) / std::milli::den)));
	if (this->fps_sec_counter >= std::milli::den) {
		std::cout << "fps = " << std::dec << fps << std::endl;
		this->fps_sec_counter = 0;
		this->fps = 0;
	}
	this->clear_cache();
}

ruis::mat4 look_at(const ruis::vec3& eye, const ruis::vec3& center, const ruis::vec3& up)
{
    ruis::vec3 f = (center - eye).normalize();
	ruis::vec3 s = f.cross(up).normalize();
    ruis::vec3 u = s.cross(f);

    ruis::mat4 lookat;
	lookat.set_identity();
	lookat.transpose();
	//lookat.set(1.f);
    lookat[0][0] = s[0];
    lookat[1][0] = s[1];
    lookat[2][0] = s[2];
    lookat[0][1] = u[0];
    lookat[1][1] = u[1];
    lookat[2][1] = u[2];
    lookat[0][2] = -f[0];
    lookat[1][2] = -f[1];
    lookat[2][2] = -f[2];
    lookat[3][0] = -s * eye;   // * = dot product
    lookat[3][1] = -u * eye;
    lookat[3][2] =  f * eye;

    return lookat.tposed();
}

//// look_at in case of other handedness
// {
//   vec<3, T, Q> const f(normalize(center - eye));
//   vec<3, T, Q> const s(normalize(cross(up, f)));
//   vec<3, T, Q> const u(cross(f, s));

//   mat<4, 4, T, Q> Result(1);
//   Result[0][0] = s.x;
//   Result[1][0] = s.y;
//   Result[2][0] = s.z;
//   Result[0][1] = u.x;
//   Result[1][1] = u.y;
//   Result[2][1] = u.z;
//   Result[0][2] = f.x;
//   Result[1][2] = f.y;
//   Result[2][2] = f.z;
//   Result[3][0] = -dot(s, eye);
//   Result[3][1] = -dot(u, eye);
//   Result[3][2] = -dot(f, eye);
//   return Result;
// }

ruis::mat4 perspective(float fovy, float aspect, float zNear, float zFar)
{
 //   assert(aspect != 0);
 //   assert(zFar != zNear);
    float tan_half_fov_y = tan(fovy / 2.0f);
    ruis::mat4 persp;
	persp.set(0.f);
    persp[0][0] = 1.0f / (aspect * tan_half_fov_y);
    persp[1][1] = 1.0f / (tan_half_fov_y);
    persp[2][2] = -(zFar + zNear) / (zFar - zNear);
    persp[2][3] = -1.0f;
    persp[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);

    return persp.tposed();
}

void car_widget::render(const ruis::matrix4& matrix) const
{
	this->widget::render(matrix);

	// ruis::mat4 mvp(matrix);  // matr = mvp matrix
	// mvp.scale(this->rect().d / 2);
	// mvp.translate(1, 1);
	// mvp.scale(1, 1);
	// mvp.scale(1, -1, -1);
	//mvp.frustum(-1, 1, -1, 1, 1, 10);
	//mvp.translate(0, -1.6, -4);
	//mvp.rotate(this->rot);
	//mvp.scale(2, 2, 2);

	// old ^^
	// new below

	//std::cout << "sin(30) = " << sin(30 * 3.1415926535 / 180) << std::endl;
	ruis::mat4 modelview, model, view, projection, mvp;
	ruis::vec3 pos{3, 1, 3};
	projection = perspective(3.1415926535f / 2.8f, this->rect().d[0] / this->rect().d[1], 0.1f, 10.0f);  
	view.set_identity();
	view = look_at(pos, ruis::vec3(0, 1, 0), ruis::vec3(0, 2, 0));
	model.set_identity();
	//model.translate(0, -0.7, -3);
	model.rotate(this->rot);
	
	modelview = view * model;
	mvp = projection * view * model;   // p * v * m
	//mvp = projection;// * mvp;

	// The normal matrix is typically the inverse transpose of the
	// upper-left 3 x 3 portion of the model-view matrix. We use the
	// inverse transpose because normal vectors transform differently
	// than the vertex position.
	ruis::mat3 normal = from_mat4(modelview); 
	normal.invert();
	normal.transpose();

	ruis::vec4 light_pos{3.0f, 4.0f, 5.0f, 1.0f};
	ruis::vec3 light_int{0.5f, 0.7f, 0.9f};

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	//this->context.get().renderer.get().shader->pos_tex->render(matr, *this->cube_vao, this->tex->tex());
    //static_cast<const ruis::render_opengles::texture_2d&>(tex_car_diffuse->tex()).bind(0); // rewritten by official draw call
	
	// void set_uniform_sampler(GLint id, GLint texture_unit_num) const;
	// void set_uniform_matrix4f(GLint id, const r4::matrix4<float>& m) const;
	// void set_uniform4f(GLint id, float x, float y, float z, float a) const;

// mat4_modelview, mat4_projection, mat3_normal, vec4_light_position, vec3_light_intensity;

	//(static_cast<ruis::render_opengles::shader_base>(shader))
	shader->set_uniform4f(shader->vec4_light_position, light_pos[0], light_pos[1], light_pos[2], light_pos[3]);
	shader->set_uniform3f(shader->vec3_light_intensity, light_int[0], light_int[1], light_int[2]);
	shader->set_uniform_matrix4f(shader->mat4_modelview, modelview);
	shader->set_uniform_matrix4f(shader->mat4_projection, projection);
	shader->set_uniform_matrix3f(shader->mat3_normal, normal);

	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {} // skip all uniform-related errors

	shader->render(mvp, *this->car_vao, this->tex_car_diffuse->tex());

	//glActiveTexture(GL_TEXTURE0 + 0);
	//glBindTexture(GL_TEXTURE_2D, static_cast<ruis::render_opengles::texture_2d&>(tex_car_diffuse->tex()));

	//car_model_obj->render();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

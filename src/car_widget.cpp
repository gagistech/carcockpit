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

using namespace carcockpit;

car_widget::car_widget(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)})
{
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
			.factory->create_vertex_array({pos_vbo, tex_vbo}, cube_indices, ruis::render::vertex_array::mode::triangles)
			.to_shared_ptr();

	this->tex = this->context.get().loader.load<ruis::res::texture>("tex_sample").to_shared_ptr();
	this->rot.set_identity();
}

void car_widget::update(uint32_t dt)
{
	this->fps_sec_counter += dt;
	++this->fps;
	this->rot *=
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		ruis::quaternion().set_rotation(r4::vector3<float>(1, 2, 1).normalize(), 1.5f * (float(dt) / std::milli::den));
	if (this->fps_sec_counter >= std::milli::den) {
		std::cout << "fps = " << std::dec << fps << std::endl;
		this->fps_sec_counter = 0;
		this->fps = 0;
	}
	this->clear_cache();
}

void car_widget::render(const ruis::matrix4& matrix) const
{
	this->widget::render(matrix);

	ruis::matrix4 matr(matrix);
	matr.scale(this->rect().d / 2);
	matr.translate(1, 1);
	matr.scale(1, -1, -1);
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	matr.frustum(-2, 2, -1.5, 1.5, 2, 100);
	matr.translate(0, 0, -4);
	matr.rotate(this->rot);

	glDisable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	this->context.get().renderer.get().shader->pos_tex->render(matr, *this->cube_vao, this->tex->tex());

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
}

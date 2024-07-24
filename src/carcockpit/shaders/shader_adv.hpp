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

#include <ruis/config.hpp>
#include <ruis/render/opengles/shader_base.hpp>
#include <ruis/render/texture_2d.hpp>
#include <ruis/render/texture_cube.hpp>

namespace ruis::render {

class shader_adv : public ruis::render::opengles::shader_base
{
public:
	GLint sampler_normal_map;
	GLint sampler_roughness_map;
	GLint sampler_cube;

	GLint mat4_modelview;
	GLint mat3_normal;

	GLint vec4_light_position;
	GLint vec3_light_intensity;
	GLint vec3_set_normal_mapping;

	ruis::vec3 set_normal_mapping_vector{1, 1, 1};

	shader_adv();
	void render(
		const ruis::render::vertex_array& va,
		const r4::matrix4<float>& mvp,
		const r4::matrix4<float>& modelview,
		const r4::matrix4<float>& projection,
		const ruis::render::texture_2d& tex_color,
		const ruis::render::texture_2d& tex_normal,
		const ruis::render::texture_2d& tex_roughness,
		const ruis::render::texture_cube& tex_cube_env,
		const ruis::vec4& light_pos,
		const ruis::vec3& light_int
	) const;

	void set_normal_mapping(bool on);
};

} // namespace ruis::render
/*
ruis-render-opengles - OpenGL ES GUI renderer

Copyright (C) 2012-2024  Ivan Gagis <igagis@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

#pragma once

//#include <ruis/render/texturing_shader.hpp>
#include <ruis/render/texture_2d.hpp>
#include <ruis/render/opengles/shader_base.hpp>

namespace ruis 
{
	using mat3 = r4::matrix3<real>;
	using matrix3 = mat3;
	static_assert(sizeof(mat3) == sizeof(real) * 3 * 3, "size mismatch");
}

namespace carcockpit {


class shader_phong : public ruis::render::opengles::shader_base
{
public:
	GLint mat4_modelview, mat3_normal, vec4_light_position, vec3_light_intensity;

	shader_phong();
	void render(const ruis::render::vertex_array& va,
	            const r4::matrix4<float>& mvp,
				const r4::matrix4<float>& modelview,   
				const ruis::render::texture_2d& tex,
				const ruis::vec4& light_pos,
				const ruis::vec3& light_int) const;

	virtual void set_uniform_matrix3f(GLint id, const r4::matrix3<float>& m) const;
	virtual void set_uniform_matrix4f(GLint id, const r4::matrix4<float>& m) const;
	virtual void set_uniform3f(GLint id, float x, float y, float z) const;
	virtual void set_uniform4f(GLint id, float x, float y, float z, float w) const;
	virtual GLint get_uniform(const char* name);
};

} // namespace ruis::render::opengles
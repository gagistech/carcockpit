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

#include "shader_skybox.hpp"

#include <ruis/render/opengles/index_buffer.hpp>
#include <ruis/render/opengles/texture_cube.hpp>
#include <ruis/render/opengles/util.hpp>
#include <ruis/render/opengles/vertex_array.hpp>
#include <ruis/render/opengles/vertex_buffer.hpp>

using namespace ruis::render;

static r4::matrix3<float> from_mat4(const r4::matrix4<float>& mat)
{
	r4::matrix3<float> m;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			m[i][j] = mat[i][j];
	return m;
}

shader_skybox::shader_skybox() :
	shader_base(
		R"qwertyuiop(
						attribute highp vec4 a0; // position

						uniform highp mat4 matrix;       // mvp matrix
						uniform highp mat4 mat3_imv;     // inverse modelview matrix 
						uniform highp mat4 mat4_ip;      // inverse projection matrix 

						varying highp vec3 eyeDirection;

						void main(void)
						{	
							vec3 unprojected = (inverseProjection * aPosition).xyz;
							eyeDirection = inverseModelview * unprojected;

							gl_Position = aPosition;
						}
	)qwertyuiop",
		R"qwertyuiop(			
						precision highp float;
		
						varying highp vec3 eyeDirection;

						uniform samplerCube texture0;
			
						void main() 
						{					
							gl_FragColor = vec4( textureCube(texture0, eyeDirection) );
						}
	)qwertyuiop"
	)
	,
	mat3_inverse_modelview(this->get_uniform("mat4_imv")),
	mat4_inverse_projection(this->get_uniform("mat4_ip"))
{}

void shader_skybox::render(
	const ruis::render::vertex_array& va,
	const r4::matrix4<float>& modelview,
	const r4::matrix4<float>& projection,
	const ruis::render::texture_cube& tex_env_cube
) const
{
	ASSERT(dynamic_cast<const ruis::render::opengles::texture_cube*>(&tex_env_cube))

	r4::matrix4<float> inverse_projection = projection.inv();
	r4::matrix3<float> inverse_modelview = from_mat4(modelview).tposed();

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render::opengles::texture_cube&>(tex_env_cube).bind(0);

	this->bind(); // bind the program

	this->set_uniform_matrix3f(this->mat3_inverse_modelview, inverse_modelview);
	this->set_uniform_matrix4f(this->mat4_inverse_projection, inverse_projection);

	this->shader_base::render(r4::matrix4<float>(), va);
}
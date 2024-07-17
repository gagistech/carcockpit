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

shader_skybox::shader_skybox() :
	shader_base(
		R"qwertyuiop(
						attribute highp vec4 a0; // position

						uniform highp mat4 matrix;       // inverse projection matrix 
						uniform highp mat3 mat3_imv;     // inverse modelview matrix 

						varying highp vec3 eyeDirection;

						void main(void)
						{	
							vec3 unprojected = (matrix * a0).xyz;
							eyeDirection = mat3_imv * unprojected;
							eyeDirection.y = -eyeDirection.y;

							gl_Position = a0;
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
	),
	mat3_inverse_modelview(this->get_uniform("mat3_imv"))
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
	r4::matrix3<float> inverse_modelview = modelview.submatrix<0, 0, 3, 3>().tposed();

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render::opengles::texture_cube&>(tex_env_cube).bind(0);

	this->bind(); // bind the program

	this->set_uniform_matrix3f(this->mat3_inverse_modelview, inverse_modelview);

	this->shader_base::render(inverse_projection, va);
}
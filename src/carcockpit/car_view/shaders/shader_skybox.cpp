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

using namespace carcockpit;

shader_skybox::shader_skybox() :
	shader_base(
		R"qwertyuiop(
						attribute highp vec4 a0; // position

						uniform highp mat4 matrix;       // mvp matrix
						uniform highp mat4 mat4_mv;      // modelview matrix 

						varying highp vec3 pos;

						void main(void)
						{
							pos = vec3( mat4_mv * a0 );        
							gl_Position = matrix * a0;
						}
	)qwertyuiop",
		R"qwertyuiop(			
						precision highp float;
		
						varying highp vec3 pos;

						uniform samplerCube texture0;
			
						void main() 
						{					
							gl_FragColor = vec4( textureCube(texture0, pos) );
						}
	)qwertyuiop"
	),
	mat4_modelview(this->get_uniform("mat4_mv"))
{}

void shader_skybox::render(
	const ruis::render::vertex_array& va,
	const r4::matrix4<float>& mvp,
	const r4::matrix4<float>& modelview,
	const ruis::render::texture_cube& tex_env_cube
) const
{
	ASSERT(dynamic_cast<const ruis::render::opengles::texture_cube*>(&tex_env_cube))
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render::opengles::texture_cube&>(tex_env_cube).bind(0);

	this->bind(); // bind the program

	this->set_uniform_matrix4f(this->mat4_modelview, modelview);

	this->shader_base::render(mvp, va);
}
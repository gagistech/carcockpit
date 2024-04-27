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

#include "shader_car.hpp"

#include <ruis/render/opengles/index_buffer.hpp>
#include <ruis/render/opengles/texture_2d.hpp>
#include <ruis/render/opengles/util.hpp>
#include <ruis/render/opengles/vertex_array.hpp>
#include <ruis/render/opengles/vertex_buffer.hpp>

using namespace carcockpit;
/*
shader_car::shader_car() :
	shader_base(
	R"qwertyuiop(
						attribute highp vec4 a0; // position
						attribute highp vec2 a1; // texture coordinate
                     attribute highp vec3 a2; // normal

						uniform highp mat4 matrix;

						varying highp vec2 tc0;
                        varying highp vec3 norm;

						void main(void)
						{
							gl_Position = matrix * a0;
							tc0 = a1;
                            norm = a2;
						}
	)qwertyuiop",
	R"qwertyuiop(
						uniform sampler2D texture0;	
						varying highp vec2 tc0;
                        varying highp vec3 norm;
						precision highp float;	
						void main(void)
						{
							float f = dot(norm, vec3(1.0, 0, 0));
							//gl_FragColor = vec4(f, f, f, 1);
							gl_FragColor = (0.4 + f) * texture2D(texture0, tc0);
						}
	)qwertyuiop"
	)
{
}
*/
shader_car::shader_car() :
	shader_base(
	R"qwertyuiop(
						attribute highp vec4 a0; // position
						attribute highp vec2 a1; // texture coordinate
                        attribute highp vec3 a2; // normal

						uniform highp mat4 matrix;       // mvp matrix
						uniform highp mat4 mat4_mv;      // modelview matrix  
						//uniform highp mat4 mat4_p;       // projection matrix  
						uniform highp mat3 mat3_n;       // normal matrix (mat3)

						uniform vec4 LightPosition;
						uniform vec3 LightIntensity;

						varying highp vec3 pos;
						varying highp vec2 tc0;
                        varying highp vec3 norm;	

						void main(void)
						{
							tc0 = vec2(a1.x, 1.0 - a1.y);
							//mat3 normalMatrix = mat3( transpose(inverse( mat4_mv )) ); //TODO: remove from shader
							norm = normalize( mat3_n * a2 );
							//norm = normalize( matrix * vec4(a2, 0) ).xyz;
							pos = vec3( mat4_mv * a0 );        
							gl_Position = matrix * a0;
						}

	)qwertyuiop",
	R"qwertyuiop(
		




				
						precision highp float;
		
						varying highp vec3 pos;
						varying highp vec2 tc0;
                        varying highp vec3 norm;

						uniform sampler2D texture0;
						
						uniform highp mat4 matrix;       // mvp matrix
						uniform highp mat4 mat4_mv;      // modelview matrix  
						//uniform highp mat4 mat4_p;       // projection matrix  
						uniform highp mat3 mat3_n;       // normal matrix (mat3)

						uniform vec4 LightPosition;
						uniform vec3 LightIntensity;

						const vec3 Kd = vec3(0.5, 0.5, 0.5);  		   // Diffuse reflectivity
						const vec3 Ka = vec3(0.1, 0.1, 0.1);  		   // Ambient reflectivity
						const vec3 Ks = vec3(0.7, 0.7, 0.7);  		   // Specular reflectivity
						const float Shininess = 20.0;                   // Specular shininess factor

						vec3 ads()
						{
							vec3 n = normalize( norm );
							vec3 s = normalize( vec3(LightPosition) - pos );
							vec3 v = normalize( vec3(-pos) );
							vec3 r = reflect( -s, n );
							return LightIntensity * ( Ka + Kd * max( dot(s, n), 0.0 ) + Ks * pow( max( dot(r,v), 0.0 ), Shininess ) ) * 2.0;
						}
						
						void main() 
						{	
							//float f = max( dot(norm, vec3(1.0, 0, 0)), 0.0);
							//vec3 light = LightIntensity * (f + 0.03);
							//gl_FragColor = vec4(light, 1.0);
							gl_FragColor = vec4(ads(), 1.0) * texture2D(texture0, tc0);
							//gl_FragColor = texture2D(texture0, tc0);
							//gl_FragColor = vec4(LightIntensity, 1.0);
							//gl_FragColor = vec4(LightIntensity, 1);
						}

	)qwertyuiop"
	),
	//texture_diffuse_uniform(this->get_uniform("texture0")),
	//texture_normal_uniform(this->get_uniform("texture1"))
	mat4_modelview(this->get_uniform("mat4_mv")),
	mat4_projection(this->get_uniform("mat4_p")),
	mat3_normal(this->get_uniform("mat3_n")),
	vec4_light_position(this->get_uniform("LightPosition")),
	vec3_light_intensity(this->get_uniform("LightIntensity"))
{
}

void shader_car::bind_me()
{
	this->bind();
}

void shader_car::render(const r4::matrix4<float>& m, const ruis::vertex_array& va, const ruis::texture_2d& tex) const
{
	ASSERT(dynamic_cast<const ruis::render_opengles::texture_2d*>(&tex))
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render_opengles::texture_2d&>(tex).bind(0);
	this->bind();

	this->shader_base::render(m, va);
}

void shader_car::set_uniform_matrix3f(GLint id, const r4::matrix3<float>& m) const
{
	if(id < 0)
		return;
	auto mm = m.tposed();
	glUniformMatrix3fv(
		id,
		1,
		// OpenGL ES 2 does not support transposing, see description of
		// 'transpose' parameter of glUniformMatrix4fv():
		// https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
		GL_FALSE,
		mm.front().data()
	);
	//ruis::render_opengles::assert_opengl_no_error();
}

void shader_car::set_uniform_matrix4f(GLint id, const r4::matrix4<float>& m) const
{
	if(id < 0)
		return;
	auto mm = m.tposed();
	glUniformMatrix4fv(
		id,
		1,
		// OpenGL ES 2 does not support transposing, see description of
		// 'transpose' parameter of glUniformMatrix4fv():
		// https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
		GL_FALSE,
		mm.front().data()
	);
	//ruis::render_opengles::assert_opengl_no_error();
}

void shader_car::set_uniform3f(GLint id, float x, float y, float z) const
{
	if(id < 0)
		return;
	glUniform3f(id, x, y, z);
	//ruis::render_opengles::assert_opengl_no_error();
}

void shader_car::set_uniform4f(GLint id, float x, float y, float z, float w) const
{
	if(id < 0)
		return;
	glUniform4f(id, x, y, z, w);
	//ruis::render_opengles::assert_opengl_no_error();
}

class shader_base_fake
{
public:
	ruis::render_opengles::program_wrapper program;
	const GLint matrix_uniform = 0;
	virtual ~shader_base_fake(){}
};

GLint shader_car::get_uniform(const char* name)
{
	int i = sizeof(shader_base_fake);
	int j = sizeof(ruis::render_opengles::shader_base);

	std::cout<< i << " " << j << std::endl;
	ruis::render_opengles::shader_base* sbf = static_cast<ruis::render_opengles::shader_base*>(this);
	shader_base_fake *rbf = reinterpret_cast<shader_base_fake*>( sbf );

	GLint ret = glGetUniformLocation(rbf->program.id, name);
	//assert_opengl_no_error();
	// if (ret < 0) {
	// 	throw std::logic_error(utki::cat("No uniform found in the shader program: ", name));
	// }
	return ret;
}
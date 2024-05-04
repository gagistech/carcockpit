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

#include "shader_adv.hpp"

#include <ruis/render/opengles/index_buffer.hpp>
#include <ruis/render/opengles/texture_2d.hpp>
#include <ruis/render/opengles/util.hpp>
#include <ruis/render/opengles/vertex_array.hpp>
#include <ruis/render/opengles/vertex_buffer.hpp>

using namespace carcockpit;

static ruis::mat3 from_mat4(const ruis::mat4& mat)
{
	ruis::mat3 m;
	for(int i = 0; i < 3; ++i)
		for(int j = 0; j < 3; ++j)
			m[i][j] = mat[i][j];
	return m;
}

static const ruis::vec4 default_light_position {5.0f, 5.0f, 5.0f, 1.0f};
static const ruis::vec3 default_light_intensity {2.0f, 2.0f, 2.0f};

shader_adv::shader_adv() :
	shader_base(
	R"qwertyuiop(
						attribute highp vec4 a0; // position
						attribute highp vec2 a1; // texture coordinate
                        attribute highp vec3 a2; // normal
						attribute highp vec3 a3; // tangent
						attribute highp vec3 a4; // bitangent

						uniform highp mat4 matrix;       // mvp matrix
						uniform highp mat4 mat4_mv;      // modelview matrix  
						uniform highp mat4 mat4_p;       // projection matrix  
						uniform highp mat3 mat3_n;       // normal matrix (mat3)

						uniform vec4 light_position;
						uniform vec3 light_intensity;

						varying highp vec3 light_dir;
						varying highp vec3 view_dir;
						varying highp vec2 tc;		
						
						void main()
						{
							// Transform normal and tangent to eye space
							vec3 norm = 	normalize(mat3_n * a2);
							vec3 tang = 	normalize(mat3_n * a3);
							vec3 binormal = normalize(mat3_n * a4);     // normalize( cross( norm, tang ) ) * a3.w; if bitangent not provided
							// Matrix for transformation to tangent space
							mat3 mat3_to_local = mat3(  tang.x, binormal.x, norm.x,
														tang.y, binormal.y, norm.y,
														tang.z, binormal.z, norm.z ) ;
							// Get the position in eye coordinates
							vec3 pos = vec3( mat4_mv * a0 );   
							// Transform light dir. and view dir. to tangent space
							light_dir = normalize( mat3_to_local * (light_position.xyz - pos) );
							view_dir = mat3_to_local * normalize(-pos);
							// Pass along the texture coordinate	
							tc = vec2(a1.x, 1.0 - a1.y);                  // * vec2(10, 10); // for supposed x10 tiling, if enabled in texture
							gl_Position = matrix * a0;
						}
						
	)qwertyuiop",
	R"qwertyuiop(
						precision highp float;

						varying highp vec3 light_dir;
						varying highp vec3 view_dir;
						varying highp vec2 tc;	

						uniform sampler2D texture0;   // color map tex
						uniform sampler2D texture1;   // normal map tex
						uniform sampler2D texture2;   // roughness map tex

						uniform vec4 light_position;
						uniform vec3 light_intensity;

						uniform vec3 set_normal_mapping; // put (0, 0, 1) to disable, (1, 1, 1) to enable
						
						struct MaterialInfo {   vec3 Ka; vec3 Kd; vec3 Ks; float Shininess; };
						//uniform MaterialInfo Material;
						const MaterialInfo Material = MaterialInfo( vec3(0.1, 0.1, 0.1), vec3(0.5, 0.5, 0.5), vec3(0.7, 0.7, 0.7), 40.0 );

						const vec3 Kd = vec3(0.5, 0.5, 0.5);  		   // Diffuse reflectivity
						const vec3 Ka = vec3(0.1, 0.1, 0.1);  		   // Ambient reflectivity
						const vec3 Ks = vec3(0.7, 0.7, 0.7);  		   // Specular reflectivity
						const float Shininess = 40.0;                  // Specular shininess factor
					
						// vec3 phongModelCB( vec3 norm, vec3 diffR, float glossFactor ) 
						// {
						// 	vec3 r = reflect( -light_dir, norm );
						// 	vec3 ambient = light_intensity * Material.Ka;
						// 	float sDotN = max( dot(light_dir, norm) , 0.0 );
						// 	vec3 diffuse = light_intensity * diffR * sDotN;
						// 	vec3 spec = vec3(0.0);
						// 	if( sDotN > 0.0 )
						// 		spec = light_intensity * Material.Ks * pow( max( dot(r, view_dir), 0.0 ), glossFactor );
						// 	return ambient + diffuse + spec;
						// }

						vec3 phongModel( vec3 norm, vec3 diffR, float glossFactor ) 
						{
							vec3 r = reflect( -light_dir, norm );
							float sDotN = max( dot(light_dir, norm) , 0.0 );

							vec3 ambient = Ka                                                    * light_intensity;
							vec3 diffuse = Kd       * sDotN                                      * light_intensity;
							vec3 spec    = Ks * pow( max( dot(r, view_dir), 0.0 ), glossFactor ) * light_intensity;

							//vec3 spec = vec3(0.0);
							//if( sDotN > 0.0 )
								//spec = light_intensity * Ks * pow( max( dot(r, view_dir), 0.0 ), glossFactor );
							
							return   ( ambient + diffuse + spec ) * diffR;
						}

						// vec3 ads()
						// {
						// 	vec3 n = normalize( norm );
						// 	vec3 s = normalize( vec3(light_position) - pos );
						// 	vec3 v = normalize( vec3(-pos) );
						// 	vec3 r = reflect( -s, n );
						// 	return light_intensity * ( Ka + Kd * max( dot(s, n), 0.0 ) + Ks * pow( max( dot(r,v), 0.0 ), Shininess ) );
						// }

						void main() 
						{
							float rough = texture2D( texture2, tc ).x;
							float gloss = ((1.0 - rough) * 80.0 + 1.0 );

							//vec3 normal = vec4(0, 0, 1.0); 
							vec4 normal4 = 2.0 * texture2D( texture1, tc ) - 1.0;
							vec3 normal = normalize(normal4.xyz * set_normal_mapping);	
							normal = vec3(normal.x, -normal.y, normal.z);						
							vec4 texColor     = texture2D( texture0, tc );   							
							gl_FragColor = vec4( phongModel( normal.xyz, texColor.rgb, 40.0), 1.0 );
						}

	)qwertyuiop"
	),
	sampler_normal_map(this->get_uniform("texture1")),
	sampler_roughness_map(this->get_uniform("texture2")),
	mat4_modelview(this->get_uniform("mat4_mv")),
	mat4_projection(this->get_uniform("mat4_p")),
	mat3_normal(this->get_uniform("mat3_n")),
	vec4_light_position(this->get_uniform("light_position")),
	vec3_light_intensity(this->get_uniform("light_intensity")),
	vec3_set_normal_mapping(this->get_uniform("set_normal_mapping"))
{
}

void shader_adv::setNormalMapping(bool on)
{
	if(on)
		set_normal_mapping_vector = ruis::vec3{1, 1, 1};
	else
		set_normal_mapping_vector = ruis::vec3{0, 0, 1};

	LOG([&](auto& o) { o << "normals " << (on ? "on" : "off") << std::endl; })
}

void shader_adv::render(const ruis::render::vertex_array& va,
			const r4::matrix4<float>& mvp,
			const r4::matrix4<float>& modelview, 
			const r4::matrix4<float>& projection,  
			const ruis::render::texture_2d& tex_color,
			const ruis::render::texture_2d& tex_normal,
			const ruis::render::texture_2d& tex_roughness,
			const ruis::vec4& light_pos = default_light_position,
			const ruis::vec3& light_int = default_light_intensity
			) const
{
	this->bind(); // bind shader program

	this->set_uniform_sampler(sampler_normal_map, 1);
	this->set_uniform_sampler(sampler_roughness_map, 2);

	ASSERT(dynamic_cast<const ruis::render::opengles::texture_2d*>(&tex_color));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render::opengles::texture_2d&>(tex_color).bind(0);
	ASSERT(dynamic_cast<const ruis::render::opengles::texture_2d*>(&tex_normal));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render::opengles::texture_2d&>(tex_normal).bind(1);
	ASSERT(dynamic_cast<const ruis::render::opengles::texture_2d*>(&tex_roughness));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
	static_cast<const ruis::render::opengles::texture_2d&>(tex_roughness).bind(2);
	
	ruis::mat3 normal = from_mat4(modelview); 
	normal.invert();
	normal.transpose();

	this->set_uniform4f(this->vec4_light_position, light_pos[0], light_pos[1], light_pos[2], light_pos[3]);
	this->set_uniform3f(this->vec3_light_intensity, light_int[0], light_int[1], light_int[2]);
	this->set_uniform_matrix4f(this->mat4_modelview, modelview);
	this->set_uniform_matrix4f(this->mat4_projection, projection);
	this->set_uniform_matrix3f(mat3_normal, normal);

	this->set_uniform3f(this->vec3_set_normal_mapping, set_normal_mapping_vector[0], set_normal_mapping_vector[1], set_normal_mapping_vector[2]);

	this->shader_base::render(mvp, va);
}

void shader_adv::set_uniform_matrix3f(GLint id, const r4::matrix3<float>& m) const
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
	//ruis::render::opengles::assert_opengl_no_error();
}

void shader_adv::set_uniform_matrix4f(GLint id, const r4::matrix4<float>& m) const
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
	//ruis::render::opengles::assert_opengl_no_error();
}

void shader_adv::set_uniform3f(GLint id, float x, float y, float z) const
{
	if(id < 0)
		return;
	glUniform3f(id, x, y, z);
	//ruis::render::opengles::assert_opengl_no_error();
}

void shader_adv::set_uniform4f(GLint id, float x, float y, float z, float w) const
{
	if(id < 0)
		return;
	glUniform4f(id, x, y, z, w);
	//ruis::render::opengles::assert_opengl_no_error();
}

class shader_base_fake
{
public:
	ruis::render::opengles::program_wrapper program;
	const GLint matrix_uniform = 0;
	virtual ~shader_base_fake(){}
};

GLint shader_adv::get_uniform(const char* name)
{
	int i = sizeof(shader_base_fake);
	int j = sizeof(ruis::render::opengles::shader_base);

	std::cout<< i << " " << j << std::endl;
	ruis::render::opengles::shader_base* sbf = static_cast<ruis::render::opengles::shader_base*>(this);
	shader_base_fake *rbf = reinterpret_cast<shader_base_fake*>( sbf );

	GLint ret = glGetUniformLocation(rbf->program.id, name);
	//assert_opengl_no_error();
	// if (ret < 0) {
	// 	throw std::logic_error(utki::cat("No uniform found in the shader program: ", name));
	// }
	return ret;
}
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

#include <ruis/context.hpp>
#include <ruis/render/renderer.hpp>
#include <ruis/res/texture_2d.hpp>
#include <ruis/res/texture_cube.hpp>


// #include "shaders/shader_adv.hpp"
// #include "shaders/shader_phong.hpp"
// #include "shaders/shader_skybox.hpp"

#include "node.hpp"
#include "scene.hpp"

namespace ruis::render {

class node;
class camera;
class light;

class scene_renderer
{
protected:
	std::shared_ptr<ruis::render::scene> scene_v;
	std::shared_ptr<ruis::render::camera> external_camera;
	// ruis::render::renderer& r;
	utki::shared_ref<ruis::context> context_v;
	ruis::mat4 view_matrix;
	ruis::mat4 projection_matrix;
	light main_light;

	std::shared_ptr<ruis::res::texture_2d> texture_default_white;
	std::shared_ptr<ruis::res::texture_2d> texture_default_black;
	std::shared_ptr<ruis::res::texture_2d> texture_default_normal;
	std::shared_ptr<ruis::res::texture_cube> texture_default_environment_cube;
	// camera main_camera;
	void render_node(utki::shared_ref<node> n, ruis::mat4 parent_model_matrix);
	// void render_mesh(utki::shared_ref<node> n, ruis::mat4 model);
	// void render_primitive(utki::shared_ref<node> n, ruis::mat4 model);

public:
	scene_renderer(utki::shared_ref<ruis::context> c);
	void render();
	void set_scene(std::shared_ptr<ruis::render::scene> scene_v);
	void set_external_camera(std::shared_ptr<ruis::render::camera> cam);

	virtual ~scene_renderer() {}
};

// class scene_renderer_regular : public scene_renderer
// {
// 	// std::shared_ptr<shader_skybox> skybox_shader;
// 	// std::shared_ptr<shader_phong> phong_shader;
// 	// std::shared_ptr<shader_adv> advanced_shader;

// public:
// 	ruis::mat4 view_matrix;

// 	void render_node(utki::shared_ref<node> n, ruis::mat4 model) override;
// 	scene_renderer_regular(ruis::render::renderer& r);
// };

// class shadow_scene_renderer : public scene_renderer{
// public:
//   const matrix shadow_matrix;

//   void render_node(const node& n, matrix model)override{
//     // efwefew
//   }
// };

// auto shadow_tex;

// scene::render(){
//   ordinary_scene_renderer or;

//   or.render(root_node);

//   if(shadow_cache_dirty){
//     shadow_scene_renderer sr(light_pos);

//     sr.render(root_node);

//     shadow_tex = sr.get_result();
//   }
// }

} // namespace ruis::render
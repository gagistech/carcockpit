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

// #include "shaders/shader_adv.hpp"
// #include "shaders/shader_phong.hpp"
// #include "shaders/shader_skybox.hpp"

#include "node.hpp"
#include "scene.hpp"

namespace ruis::render {

class node;

class scene
{
public:
	std::string name;
	// utki::shared_ref<ruis::context> context_;
	std::vector<utki::shared_ref<node>> nodes;
	// std::vector<utki::shared_ref<mesh>> meshes; // mesh order is important on loading stage
	// scene(utki::shared_ref<ruis::context> c);
	scene();
};

class scene_renderer
{
protected:
	ruis::mat4 projection;
	virtual void render_node(utki::shared_ref<node> n, ruis::mat4 model) = 0;

public:
	void render(utki::shared_ref<node> n, ruis::mat4 parent_model);

	virtual ~scene_renderer() {}
};

class scene_renderer_regular : public scene_renderer
{
	// std::shared_ptr<shader_skybox> skybox_shader;
	// std::shared_ptr<shader_phong> phong_shader;
	// std::shared_ptr<shader_adv> advanced_shader;

public:
	ruis::mat4 view_matrix;

	void render_node(utki::shared_ref<node> n, ruis::mat4 model) override;
	scene_renderer_regular();
};

} // namespace ruis::render

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

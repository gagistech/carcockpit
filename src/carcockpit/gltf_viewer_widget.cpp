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

#include "gltf_viewer_widget.hpp"

#include <ratio>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <papki/fs_file.hpp>
#include <ruis/render/opengles/texture_2d.hpp>
#include <ruis/res/texture_cube.hpp>

#include "../ruis/render/scene/gltf_loader.hpp"

#include "application.hpp"

using namespace carcockpit;
using namespace ruis::render;

gltf_viewer_widget::gltf_viewer_widget(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)}),
	ruis::fraction_widget(this->context, {})
{
	// this->tex = this->context.get().loader.load<ruis::res::texture_2d>("tex_sample").to_shared_ptr();
	// this->rot.set_identity();

	int maxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	LOG([&](auto& o) {
		o << "Max texture size: " << maxTextureSize << std::endl;
	})
	LOG([&](auto& o) {
		o << "<< LOAD GLTF >>" << std::endl;
	})

	ruis::render::gltf_loader l(*this->context.get().renderer.get().factory);
	// demoscene = l.load(papki::fs_file("../res/samples_gltf/parent_and_children.glb")).to_shared_ptr();
	// demoscene = l.load(papki::fs_file("../res/samples_gltf/camera.glb")).to_shared_ptr();
	demoscene = l.load(papki::fs_file("../res/samples_gltf/spray.glb")).to_shared_ptr();

	sc_renderer = std::make_shared<ruis::render::scene_renderer>(this->context);
	sc_renderer->set_scene(demoscene);

	camrip = std::make_shared<ruis::render::camera>();
	sc_renderer->set_external_camera(camrip);
}

void gltf_viewer_widget::update(uint32_t dt)
{
	demoscene->update(dt);

	this->fps_sec_counter += dt;
	this->time += dt;
	[[maybe_unused]] float ft = static_cast<float>(this->time) / std::milli::den;
	float fdt = static_cast<float>(dt) / std::milli::den;
	++this->fps;

	// this->rot =
	// 	// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
	// 	(ruis::quaternion()
	// 		 .set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 2.0f * 3.1415926f * float(get_fraction()))) *
	// 	(ruis::quaternion().set_rotation(r4::vector3<float>(0, 1, 0).normalize(), 0.1f * ft));

	// if(camera_transition_ongoing)
	{
		camera_position += (camera_attractor - camera_position) * fdt / camera_transition_duration;
		ruis::vec3 remains = camera_attractor - camera_position;
		ruis::real l2 = remains.x() * remains.x() + remains.y() * remains.y() + remains.z() * remains.z();
		const ruis::real threshold = 0.00001;
		if (l2 < threshold) {
			// camera_transition_ongoing = false;
			camera_position = camera_attractor;
		}
	}

	if (this->fps_sec_counter >= std::milli::den) {
		std::cout << "fps = " << std::dec << fps << std::endl;
		this->fps_sec_counter = 0;
		this->fps = 0;
	}

	this->clear_cache();
}

void gltf_viewer_widget::toggleCamera(bool toggle)
{
	// camera_transition_ongoing = true;
	if (toggle)
		camera_attractor = camera_position_top;
	else
		camera_attractor = camera_position_front;
}

void gltf_viewer_widget::set_normal_mapping(bool toggle)
{
	application::inst().shader_adv_v.set_normal_mapping(toggle);
}

bool gltf_viewer_widget::on_mouse_button(const ruis::mouse_button_event& e)
{
	std::cout << "Is Down = " << e.is_down << std::endl;

	if (e.button == ruis::mouse_button::wheel_up) {
		camera_attractor /= 1.07;
	} else if (e.button == ruis::mouse_button::wheel_down) {
		camera_attractor *= 1.07;
	} else if (e.button == ruis::mouse_button::left) {
		mouse_rotate = e.is_down;
		if (e.is_down) {
			mouse_changeview_start = e.pos;
			camera_changeview_start = camera_position;
		}
	}
	return true;
}

bool gltf_viewer_widget::on_mouse_move(const ruis::mouse_move_event& e)
{
	if (mouse_rotate) {
		ruis::vec2 diff = e.pos - mouse_changeview_start;
		ruis::vec4 diff4 = diff;

		// ruis::mat4 inv_view = get_view_matrix().inv();
		// diff4 = inv_view * diff4;

		ruis::quat q1, q2;
		q1.set_rotation(0, 1, 0, -diff4.x() / 200.0f);
		// camera_attractor.rotate(q);
		q2.set_rotation(1, 0, 0, -diff4.y() / 200.0f);
		// camera_attractor.rotate(q);

		ruis::vec3 cam2go = camera_changeview_start;
		cam2go.rotate(q1);
		cam2go.rotate(q2);

		camera_attractor = cam2go;
	}

	return false;
}

bool gltf_viewer_widget::on_key(const ruis::key_event& e)
{
	return false;
}

ruis::mat4 gltf_viewer_widget::get_view_matrix() const
{
	ruis::mat4 view;
	view.set_identity();
	view.set_look_at(camera_position, camera_target, ruis::vec3(0, 2, 0));
	return view;
}

void gltf_viewer_widget::render(const ruis::matrix4& matrix) const
{
	this->widget::render(matrix);

	ruis::mat4 viewport_matrix{matrix};
	viewport_matrix.scale(this->rect().d / 2);
	viewport_matrix.translate(1, 1);
	viewport_matrix.scale(1, -1, -1);

	ruis::mat4 viewport;
	viewport.set_identity();

	//	viewport = matrix;
	//	viewport.scale(1.0f / this->rect().d[0], 1.0f / this->rect().d[1], 1.0f);

	// float fms = static_cast<float>(this->time) / std::milli::den;
	// float xx = 3 * cosf(fms / 2);
	// float zz = 3 * sinf(fms / 2);

	// glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT); // TODO: probably this should be done externally,
								  // because it clears all the framebuffer, not just area of this widget

	camrip->pos = camera_position;
	camrip->target = camera_target;
	camrip->up = ruis::vec3(0, 2, 0);
	camrip->fovy = 3.1415926535f / 4.f;
	camrip->near = .1f;
	camrip->far = 20.f;

	this->sc_renderer->render(rect(), viewport_matrix);

	// advanced_s->render(
	// 	*this->car_vao,
	// 	mvp,
	// 	modelview,
	// 	projection,
	// 	this->tex_rust_diffuse->tex(),
	// 	this->tex_rust_normal->tex(),
	// 	this->tex_rust_roughness->tex(),
	// 	this->tex_cube_env_hata->tex(),
	// 	light_pos_view,
	// 	light_int
	// );

	glDisable(GL_DEPTH_TEST);
}

/// How to register mouse events outside app window ?
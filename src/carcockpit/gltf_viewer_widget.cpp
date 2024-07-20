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

#include "../ruis/render/scene/gltf_loader.hxx"

#include "application.hpp"

using namespace carcockpit;
using namespace ruis::render;

gltf_viewer_widget::gltf_viewer_widget(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)}),
	ruis::fraction_widget(this->context, {}),
	params(std::move(params.gltf_params))
{
	LOG([&](auto& o) {
		o << "[LOAD GLTF] " << this->params.path_to_gltf << std::endl;
	})

	ruis::render::gltf_loader l(*this->context.get().renderer.get().factory);

	demoscene = l.load(papki::fs_file(this->params.path_to_gltf)).to_shared_ptr();

	sc_renderer = std::make_shared<ruis::render::scene_renderer>(this->context);
	sc_renderer->set_scene(demoscene);

	camrip = std::make_shared<ruis::render::camera>();
	sc_renderer->set_external_camera(camrip);
	sc_renderer->set_scene_scaling_factor(this->params.scaling_factor);
	sc_renderer->set_environment_cube(this->params.environment_cube);
}

void gltf_viewer_widget::update(uint32_t dt)
{
	demoscene->update(dt);

	this->fps_sec_counter += dt;
	this->time += dt;
	[[maybe_unused]] float ft = static_cast<float>(this->time) / std::milli::den;
	float fdt = static_cast<float>(dt) / std::milli::den;
	++this->fps;

	float xx = 3 * cosf(ft / 2);
	float zz = 3 * sinf(ft / 2);

	auto light = demoscene->get_primary_light();
	if (light) {
		light->pos = {xx, 3, zz, 1};
	}

	if (this->fps_sec_counter >= std::milli::den) {
		// std::cout << "fps = " << std::dec << fps << std::endl;
		this->fps_sec_counter = 0;
		this->fps = 0;
	}

	camera_position += (camera_attractor - camera_position) * fdt / camera_transition_duration;
	ruis::vec3 remains = camera_attractor - camera_position;
	ruis::real l2 = remains.x() * remains.x() + remains.y() * remains.y() + remains.z() * remains.z();
	const ruis::real threshold = 0.00001;
	if (l2 < threshold) {
		camera_position = camera_attractor;
	}

	this->clear_cache();
}

void gltf_viewer_widget::toggle_camera(bool toggle)
{
	if (toggle)
		camera_attractor = camera_position_top;
	else
		camera_attractor = camera_position_front;
}

void gltf_viewer_widget::set_normal_mapping(bool toggle)
{
	application::inst().shader_adv_v.set_normal_mapping(toggle);
}

constexpr float snap_speed = 1.07; // 1 is zero speed

bool gltf_viewer_widget::on_mouse_button(const ruis::mouse_button_event& e)
{
	if (e.button == ruis::mouse_button::wheel_up) {
		camera_attractor -= this->params.camera_target;
		camera_attractor /= snap_speed;
		camera_attractor += this->params.camera_target;

		if (!this->params.smooth_navigation_zoom) {
			camera_position -= this->params.camera_target;
			camera_position /= snap_speed;
			camera_position += this->params.camera_target;
		}

	} else if (e.button == ruis::mouse_button::wheel_down) {
		camera_attractor -= this->params.camera_target;
		camera_attractor *= snap_speed;
		camera_attractor += this->params.camera_target;

		if (!this->params.smooth_navigation_zoom) {
			camera_position -= this->params.camera_target;
			camera_position *= snap_speed;
			camera_position += this->params.camera_target;
		}

	} else if (e.button == ruis::mouse_button::left) {
		mouse_orbit = e.is_down;
		if (e.is_down) {
			mouse_changeview_start = e.pos;
			camera_changeview_start = camera_position;
		}
	}
	return true;
}

bool gltf_viewer_widget::on_mouse_move(const ruis::mouse_move_event& e)
{
	constexpr float mouse_orbit_speed_multiplier = 2.0f;

	if (mouse_orbit) {
		ruis::vec2 diff = e.pos - mouse_changeview_start;

		ruis::vec3 cam2go = camera_changeview_start - this->params.camera_target;
		ruis::vec3 axis = -cam2go.cross(ruis::vec3(0, 1, 0));
		axis.normalize();

		auto cam_norm = cam2go.normed();
		float theta = ::acosf(cam_norm.y());
		float dtheta = -diff.y() * mouse_orbit_speed_multiplier / (rect().d.y() + 1);

		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		float theta_lower_limit = static_cast<float>(M_PI_2) - params.orbit_angle_upper_limit + 0.00001f;
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		float theta_upper_limit = params.orbit_angle_lower_limit + static_cast<float>(M_PI_2) - 0.00001f;

		// restrict camera orbit angle to respect given range
		if (theta + dtheta < theta_lower_limit)
			dtheta = theta_lower_limit - theta;
		if (theta + dtheta > theta_upper_limit)
			dtheta = theta_upper_limit - theta;

		ruis::quat q1, q2;
		q1.set_rotation(axis.x(), axis.y(), axis.z(), dtheta);
		q2.set_rotation(0, 1, 0, -diff.x() * mouse_orbit_speed_multiplier / (rect().d.x() + 1));

		cam2go.rotate(q1);
		cam2go.rotate(q2);

		cam2go += this->params.camera_target;

		if (this->params.smooth_navigation_orbit)
			camera_attractor = cam2go;
		else
			camera_position = camera_attractor = cam2go;
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
	view.set_look_at(camera_position, this->params.camera_target, ruis::vec3(0, 1, 0));
	return view;
}

void gltf_viewer_widget::render(const ruis::matrix4& matrix) const
{
	this->widget::render(matrix);

	ruis::mat4 viewport_matrix{matrix};
	viewport_matrix.scale(this->rect().d / 2);
	viewport_matrix.translate(1, 1);
	viewport_matrix.scale(1, -1, -1);

	camrip->pos = camera_position;
	camrip->target = this->params.camera_target;
	camrip->up = ruis::vec3(0, 1, 0);
	camrip->fovy = M_PI_4;

	this->sc_renderer->render(rect(), viewport_matrix);
}

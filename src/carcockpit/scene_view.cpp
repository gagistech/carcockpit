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

#include "scene_view.hpp"

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

scene_view::scene_view(utki::shared_ref<ruis::context> context, all_parameters params) :
	ruis::widget(std::move(context), {.widget_params = std::move(params.widget_params)}),
	params(std::move(params.scene_params))
{
	LOG([&](auto& o) {
		o << "[LOAD GLTF] " << this->params.file << std::endl;
	})

	ruis::render::gltf_loader l(*this->context.get().renderer.get().factory);

	scene_v = l.load(papki::fs_file(this->params.file)).to_shared_ptr();

	scene_renderer_v = std::make_shared<ruis::render::scene_renderer>(this->context);
	scene_renderer_v->set_scene(scene_v);

	camera_v = std::make_shared<ruis::render::camera>();
	scene_renderer_v->set_external_camera(camera_v);
	scene_renderer_v->set_scene_scaling_factor(this->params.scaling_factor);
	scene_renderer_v->set_environment_cube(this->params.environment_cube);
}

void scene_view::update(uint32_t dt)
{
	scene_v->update(dt);

	this->fps_sec_counter += dt;
	this->time += dt;
	[[maybe_unused]] float time_sec = float(this->time) / std::milli::den;
	float dt_sec = float(dt) / std::milli::den;
	++this->fps;

	auto light = scene_v->get_primary_light();
	if (light) {
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		float light_x = 3 * cosf(time_sec / 2);
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		float light_z = 3 * sinf(time_sec / 2);
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		light->pos = {light_x, 3, light_z, 1};
	}

	if (this->fps_sec_counter >= std::milli::den) {
		std::cout << "fps = " << std::dec << fps << std::endl;
		this->fps_sec_counter = 0;
		this->fps = 0;
	}

	camera_position += (camera_attractor - camera_position) * dt_sec / camera_transition_duration;
	ruis::vec3 remains = camera_attractor - camera_position;

	constexpr ruis::real threshold = 1e-6;

	if (remains.norm_pow2() < threshold) {
		camera_position = camera_attractor;
	}

	this->clear_cache();
}

constexpr float snap_speed = 1.07; // 1 is zero speed

bool scene_view::on_mouse_button(const ruis::mouse_button_event& e)
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
		mouse_is_orbiting = e.is_down;
		if (e.is_down) {
			mouse_changeview_start = e.pos;
			camera_changeview_start = camera_position;
		}
	}
	return true;
}

bool scene_view::on_mouse_move(const ruis::mouse_move_event& e)
{
	constexpr float mouse_orbit_speed_multiplier = 2.0f;

	if (mouse_is_orbiting) {
		ruis::vec2 diff = e.pos - mouse_changeview_start;

		ruis::vec3 cam_pos_relative = camera_changeview_start - this->params.camera_target;
		ruis::vec3 axis = -cam_pos_relative.cross(ruis::vec3(0, 1, 0));
		axis.normalize();

		auto cam_norm = cam_pos_relative.normed();
		float theta = ::acosf(cam_norm.y());
		float dtheta = -diff.y() * mouse_orbit_speed_multiplier / (rect().d.y() + 1);

		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		float theta_lower_limit = float(utki::pi) / 2 - params.orbit_angle_upper_limit + 0.00001f;
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
		float theta_upper_limit = params.orbit_angle_lower_limit + float(utki::pi) / 2 - 0.00001f;

		// restrict camera orbit angle to respect given range
		if (theta + dtheta < theta_lower_limit)
			dtheta = theta_lower_limit - theta;
		if (theta + dtheta > theta_upper_limit)
			dtheta = theta_upper_limit - theta;

		ruis::quat q1, q2;
		q1.set_rotation(axis, dtheta);
		q2.set_rotation(0, 1, 0, -diff.x() * mouse_orbit_speed_multiplier / (rect().d.x() + 1));

		cam_pos_relative.rotate(q1);
		cam_pos_relative.rotate(q2);

		cam_pos_relative += this->params.camera_target;

		if (this->params.smooth_navigation_orbit)
			camera_attractor = cam_pos_relative;
		else
			camera_position = camera_attractor = cam_pos_relative;
	}

	return false;
}

bool scene_view::on_key(const ruis::key_event& e)
{
	return false;
}

void scene_view::render(const ruis::matrix4& matrix) const
{
	this->widget::render(matrix);

	ruis::mat4 viewport_matrix{matrix};
	viewport_matrix.scale(this->rect().d / 2);
	viewport_matrix.translate(1, 1);
	viewport_matrix.scale(1, -1, -1);

	camera_v->pos = camera_position;
	camera_v->target = this->params.camera_target;
	camera_v->up = ruis::vec3(0, 1, 0);
	camera_v->fovy = utki::pi / 4;

	this->scene_renderer_v->render(rect().d, viewport_matrix);
}

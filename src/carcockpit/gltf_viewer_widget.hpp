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

#include <ruis/res/texture_2d.hpp>
#include <ruis/res/texture_cube.hpp>
#include <ruis/updateable.hpp>
#include <ruis/widgets/base/fraction_widget.hpp>
#include <ruis/widgets/widget.hpp>

#include "../ruis/render/scene/scene.hpp"
#include "../ruis/render/scene/scene_renderer.hxx"

#include "shaders/shader_adv.hpp"
#include "shaders/shader_phong.hpp"
#include "shaders/shader_skybox.hpp"

namespace carcockpit {

constexpr ruis::vec3 default_camera_position_front{0, 1.5, 7};
constexpr ruis::vec3 default_camera_position_top{0, 9, 0};
constexpr float default_camera_transition_duration{0.1};

class gltf_viewer_widget : public ruis::fraction_widget, public ruis::updateable, virtual public ruis::widget
{
	std::shared_ptr<ruis::render::scene> demoscene;
	std::shared_ptr<ruis::render::scene_renderer> sc_renderer;
	std::shared_ptr<ruis::render::camera> camrip;

	ruis::vec3 camera_position_front{default_camera_position_front};
	ruis::vec3 camera_position_top{default_camera_position_top};
	ruis::vec3 camera_position{camera_position_top};
	ruis::vec3 camera_attractor{camera_position_front};

	ruis::real camera_transition_duration{default_camera_transition_duration};
	bool mouse_orbit = false;
	ruis::vec2 mouse_changeview_start;
	ruis::vec3 camera_changeview_start;

	uint32_t fps = 0;
	uint32_t fps_sec_counter = 0;
	uint32_t time = 0;

public:
	struct parameters {
		std::string path_to_gltf;
		float scaling_factor{1.0f};
		ruis::vec3 camera_target{0, 0, 0};
		bool smooth_navigation_orbit = true;
		bool smooth_navigation_zoom = true;
		ruis::real orbit_angle_upper_limit = utki::pi / 2;
		ruis::real orbit_angle_lower_limit = utki::pi / 2;
		std::shared_ptr<ruis::res::texture_cube> environment_cube;
	};

private:
	parameters params;

public:
	struct all_parameters {
		ruis::widget::parameters widget_params;
		parameters gltf_params;
	};

	gltf_viewer_widget(utki::shared_ref<ruis::context> context, all_parameters params);

	void render(const ruis::matrix4& matrix) const override;
	void update(uint32_t dt) override;
	void toggle_camera(bool toggle);
	void set_normal_mapping(bool toggle);

	bool on_mouse_button(const ruis::mouse_button_event& e) override;
	bool on_mouse_move(const ruis::mouse_move_event& e) override;
	bool on_key(const ruis::key_event& e) override;

	ruis::mat4 get_view_matrix() const;
};

namespace make {
inline utki::shared_ref<gltf_viewer_widget> gltf_viewer_widget(
	utki::shared_ref<ruis::context> c,
	gltf_viewer_widget::all_parameters params
)
{
	return utki::make_shared<carcockpit::gltf_viewer_widget>(std::move(c), std::move(params));
}
} // namespace make

} // namespace carcockpit

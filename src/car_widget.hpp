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

#include <ruis/res/texture.hpp>
#include <ruis/updateable.hpp>
#include <ruis/widgets/widget.hpp>
#include <ruis/widgets/base/fraction_widget.hpp>
#include "car_view/model_obj.hpp"
#include "car_view/shaders/shader_phong.hpp"
#include "car_view/shaders/shader_adv.hpp"


namespace carcockpit {

class car_widget : public ruis::fraction_widget, public ruis::updateable, virtual public ruis::widget
{
	std::shared_ptr<ruis::res::texture> tex;
	std::shared_ptr<ruis::res::texture> tex_test;

	std::shared_ptr<ruis::res::texture> tex_car_diffuse;
	std::shared_ptr<ruis::res::texture> tex_car_normal;
	std::shared_ptr<ruis::res::texture> tex_car_roughness;
	//std::shared_ptr<ruis::res::texture> tex_car_opacity;
	//std::shared_ptr<ruis::res::texture> tex_car_metallic;

	std::shared_ptr<ruis::res::texture> tex_rust_diffuse;
	std::shared_ptr<ruis::res::texture> tex_rust_normal;
	std::shared_ptr<ruis::res::texture> tex_rust_roughness;

	ruis::quaternion rot = ruis::quaternion().set_identity();

	std::shared_ptr<ruis::render::vertex_array> cube_vao;
	std::shared_ptr<ruis::render::vertex_array> car_vao;
	std::shared_ptr<ruis::render::vertex_array> light_vao;
	std::shared_ptr<ruis::render::vertex_array> vao_lamba_l;
	std::shared_ptr<ruis::render::vertex_array> vao_lamba_r;


	std::shared_ptr<shader_phong> phong_s;
	std::shared_ptr<shader_adv>   advanced_s;

	ruis::vec3 camera_position_front {3.2, 1.4, 3.2};
	ruis::vec3 camera_position_top {0.1, 6, 0.1};

	ruis::vec3 camera_position {camera_position_top};
    ruis::vec3 camera_attractor {camera_position_front};

	ruis::real camera_transition_duration = 0.4; // not seconds ;)
	bool camera_transition_ongoing = true;

	unsigned fps = 0;
	uint32_t fps_sec_counter = 0;
	uint32_t time = 0;

public:
	struct parameters {};

private:
	// TODO: remove lint suppression
	// NOLINTNEXTLINE(clang-diagnostic-unused-private-field)
	parameters params;

public:
	struct all_parameters {
		ruis::widget::parameters widget_params;
		parameters car_params;
	};

	car_widget(utki::shared_ref<ruis::context> context, all_parameters params);

	void render(const ruis::matrix4& matrix) const override;
	void update(uint32_t dt) override;
	void toggleCamera(bool toggle);
	void setNormalMapping(bool toggle);
};

namespace make {
inline utki::shared_ref<car_widget> car_widget(utki::shared_ref<ruis::context> c, car_widget::all_parameters params)
{
	return utki::make_shared<carcockpit::car_widget>(std::move(c), std::move(params));
}
} // namespace make

} // namespace carcockpit

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

namespace carcockpit {

class car_widget : public ruis::widget, public ruis::updateable
{
	std::shared_ptr<ruis::res::texture> tex;

	ruis::quaternion rot = ruis::quaternion().set_identity();

	std::shared_ptr<ruis::vertex_array> cube_vao;

	unsigned fps = 0;
	uint32_t fps_sec_counter = 0;

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
};

namespace make {
inline utki::shared_ref<car_widget> car_widget(utki::shared_ref<ruis::context> c, car_widget::all_parameters params)
{
	return utki::make_shared<carcockpit::car_widget>(std::move(c), std::move(params));
}
} // namespace make

} // namespace carcockpit

/*
carcockpit - Car cockpit example GUI project

Copyright (C) 2024-2025 Gagistech Oy <gagisechoy@gmail.com>

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

#include <ruis/res/image.hpp>
#include <ruis/widget/base/blending_widget.hpp>
#include <ruis/widget/base/fraction_widget.hpp>
#include <ruis/widget/widget.hpp>
#include <utki/math.hpp>

namespace ruis {
class gauge :
	public ruis::blending_widget, //
	public ruis::fraction_widget,
	virtual public ruis::widget
{
public:
	struct parameters {
		utki::shared_ref<const ruis::res::image> arrow;
		std::shared_ptr<const ruis::res::image> shadow;

		real arm_fraction = 1;

		real start_angle_rad = 0;
		real end_angle_rad = 0;
	};

	struct all_parameters {
		ruis::layout::parameters layout_params;
		ruis::widget::parameters widget_params;
		ruis::blending_widget::parameters blending_params;
		parameters params;
	};

private:
	parameters params;

	std::shared_ptr<const ruis::render::texture_2d> arrow_tex;
	std::shared_ptr<const ruis::render::texture_2d> shadow_tex;

public:
	gauge(
		utki::shared_ref<ruis::context> c, //
		all_parameters params
	);

	void on_lay_out() override;

	void render(const ruis::mat4& matrix) const override;
};

namespace make {
inline utki::shared_ref<ruis::gauge> gauge(utki::shared_ref<ruis::context> c,//
	 ruis::gauge::all_parameters params)
{
	return utki::make_shared<ruis::gauge>(std::move(c), //
	std::move(params));
}
} // namespace make

} // namespace ruis

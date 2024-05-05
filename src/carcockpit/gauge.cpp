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

#include "gauge.hpp"

#include <ruis/util/util.hpp>

using namespace ruis;

gauge::gauge(utki::shared_ref<ruis::context> c, all_parameters p) :
	widget(std::move(c), {.widget_params = std::move(p.widget_params)}),
	blending_widget(this->context, std::move(p.blending_params)),
	fraction_widget(this->context, {}),
	params(std::move(p.params))
{}

void gauge::on_lay_out()
{
	auto arrow_dim = this->params.arrow.get().dims();
	real arm_length = arrow_dim.x() * this->params.arm_fraction;

	if (arm_length <= 0) {
		return;
	}

	auto scale = (std::max(this->rect().d.x(), this->rect().d.y()) / 2) / arm_length;

	this->arrow_tex = this->params.arrow.get().get(arrow_dim * scale).to_shared_ptr();

	if (this->params.shadow) {
		// TRACE(<< "this->shadow->dims() * scale = " << this->shadow->dims() *
		// scale << std::endl)
		this->shadow_tex = this->params.shadow->get(this->params.shadow->dims() * scale).to_shared_ptr();
		// TRACE(<< "this->shadow_tex->dims() = " << this->shadow_tex->dims() <<
		// std::endl)
	}
}

void gauge::render(const matrix4& matrix) const
{
	ASSERT(this->arrow_tex)

	if (!this->arrow_tex->dims.is_positive() || this->params.arm_fraction <= 0) {
		return;
	}

	this->set_blending_to_renderer();

	matrix4 matr(matrix);
	matr.translate(this->rect().d / 2);
	matr.scale(this->rect().d / 2);

	matrix4 mmm;
	mmm.set_identity();
	mmm.rotate(
		-(this->params.start_angle_rad +
		  (this->params.end_angle_rad - this->params.start_angle_rad) * this->get_fraction())
	);
	{
		auto div = this->arrow_tex->dims.x() * this->params.arm_fraction;
		ASSERT(div >= 0)
		mmm.scale(1 / div);
	}

	constexpr auto half = 0.5;

	if (this->shadow_tex && this->shadow_tex->dims.is_positive()) {
		auto arrow_fraction = this->arrow_tex->dims.x() / this->shadow_tex->dims.x();

		constexpr auto shadow_offset = real(0.025f);

		matrix4 m(matr);
		m *= matrix4().set_identity().translate(shadow_offset, shadow_offset) * mmm;
		m.scale(this->shadow_tex->dims);
		m.translate(-(1 - this->params.arm_fraction) * arrow_fraction - (1 - arrow_fraction) / 2, -half);
		this->shadow_tex->render(m);
	}

	{
		matrix4 m(matr);
		m *= mmm;
		m.scale(this->arrow_tex->dims);
		m.translate(-(1 - this->params.arm_fraction), -half);
		this->arrow_tex->render(m);
	}
}

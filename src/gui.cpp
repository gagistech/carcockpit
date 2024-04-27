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

#include "gui.hpp"

#include <ruis/widgets/container.hpp>
#include <ruis/widgets/label/image.hpp>
#include <ruis/widgets/slider/scroll_bar.hpp>

#include "car_widget.hpp"
#include "gauge.hpp"

using namespace std::string_literals;

using namespace carcockpit;

namespace m {
using namespace ruis::make;
using namespace carcockpit::make;
} // namespace m

utki::shared_ref<ruis::widget> carcockpit::make_root_layout(utki::shared_ref<ruis::context> c)
{
	using ruis::lp;

	// clang-format off
	auto w = m::container(
        c,
        {
            .container_params = {
                .layout = ruis::layout::column
            }
        },
        {
            // TODO: should be slider, not scroll bar, band is not needed
            utki::make_shared<ruis::horizontal_scroll_bar>(
                c,
                tml::read(R"(
                    id{gauge_slider}
                    lp{dx{fill}dy{30pp}}
                )")
            ),

            m::container(
                c,
                {
                    .widget_params = {
                        .lp = {
                            .dims = {lp::fill, lp::fill},
                            .weight = 1
                        }
                    },
                    .container_params = {
                        .layout = ruis::layout::row
                    }
                },
                {
                    m::container(
                        c,
                        {
                            .widget_params = {
                                .lp = {
                                    .dims = {lp::fill, lp::fill},
                                    .weight = 1
                                }
                            },
                            .container_params = {
                                .layout = ruis::layout::pile
                            }
                        },
                        {
                            m::image(
                                c,
                                {
                                    .widget_params = {
                                        .lp = {
                                            .dims = {lp::fill, lp::fill}
                                        }
                                    },
                                    .image_params = {
                                        .img = c.get().loader.load<ruis::res::image>("img_gauge_scale")
                                    }
                                }
                            ),
                            m::gauge(
                                c,
                                {
                                    .widget_params = {
                                        .id = "gauge"s,
                                        .lp = {
                                            .dims = {lp::fill, lp::fill}
                                        }
                                    },
                                    .params = {
                                        .arrow = c.get().loader.load<ruis::res::image>("img_gauge_arrow"),
                                        .shadow = c.get().loader.load<ruis::res::image>("img_gauge_arrow_shadow"),
                                        .arm_fraction = 0.75, // NOLINT
                                        .start_angle_rad = utki::deg_to_rad(225.0), // NOLINT
                                        .end_angle_rad = utki::deg_to_rad(-45.0) // NOLINT
                                    }
                                }
                            )
                        }
                    ),
                    m::car_widget(c,
                        {
                            .widget_params = {
                                .id = "car_widget"s,
                                .lp = {
                                    .dims = {lp::fill, lp::fill},
                                    .weight = 5 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                                },
                                .clip=true
                            }
                            // ,
                            // .params = {
                            //             .diffuse = c.get().loader.load<ruis::res::image>("img_gauge_arrow"),
                            //             .specular = c.get().loader.load<ruis::res::image>("img_gauge_arrow"),
                            //             .roughness = c.get().loader.load<ruis::res::image>("img_gauge_arrow_shadow"),
                            //             .normal = c.get().loader.load<ruis::res::image>("img_gauge_arrow_shadow")
                            //         }
                        }
                    )
                }
            )

        }
    );
	// clang-format on

	auto& gauge = w.get().get_widget_as<ruis::gauge>("gauge");
    auto& slider = w.get().get_widget_as<ruis::fraction_widget>("gauge_slider");
    auto& car_widget = w.get().get_widget_as<carcockpit::car_widget>("car_widget");

	slider.fraction_change_handler = [&cw = car_widget, &g = gauge](ruis::fraction_widget& s) 
    {
		g.set_fraction(s.get_fraction());
        //cw.set_rotation(s.fraction());
        cw.set_fraction(s.get_fraction());
	};

	auto car = w.get().try_get_widget_as<carcockpit::car_widget>("car_widget");

	c.get().updater.get().start(car, 0);

	return w;
}

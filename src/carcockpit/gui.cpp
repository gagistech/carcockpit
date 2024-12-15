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

#include <ruis/widget/container.hpp>
#include <ruis/widget/label/image.hpp>
#include <ruis/widget/slider/slider.hpp>

#include "gauge.hpp"
#include "scene_view.hpp"

using namespace std::string_literals;

using namespace ruis::length_literals;

using namespace carcockpit;

namespace m {
using namespace ruis::make;
using namespace carcockpit::make;
} // namespace m

utki::shared_ref<ruis::key_proxy> carcockpit::make_root_widgets(utki::shared_ref<ruis::context> c)
{
	using ruis::lp;

	// clang-format off
	auto kp = m::key_proxy(c,
        {
            .container_params = {
                .layout = ruis::layout::column
            }
        },
        {
            m::slider(c,
                {
                    .layout_params = {
                        .dims = {lp::fill, 30_pp} // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                    },
                    .widget_params = {
                        .id = "gauge_slider"s
                    },
                    .oriented_params = {
                        .vertical = false
                    }
                }
            ),
            m::container(
                c,
                {
                    .layout_params = {
                        .dims = {lp::fill, lp::fill},
                        .weight = 1
                    },
                    .container_params = {
                        .layout = ruis::layout::row
                    }
                },
                {
                    m::container(
                        c,
                        {
                            .layout_params = {
                                .dims = {lp::fill, lp::fill},
                                .weight = 1
                            },
                            .container_params = {
                                .layout = ruis::layout::pile
                            }
                        },
                        {
                            m::image(
                                c,
                                {
                                    .layout_params = {
                                        .dims = {lp::fill, lp::fill}
                                    },
                                    .image_params = {
                                        .img = c.get().loader.load<ruis::res::image>("img_gauge_scale")
                                    }
                                }
                            ),
                            m::gauge(
                                c,
                                {
                                    .layout_params = {
                                        .dims = {lp::fill, lp::fill}
                                    },
                                    .widget_params = {
                                        .id = "gauge"s
                                    },
                                    .params = {
                                        .arrow = c.get().loader.load<ruis::res::image>("img_gauge_arrow"),
                                        .shadow = c.get().loader.load<ruis::res::image>("img_gauge_arrow_shadow"),
                                        .arm_fraction = 0.75, // NOLINT
                                        .start_angle_rad = utki::deg_to_rad(-225.0), // NOLINT
                                        .end_angle_rad = utki::deg_to_rad(45.0) // NOLINT
                                    }
                                }
                            )
                        }
                    ),
                
                    m::scene_view(c,
                        {
                            .layout_params = {
                                .dims = {lp::fill, lp::fill},
                                .weight = 5 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                            },
                            .widget_params = {
                                .id = "scene_view_1"s,
                                .clip = true,
                                .depth = true
                            }
                            ,
                            .scene_params = {
                                .file = "../res/samples_gltf/spray.glb"s,
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .scaling_factor = 20.0f,
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .camera_target = ruis::vec3(0, 2, 0),
                                .smooth_navigation_orbit = false,
                                .smooth_navigation_zoom = false,
                                .orbit_angle_lower_limit = 0,
                                .environment_cube = c.get().loader.load<ruis::res::texture_cube>("tex_cube_env_italy").to_shared_ptr(),                               
                                .show_environment = true
                            }
                        }
                    ),

                    m::scene_view(c,
                        {
                            .layout_params = {
                                .dims = {lp::fill, lp::fill},
                                .weight = 5 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                            },
                            .widget_params = {
                                .id = "scene_view_2"s,
                                .clip = true,
                                .depth = true
                            }
                            ,
                            .scene_params = {
                                .file = "../res/samples_gltf/kar.glb"s,
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .scaling_factor = 2.0f,
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .camera_target = ruis::vec3(0, 2, 0),
                                .smooth_navigation_orbit = false,
                                .smooth_navigation_zoom = false,
                                .orbit_angle_lower_limit = utki::pi / 2,
                                .environment_cube = c.get().loader.load<ruis::res::texture_cube>("tex_cube_env_italy").to_shared_ptr(),                               
                                .show_environment = true
                            }
                        }
                    )
                }
            )
        }
    );


	auto& gauge = kp.get().get_widget_as<ruis::gauge>("gauge");
	auto& slider = kp.get().get_widget_as<ruis::fraction_widget>("gauge_slider");

	slider.fraction_change_handler = [&g = gauge](ruis::fraction_widget& s) {
		g.set_fraction(s.get_fraction());
	};

	auto viewer1 = kp.get().try_get_widget_as<carcockpit::scene_view>("scene_view_1");
	auto viewer2 = kp.get().try_get_widget_as<carcockpit::scene_view>("scene_view_2");

	c.get().updater.get().start(viewer1, 0);
	c.get().updater.get().start(viewer2, 0);

	return kp;
}

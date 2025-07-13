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

#include "gui.hpp"

#include <ruis/widget/container.hpp>
#include <ruis/widget/label/image.hpp>
#include <ruis/widget/slider/slider.hpp>

#include "application.hpp"
#include "gauge.hpp"
#include "scene_view.hpp"

using namespace std::string_literals;

using namespace ruis::length_literals;

using namespace carcockpit;

namespace m {
using namespace ruis::make;
using namespace carcockpit::make;
} // namespace m

namespace {
utki::shared_ref<ruis::push_button> make_close_button(utki::shared_ref<ruis::context> c)
{
	// clang-format off
    return m::push_button(c,
        {
            .layout_params{
                .dims = {ruis::dim::min, ruis::dim::fill}
            },
            .widget_params{
                .id = "close_button"s
            }
        },
        {
            m::image(c,
                {
                    .layout_params{
                        .dims = {ruis::dim::min, ruis::dim::fill}
                    },
                    .image_params{
                        .img = c.get().loader().load<ruis::res::image>("img_close"sv),
                        .keep_aspect_ratio = true
                    }
                }
            )
        }
    );
	// clang-format on
}
} // namespace

root_widget_info carcockpit::make_root_widget(utki::shared_ref<ruis::context> c)
{
	auto close_button = make_close_button(c);

	// clang-format off
	auto kp = m::key_proxy(c,
        {
            .container_params = {
                .layout = ruis::layout::column
            }
        },
        {
            m::row(c,
                {
                    .layout_params{
                        .dims = {ruis::dim::fill, 30_pp}// NOLINT(cppcoreguidelines-avoid-magic-numbers)
                    }
                },
                {
                    m::slider(c,
                        {
                            .layout_params = {
                                .dims = {ruis::dim::fill, ruis::dim::fill},
                                .weight = 1
                            },
                            .widget_params = {
                                .id = "gauge_slider"s
                            },
                            .oriented_params = {
                                .vertical = false
                            }
                        }
                    ),
                    close_button
                }
            ),
            m::container(
                c,
                {
                    .layout_params = {
                        .dims = {ruis::dim::fill, ruis::dim::fill},
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
                                .dims = {ruis::dim::fill, ruis::dim::fill},
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
                                        .dims = {ruis::dim::fill, ruis::dim::fill}
                                    },
                                    .image_params = {
                                        .img = c.get().loader().load<ruis::res::image>("img_gauge_scale")
                                    }
                                }
                            ),
                            m::gauge(
                                c,
                                {
                                    .layout_params = {
                                        .dims = {ruis::dim::fill, ruis::dim::fill}
                                    },
                                    .widget_params = {
                                        .id = "gauge"s
                                    },
                                    .params = {
                                        .arrow = c.get().loader().load<ruis::res::image>("img_gauge_arrow"),
                                        .shadow = c.get().loader().load<ruis::res::image>("img_gauge_arrow_shadow"),
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
                                .dims = {ruis::dim::fill, ruis::dim::fill},
                                .weight = 5 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                            },
                            .widget_params = {
                                .id = "scene_view_1"s,
                                .clip = true,
                                .depth = true
                            }
                            ,
                            .scene_params = {
                                .file = utki::cat(carcockpit::application::inst().res_path, "samples_gltf/spray.glb"),
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .scaling_factor = 10.0f,
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .camera_target = ruis::vec3(1, 1, 1),
                                .smooth_navigation_orbit = true,
                                .smooth_navigation_zoom = true,
                                .orbit_angle_upper_limit = utki::pi / 4,
		                        .orbit_angle_lower_limit = utki::pi / 4,
                                .environment_cube = c.get().loader().load<ruis::res::texture_cube>("tex_cube_env_castle").to_shared_ptr()                               
                            }
                        }
                    ),
                    m::scene_view(c,
                        {
                            .layout_params = {
                                .dims = {ruis::dim::fill, ruis::dim::fill},
                                .weight = 5 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                            },
                            .widget_params = {
                                .id = "scene_view_2"s,
                                .clip = true,
                                .depth = true
                            }
                            ,
                            .scene_params = {
                                .file = utki::cat(carcockpit::application::inst().res_path, "samples_gltf/camera.glb"),
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .scaling_factor = 10.0f,
                                // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                .camera_target = ruis::vec3(-1, -1, -1),
                                .smooth_navigation_orbit = false,
                                .smooth_navigation_zoom = false,
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

	auto& viewer1 = kp.get().get_widget_as<carcockpit::scene_view>("scene_view_1");
	auto& viewer2 = kp.get().get_widget_as<carcockpit::scene_view>("scene_view_2");

	c.get().updater.get().start(utki::make_shared_from(viewer1), 0);
	c.get().updater.get().start(utki::make_shared_from(viewer2), 0);

	return {
        .root_key_proxy = std::move(kp),
        .close_button = std::move(close_button)
    };
}

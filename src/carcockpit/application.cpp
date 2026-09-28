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

#include "application.hpp"

#include <ruis/standard_resources.hpp>
#include <utki/config.hpp>

#if CFG_OS_NAME != CFG_OS_NAME_EMSCRIPTEN
#	include <clargs/parser.hpp>
#endif

#include "gui.hpp"
#include "scene_view.hpp"

using namespace carcockpit;
using namespace std::string_literals;

namespace {
constexpr auto screen_width = 1024;
constexpr auto screen_height = 600;
} // namespace

application::application(
	bool windowed, //
	std::string_view res_path
) :
	ruisapp::application({
		.name = std::string(app_name) //
	}),
	res_path(fsif::as_dir(res_path))
{
	auto& win = this->make_window({
		.dims = {screen_width, screen_height},
		.title = std::string(app_name),
		.fullscreen = !windowed,
		.buffers = {ruisapp::buffer::depth}
	});

	win.gui.context.get().window().close_handler = [this]() {
		this->quit();
	};

	win.gui.context.get().loader().mount_res_pack(this->get_res_file(fsif::as_dir(this->res_path)).get());

	win.gui.default_key_handler = [this](const ruis::key_event& e) {
		if (e.action == ruis::button_action::press) {
			if (e.combo.key == ruis::key::escape) {
				this->quit();
			}
		}
	};

	win.gui.set_root(make_root_widget(win.gui.context));
}

std::unique_ptr<application> carcockpit::make_application(
	std::string_view executable, //
	utki::span<std::string_view> args
)
{
#if CFG_OS_NAME == CFG_OS_NAME_EMSCRIPTEN
	bool windowed = true;
	std::string res_path = "res/"s;
#else
	bool windowed = false;

	// TODO: look in /usr/local/share/carcockpit first?
	std::string res_path = utki::cat(
		"/usr/share/"sv, //
		application::app_name
	);
	// std::string res_path = "res/"s;

	clargs::parser p;

	p.add("window", "run in window mode", [&]() {
		windowed = true;
	});

	p.add(
		"res-path",
		utki::cat(
			"resources path, default = /usr/share/"sv, //
			application::app_name
		),
		[&](std::string_view v) {
			res_path = v;
		}
	);

	p.parse(args);
#endif

	return std::make_unique<application>(
		windowed, //
		res_path
	);
}

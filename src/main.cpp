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

#include <clargs/parser.hpp>
#include <ruisapp/application.hpp>

#include "gui.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {
constexpr std::string_view app_name = "carcockpit"sv;
} // namespace

namespace carcockpit {

class application : public ruisapp::application
{
public:
	application(bool window, std::string_view res_path) :
		ruisapp::application( //
			std::string(app_name),
			[]() {
				// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
				ruisapp::window_params wp(r4::vector2<unsigned>(1024, 600));
				wp.buffers.set(ruisapp::window_params::buffer::depth);
				return wp;
			}()
		)
	{
		this->set_fullscreen(!window);

		this->gui.init_standard_widgets(*this->get_res_file());

		this->gui.context.get().loader.mount_res_pack(*this->get_res_file(papki::as_dir(res_path)));

		auto kp = make_root_widgets(this->gui.context);
		kp.get().key_handler = [this](ruis::key_proxy&, const ruis::key_event& e) {
			if (e.is_down) {
				if (e.combo.key == ruis::key::escape) {
					this->quit();
				}
			}
			return false;
		};

		this->gui.set_root(std::move(kp));
	}
};

const ruisapp::application_factory app_fac([](auto executable, auto args) {
	bool window = false;

	// TODO: look in /usr/local/share/carcockpit first?
	std::string res_path = utki::cat("/usr/share/"sv, app_name);
	//std::string res_path = "res/"s; 

	clargs::parser p;

	p.add("window", "run in window mode", [&]() {
		window = true;
	});

	p.add("res-path", utki::cat("resources path, default = /usr/share/"sv, app_name), [&](std::string_view v) {
		res_path = v;
	});

	p.parse(args);

	return std::make_unique<application>(window, res_path);
});

} // namespace carcockpit

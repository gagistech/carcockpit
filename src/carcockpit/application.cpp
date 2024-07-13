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

#include "application.hpp"

#include "gltf_viewer_widget.hpp"
#include "gui.hpp"

using namespace carcockpit;
using namespace std::string_literals;

application::application(bool window, std::string_view res_path) :
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
			} else if (e.combo.key == ruis::key::space) {
				this->toggle_camera();
			} else if (e.combo.key == ruis::key::n) {
				this->toggle_normal_mapping();
			}
		}
		return false;
	};

	this->gui.set_root(std::move(kp));
}

void application::toggle_camera()
{
	cam_toggle = !cam_toggle;
	auto car_w = this->gui.get_root().try_get_widget_as<carcockpit::gltf_viewer_widget>("gltf_viewer_widget");
	if (car_w) {
		car_w->toggle_camera(cam_toggle);
	}
}

void application::toggle_normal_mapping()
{
	nm_toggle = !nm_toggle;
	auto car_w = this->gui.get_root().try_get_widget_as<carcockpit::gltf_viewer_widget>("gltf_viewer_widget");
	if (car_w) {
		car_w->set_normal_mapping(nm_toggle);
	}
}
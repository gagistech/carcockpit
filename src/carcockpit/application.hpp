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

#include <ruisapp/application.hpp>

#include "shaders/shader_adv.hpp"
#include "shaders/shader_phong.hpp"
#include "shaders/shader_skybox.hpp"

using namespace std::string_view_literals;

namespace carcockpit {

class application : public ruisapp::application
{
public:
	application(bool window, std::string_view res_path);

	void toggle_camera();
	void toggle_normal_mapping();

	static constexpr std::string_view app_name = "carcockpit"sv;

	static application& inst()
	{
		return dynamic_cast<application&>(ruisapp::application::inst());
	}

	ruis::render::shader_skybox shader_skybox_v;
	ruis::render::shader_phong shader_phong_v;
	ruis::render::shader_adv shader_adv_v;

private:
	bool cam_toggle = false;
	bool nm_toggle = true;
};

} // namespace carcockpit
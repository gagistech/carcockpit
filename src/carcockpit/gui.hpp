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

#include <ruis/widget/button/push_button.hpp>
#include <ruis/widget/proxy/key_proxy.hpp>

namespace carcockpit {

struct root_widget_info {
	utki::shared_ref<ruis::key_proxy> key_proxy;
	utki::shared_ref<ruis::push_button> close_button;
};

root_widget_info make_root_widget(utki::shared_ref<ruis::context> c);

} // namespace carcockpit

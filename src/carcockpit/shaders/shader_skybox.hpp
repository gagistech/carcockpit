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

#include <ruis/render/opengles/shader_base.hpp>
#include <ruis/render/texture_cube.hpp>

namespace ruis::render {
class shader_skybox : public ruis::render::opengles::shader_base
{
public:
	GLint mat3_inverse_modelview;

	shader_skybox();
	void render(
		const ruis::render::vertex_array& va,
		const r4::matrix4<float>& modelview,
		const r4::matrix4<float>& projection,
		const ruis::render::texture_cube& tex_env_cube
	) const;
};

} // namespace ruis::render
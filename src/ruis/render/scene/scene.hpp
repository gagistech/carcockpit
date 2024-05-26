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

#include <ruis/render/renderer.hpp>
#include <ruis/context.hpp>

#include "node.hpp"

namespace ruis::render {

class scene
{
public:
	//utki::shared_ref<ruis::context> context_;
	std::vector<utki::shared_ref<node>> nodes;
	// std::vector<utki::shared_ref<mesh>> meshes; // mesh order is important on loading stage
	//scene(utki::shared_ref<ruis::context> c);
	scene();
};

struct buffer_view // currently we support only one data buffer, the single data buffer located in the .glb file
{
	// utki::span<uint8_t> buffer; // for further development use
	uint32_t byteLength;
	uint32_t byte_offset;
	uint32_t target;

	buffer_view(uint32_t byteLength, uint32_t byte_offset, uint32_t target) :
		byteLength(byteLength),
		byte_offset(byte_offset),
		target(target)
	{}
};

struct accessor {
	utki::shared_ref<buffer_view> bv;
	uint32_t component_type;
	uint32_t count;
	enum class type {
		scalar,
		vec2,
		vec3,
		vec4,
		mat2,
		mat3,
		mat4
	} acctype;

	accessor(utki::shared_ref<buffer_view> bv, uint32_t component_type, uint32_t count, type acctype) :
		bv(bv),
		component_type(component_type),
		count(count),
		acctype(acctype)
	{}
};

utki::shared_ref<scene> read_gltf(const papki::file& fi, ruis::render::render_factory& rf);

} // namespace ruis::render
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

#include <jsondom/dom.hpp>
#include <ruis/context.hpp>
#include <ruis/render/renderer.hpp>

#include "mesh.hpp"
#include "node.hpp"
#include "scene.hpp"

namespace ruis::render {

struct buffer_view;
struct accessor;

class gltf_loader
{
	ruis::render::render_factory& render_factory_;
	bool use_short_indices; // opengles 2.0, for example, supports only 16-bit indices

	utki::span<const uint8_t> glb_binary_buffer;

	// order of items in arrays below is important on loading stage
	std::vector<utki::shared_ref<scene>> scenes; // all scenes that are listed in gltf file
	std::vector<utki::shared_ref<node>> nodes; // all nodes that are listed in gltf file
	std::vector<utki::shared_ref<mesh>> meshes; // meshes.
	std::vector<utki::shared_ref<accessor>> accessors; // accessors.
	std::vector<utki::shared_ref<buffer_view>> buffer_views; // bv's

	std::vector<std::vector<uint32_t>> child_indices; // storage for node child hierarchy (only during loading stage)

	inline int read_int(const jsondom::value& json, const std::string& name);
	inline bool read_int_checked(const jsondom::value& json, const std::string& name, int& value);
	inline bool read_uint32_checked(const jsondom::value& json, const std::string& name, uint32_t& value);
	inline float read_float(const jsondom::value& json, const std::string& name);
	inline const std::string& read_string(const jsondom::value& json, const std::string& name);
	inline bool read_string_checked(const jsondom::value& json, const std::string& name, std::string& value);
	bool read_uint_array_checked(const jsondom::value& json, const std::string& name, std::vector<uint32_t>& array);
	ruis::vec2 read_vec2(const jsondom::value& json, const std::string& name);
	ruis::vec3 read_vec3(const jsondom::value& json, const std::string& name);
	ruis::vec4 read_vec4(const jsondom::value& json, const std::string& name);
	ruis::quat read_quat(const jsondom::value& json, const std::string& name);

	template <typename T>
	std::shared_ptr<ruis::render::vertex_buffer> create_vertex_buffer_float(
		utki::span<const uint8_t> buffer,
		uint32_t acc_count,
		// uint32_t acc_offset,
		uint32_t acc_stride
	);

	template <typename T, size_t dimension>
	r4::vector<T, dimension> read_vec(const jsondom::value& json, const std::string& name);

	utki::shared_ref<buffer_view> read_buffer_view(const jsondom::value& buffer_view_json);
	utki::shared_ref<accessor> read_accessor(const jsondom::value& accessor_json);
	utki::shared_ref<mesh> read_mesh(const jsondom::value& mesh_json);
	utki::shared_ref<node> read_node(const jsondom::value& node_json);
	utki::shared_ref<scene> read_scene(const jsondom::value& scene_json);

public:
	utki::shared_ref<scene> load(const papki::file& fi);
	gltf_loader(ruis::render::render_factory& render_factory_, bool use_short_indices);
};

struct buffer_view // currently we support only one data buffer, the single data buffer located in the .glb file
{
	// utki::span<uint8_t> buffer; // for further development use
	uint32_t byte_length;
	uint32_t byte_offset;
	uint32_t byte_stride;

	enum class target {
		undefined = 0,
		array_buffer = 34962,
		element_Array_buffer = 34963
	} target_;

	buffer_view(uint32_t byte_length, uint32_t byte_offset, uint32_t byte_stride, target target_) :
		byte_length(byte_length),
		byte_offset(byte_offset),
		byte_stride(byte_stride),
		target_(target_)
	{}
};

struct accessor {
	utki::shared_ref<buffer_view> bv;
	uint32_t count;
	uint32_t byte_offset;
	// uint32_t byte_stride;

	enum class type {
		undefined = 0,
		scalar = 1,
		vec2 = 2,
		vec3 = 3,
		vec4 = 4,
		mat2 = 5, // whoa
		mat3 = 9,
		mat4 = 16
	} type_;

	enum class component_type {
		undefined = 0,
		act_signed_byte = 5120,
		act_unsigned_byte = 5121,
		act_signed_short = 5122,
		act_unsigned_short = 5123,
		act_unsigned_int = 5125,
		act_float = 5126
	} component_type_;

	std::shared_ptr<ruis::render::vertex_buffer> vbo;
	std::shared_ptr<ruis::render::index_buffer> ibo;

	accessor(
		utki::shared_ref<buffer_view> bv,
		uint32_t count,
		uint32_t byte_offset,
		// uint32_t byte_stride,
		type type_,
		component_type component_type_
	);
};

} // namespace ruis::render
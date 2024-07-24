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

#include <variant>

#include <jsondom/dom.hpp>
#include <ruis/context.hpp>
#include <ruis/render/renderer.hpp>

#include "mesh.hpp"
#include "node.hpp"
#include "scene.hpp"

namespace ruis::render {

struct buffer_view;
struct accessor;
struct image_l;
struct sampler_l;
class material;
using vertex_data_t = std::variant<
	std::vector<float>,
	std::vector<ruis::vec2>,
	std::vector<ruis::vec3>,
	std::vector<ruis::vec4>,
	std::vector<uint16_t>,
	std::vector<uint32_t>>;

class gltf_loader
{
	ruis::render::factory& factory_v;

	utki::span<const uint8_t> glb_binary_buffer;

	// order of items in arrays below is important during loading stage
	std::vector<utki::shared_ref<scene>> scenes;
	std::vector<utki::shared_ref<node>> nodes;
	std::vector<utki::shared_ref<mesh>> meshes;
	std::vector<utki::shared_ref<accessor>> accessors;
	std::vector<utki::shared_ref<buffer_view>> buffer_views;

	std::vector<utki::shared_ref<material>> materials;
	std::vector<utki::shared_ref<ruis::render::texture_2d>> textures;
	std::vector<utki::shared_ref<sampler_l>> samplers;
	std::vector<utki::shared_ref<image_l>> images;

	std::vector<std::vector<uint32_t>> child_indices; // storage for node child hierarchy (only during loading stage)

	template <typename tp_type>
	void create_vertex_buffer_float(
		utki::shared_ref<ruis::render::accessor>,
		utki::span<const uint8_t> buffer,
		uint32_t acc_count,
		uint32_t acc_stride
	);

	template <typename tp_type>
	std::vector<utki::shared_ref<tp_type>> read_root_array(
		std::function<tp_type(const jsondom::value& j)> read_func,
		const jsondom::value& root_json,
		const std::string& name
	);

	template <typename tp_type>
	utki::shared_ref<ruis::render::vertex_array> create_vao_with_tangent_space(
		utki::shared_ref<accessor> index_accessor,
		utki::shared_ref<accessor> position_accessor,
		utki::shared_ref<accessor> texcoord_0_accessor,
		utki::shared_ref<accessor> normal_accessor
	);

	utki::shared_ref<buffer_view> read_buffer_view(const jsondom::value& buffer_view_json);
	utki::shared_ref<accessor> read_accessor(const jsondom::value& accessor_json);
	utki::shared_ref<mesh> read_mesh(const jsondom::value& mesh_json);
	utki::shared_ref<node> read_node(const jsondom::value& node_json);
	utki::shared_ref<scene> read_scene(const jsondom::value& scene_json);

	utki::shared_ref<image_l> read_image(const jsondom::value& image_json);
	utki::shared_ref<sampler_l> read_sampler(const jsondom::value& sampler_json);
	utki::shared_ref<ruis::render::texture_2d> read_texture(const jsondom::value& texture_json);
	utki::shared_ref<material> read_material(const jsondom::value& material_json);

public:
	utki::shared_ref<scene> load(const papki::file& fi);
	gltf_loader(ruis::render::factory& factory_v);
};

struct buffer_view // currently we support only one data buffer, the single data buffer located in the .glb file
{
	uint32_t byte_length;
	uint32_t byte_offset;
	uint32_t byte_stride;

	enum class target {
		undefined = 0,
		array_buffer = 34962,
		element_array_buffer = 34963
	} target_v;

	buffer_view(uint32_t byte_length, uint32_t byte_offset, uint32_t byte_stride, target target_v) :
		byte_length(byte_length),
		byte_offset(byte_offset),
		byte_stride(byte_stride),
		target_v(target_v)
	{}
};

struct accessor {
	utki::shared_ref<buffer_view> bv;
	uint32_t count;
	uint32_t byte_offset;

	enum class type {
		undefined = 0,
		scalar = 1,
		vec2 = 2,
		vec3 = 3,
		vec4 = 4,
		mat2 = 5,
		mat3 = 9,
		mat4 = 16
	} type_v;

	enum class component_type {
		undefined = 0,
		act_signed_byte = 5120,
		act_unsigned_byte = 5121,
		act_signed_short = 5122,
		act_unsigned_short = 5123,
		act_unsigned_int = 5125,
		act_float = 5126
	} component_type_v;

	std::shared_ptr<ruis::render::vertex_buffer> vbo;
	std::shared_ptr<ruis::render::index_buffer> ibo;

	vertex_data_t data;

	accessor(
		utki::shared_ref<buffer_view> bv,
		uint32_t count,
		uint32_t byte_offset,
		type type_v,
		component_type component_type_v
	);
};

struct image_l {
	std::string name;
	utki::shared_ref<buffer_view> bv;
	enum class mime_type {
		undefined = 0,
		image_jpeg = 1,
		image_png = 2
	} mime_type_v;

	image_l(std::string name, utki::shared_ref<buffer_view> bv, mime_type mime_type_v) :
		name(std::move(name)),
		bv(std::move(bv)),
		mime_type_v(mime_type_v)
	{}
};

struct sampler_l {
	enum class filter {
		nearest = 9728,
		linear = 9729,
		nearest_mipmap_nearest = 9984,
		linear_mipmap_nearest = 9985,
		nearest_mipmap_linear = 9986,
		linear_mipmap_linear = 9987
	};

	enum class wrap {
		clamp_to_edge = 33071,
		mirrored_repeat = 33648,
		repeat = 10497
	};

	filter min;
	filter mag;
	wrap wrap_s;
	wrap wrap_t;

	sampler_l(filter min, filter mag, wrap wrap_s, wrap wrap_t) :
		min(min),
		mag(mag),
		wrap_s(wrap_s),
		wrap_t(wrap_t)
	{}
};

} // namespace ruis::render
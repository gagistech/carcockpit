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

#include "gltf_loader.hpp"

#include <jsondom/dom.hpp>
#include <utki/deserializer.hpp>
#include <utki/string.hpp>
#include <utki/util.hpp>

using namespace std::string_view_literals;
using namespace ruis::render;

accessor::accessor(utki::shared_ref<buffer_view> bv, uint32_t count, type type_, component_type component_type_) :
	bv(bv),
	count(count),
	type_(type_),
	component_type_(component_type_)
{}

gltf_loader::gltf_loader(ruis::render::render_factory& render_factory_, bool use_short_indices) :
	render_factory_(render_factory_),
	use_short_indices(use_short_indices)
{}

inline int gltf_loader::read_int(const jsondom::value& json, const std::string& name)
{
	return json.object().at(name).number().to_int32();
}

inline float gltf_loader::read_float(const jsondom::value& json, const std::string& name)
{
	return json.object().at(name).number().to_float();
}

inline const std::string& gltf_loader::read_string(const jsondom::value& json, const std::string& name)
{
	return json.object().at(name).string();
}

template <typename T, size_t dimension>
r4::vector<T, dimension> gltf_loader::read_vec(const jsondom::value& json, const std::string& name)
{
	r4::vector<T, dimension> value;
	int i = 0;
	for (const auto& subjson : json.object().at(name).array()) {
		value[i++] = subjson.number().to_float();
	}
	return value;
}

ruis::vec2 gltf_loader::read_vec2(const jsondom::value& json, const std::string& name)
{
	return read_vec<float, 2>(json, name);
}

ruis::vec3 gltf_loader::read_vec3(const jsondom::value& json, const std::string& name)
{
	return read_vec<float, 3>(json, name);
}

ruis::vec4 gltf_loader::read_vec4(const jsondom::value& json, const std::string& name)
{
	return read_vec<float, 4>(json, name);
}

ruis::quat gltf_loader::read_quat(const jsondom::value& json, const std::string& name)
{
	ruis::quat value;
	int i = 0;
	for (const auto& subjson : json.object().at(name).array()) {
		if (i < 3)
			value.v[i++] = subjson.number().to_float();
		else
			value.s = subjson.number().to_float();
	}
	return value;
}

utki::shared_ref<buffer_view> gltf_loader::read_buffer_view(const jsondom::value& buffer_view_json)
{
	auto new_buffer_view = utki::make_shared<buffer_view>(
		read_int(buffer_view_json, "byteLength"),
		read_int(buffer_view_json, "byteOffset"),
		static_cast<buffer_view::target>(read_int(buffer_view_json, "target"))
	);
	return new_buffer_view;
}

template <typename T>
std::shared_ptr<ruis::render::vertex_buffer> gltf_loader::create_vertex_buffer_float(utki::span<const uint8_t> buffer)
{
	utki::deserializer d(buffer);
	std::vector<T> vertex_attribute_buffer;
	vertex_attribute_buffer.reserve(buffer.size_bytes() / sizeof(T));

	for (size_t i = 0; i < buffer.size_bytes(); i += sizeof(T)) {
		T t;

		if constexpr (std::is_same_v<T, float>) {
			t = d.read_float_le();
		} else {
			for (size_t j = 0; j < sizeof(T) / sizeof(float); ++j)
				t[j] = d.read_float_le();
		}
		// we suppose this [] will also work for matrices, anyway, we don't expect matrices as vert attribs
		// also, only float vectors for now. Need a template<> deserialize function for nice notation
		vertex_attribute_buffer.push_back(std::move(t));
	}

	auto vbo = render_factory_.create_vertex_buffer(utki::make_span(vertex_attribute_buffer));
	return vbo;
}

utki::shared_ref<accessor> gltf_loader::read_accessor(const jsondom::value& accessor_json)
{
	accessor::type type_ = accessor::type::vec3;
	std::string type_s = read_string(accessor_json, "type");

	if (type_s == "SCALAR")
		type_ = accessor::type::scalar;
	else if (type_s == "VEC2")
		type_ = accessor::type::vec2;
	// else if(type_s == "VEC3") type_ = accessor::type::vec3;  // huge performance boost
	else if (type_s == "VEC4")
		type_ = accessor::type::vec4;
	else if (type_s == "MAT2")
		type_ = accessor::type::mat2;
	else if (type_s == "MAT3")
		type_ = accessor::type::mat3;
	else if (type_s == "MAT4")
		type_ = accessor::type::mat4;

	int buffer_view_index = read_int(accessor_json, "bufferView");

	auto new_accessor = utki::make_shared<accessor>(
		buffer_views[buffer_view_index],
		read_int(accessor_json, "count"),
		type_,
		static_cast<accessor::component_type>(read_int(accessor_json, "componentType"))
	);

	// TODO: take count and byteOffset into account

	const size_t offset = new_accessor.get().bv.get().byte_offset;
	const size_t length = new_accessor.get().bv.get().byte_length;
	auto buf = glb_binary_buffer.subspan(offset, length);
	// const size_t num_components = static_cast<const size_t>(new_accessor.get().type_);

	if (new_accessor.get().bv.get().target_ == buffer_view::target::array_buffer) {
		if (new_accessor.get().component_type_ == accessor::component_type::act_float) {
			if (new_accessor.get().type_ == accessor::type::scalar)
				new_accessor.get().vbo = create_vertex_buffer_float<float>(buf);
			if (new_accessor.get().type_ == accessor::type::vec2)
				new_accessor.get().vbo = create_vertex_buffer_float<ruis::vec2>(buf);
			else if (new_accessor.get().type_ == accessor::type::vec3)
				new_accessor.get().vbo = create_vertex_buffer_float<ruis::vec3>(buf);
			else if (new_accessor.get().type_ == accessor::type::vec4)
				new_accessor.get().vbo = create_vertex_buffer_float<ruis::vec4>(buf);
			else {
				throw std::logic_error("Matrix vertex attributes currently not supported");
			}

			// create_vertex_buffer_float<r4::vector<float, num_components>>(buf);
		}
	} else if (new_accessor.get().bv.get().target_ == buffer_view::target::element_Array_buffer) {
		utki::deserializer d(buf);
		if (new_accessor.get().component_type_ == accessor::component_type::act_unsigned_short) {
			std::vector<uint16_t> index_attribute_buffer;
			index_attribute_buffer.reserve(length / sizeof(uint16_t));

			for (size_t i = 0; i < length; i += sizeof(uint16_t))
				index_attribute_buffer.push_back(d.read_uint16_le());

			new_accessor.get().ibo = render_factory_.create_index_buffer(utki::make_span(index_attribute_buffer));
		} else if (new_accessor.get().component_type_ == accessor::component_type::act_unsigned_int) {
			if (use_short_indices) {
				std::vector<uint16_t> index_attribute_buffer;
				index_attribute_buffer.reserve(length / sizeof(uint32_t));

				for (size_t i = 0; i < length; i += sizeof(uint32_t))
					index_attribute_buffer.push_back(static_cast<uint16_t>(d.read_uint32_le()));

				new_accessor.get().ibo = render_factory_.create_index_buffer(utki::make_span(index_attribute_buffer));
			} else {
				std::vector<uint32_t> index_attribute_buffer;
				index_attribute_buffer.reserve(length / sizeof(uint32_t));

				for (size_t i = 0; i < length; i += sizeof(uint32_t))
					index_attribute_buffer.push_back(d.read_uint32_le());

				new_accessor.get().ibo = render_factory_.create_index_buffer(utki::make_span(index_attribute_buffer));
			}
		}
	}

	return new_accessor;
}

utki::shared_ref<mesh> gltf_loader::read_mesh(const jsondom::value& mesh_json)
{
	std::vector<utki::shared_ref<primitive>> primitives;
	const std::string name = read_string(mesh_json, "name");
	auto json_primitives_array = mesh_json.object().at("primitives").array();

	for (const auto& json_primitive : json_primitives_array) {
		auto attributes_json = json_primitive.object().at("attributes");

		int index_accessor = read_int(json_primitive, "indices");
		int position_accessor = read_int(attributes_json, "POSITION");
		int normal_accessor = read_int(attributes_json, "NORMAL");
		int texcoord_0_accessor = read_int(attributes_json, "TEXCOORD_0");
		int tangent_accessor = read_int(attributes_json, "TANGENT");

		std::cout << name << std::endl;
		std::cout << index_accessor << std::endl;
		std::cout << position_accessor << std::endl;
		std::cout << normal_accessor << std::endl;
		std::cout << texcoord_0_accessor << std::endl;
		std::cout << tangent_accessor << std::endl;
		std::cout << accessors.size() << std::endl;

		// auto vbo_bitangents = render_factory_.create_vertex_buffer(utki::make_span(bitangents));

		auto vao = render_factory_.create_vertex_array(
			{
				utki::shared_ref<ruis::render::vertex_buffer>(std::move(accessors[position_accessor].get().vbo)),
				utki::shared_ref<ruis::render::vertex_buffer>(std::move(accessors[texcoord_0_accessor].get().vbo)),
				utki::shared_ref<ruis::render::vertex_buffer>(std::move(accessors[normal_accessor].get().vbo)),
				utki::shared_ref<ruis::render::vertex_buffer>(std::move(accessors[tangent_accessor].get().vbo))
				// , vbo_bitangents
			},
			utki::shared_ref<ruis::render::index_buffer>(std::move(accessors[index_accessor].get().ibo)),
			ruis::render::vertex_array::mode::triangles
		);

		auto material_ = utki::make_shared<material>();
		primitives.push_back(utki::make_shared<primitive>(vao, material_));
	}

	auto new_mesh = utki::make_shared<mesh>(primitives, name);
	return new_mesh;
}

utki::shared_ref<node> gltf_loader::read_node(const jsondom::value& node_json)
{
	trs transformation = transformation_identity;

	std::string name = read_string(node_json, "name");
	transformation.rotation = read_quat(node_json, "rotation");
	transformation.scale = read_vec3(node_json, "scale");
	transformation.translation = read_vec3(node_json, "translation");

	int mesh_index = -1;
	try {
		mesh_index = read_int(node_json, "mesh");
	} catch (std::exception& e) {
		// no mesh: empty node, not an error
	}
	// catch(std::out_of_range)
	// catch(std::logic_error)
	// catch(std::invalid_argument)

	auto new_node =
		utki::make_shared<node>(mesh_index > 0 ? meshes[mesh_index].to_shared_ptr() : nullptr, name, transformation);
	return new_node;
}

utki::shared_ref<scene> gltf_loader::load(const papki::file& fi)
{
	auto new_scene = utki::make_shared<scene>();

	auto gltf = fi.load();

	utki::deserializer d(gltf);

	constexpr auto gltf_header_size = 4;
	if (auto header_sv = d.read_string(gltf_header_size); header_sv != "glTF"sv) {
		throw std::invalid_argument(utki::cat("read_gltf(): file header is not 'glTF': ", header_sv));
	}

	auto version = d.read_uint32_le();
	constexpr auto expected_gltf_version = 2;
	if (version != expected_gltf_version) {
		throw std::invalid_argument(utki::cat("read_gltf(): glTF file version is not as expected: ", version));
	}

	auto gltf_length = d.read_uint32_le();
	if (gltf_length != gltf.size()) {
		throw std::invalid_argument(
			utki::cat("read_gltf(): glTF file size ", gltf.size(), " does not match length ", gltf_length)
		);
	}

	auto chunk_length = d.read_uint32_le();
	if (chunk_length == 0) {
		throw std::invalid_argument("read_gltf(): chunk length = 0");
	}
	constexpr auto chunk_type_length = 4;
	if (auto chunk_type = d.read_string(chunk_type_length); chunk_type != "JSON"sv) {
		throw std::invalid_argument(
			utki::cat("read_gltf(): unexpected first chunk type: ", chunk_type, ", expected JSON")
		);
	}

	auto json_span = d.read_span(chunk_length);

	chunk_length = d.read_uint32_le();
	if (chunk_length == 0) {
		throw std::invalid_argument("read_gltf(): chunk length = 0");
	}
	if (auto chunk_type = d.read_string(chunk_type_length); chunk_type != "BIN\0"sv) { // TODO null terminator ?
		throw std::invalid_argument(
			utki::cat("read_gltf(): unexpected first chunk type: ", chunk_type, ", expected JSON")
		);
	}

	glb_binary_buffer = d.read_span(chunk_length);

	auto json = jsondom::read(json_span);
	ASSERT(json.is_object())

	for (const auto& kv : json.object()) {
		std::cout << "key = " << kv.first << ", value type = " << unsigned(kv.second.get_type()) << std::endl;
	}

	std::cout << "loading bufferViews" << std::endl;
	{
		auto it = json.object().find("bufferViews");
		if (it == json.object().end() || !it->second.is_array()) {
			throw std::invalid_argument("read_gltf(): glTF does not have any valid bufferViews");
		} else {
			for (const auto& buffer_view_json : it->second.array()) {
				buffer_views.push_back(read_buffer_view(buffer_view_json));
			}
		}
	}

	std::cout << "loading accessors" << std::endl;
	{
		auto it = json.object().find("accessors");
		if (it == json.object().end() || !it->second.is_array()) {
			throw std::invalid_argument("read_gltf(): glTF does not have any valid accessors");
		} else {
			for (const auto& accessor_json : it->second.array()) {
				accessors.push_back(read_accessor(accessor_json));
			}
		}
	}

	std::cout << "loading meshes" << std::endl;
	// load meshes
	auto meshes_it = json.object().find("meshes");
	if (meshes_it == json.object().end() || !meshes_it->second.is_array()) {
		throw std::invalid_argument("read_gltf(): glTF does not have any valid nodes");
	} else {
		// load all meshes into the scene object (actually, there should be an uber-object for mesh storage, but we
		// currenetly support only one scene)
		for (const auto& mesh_json : meshes_it->second.array()) {
			meshes.push_back(read_mesh(mesh_json));
		}
	}

	std::cout << "loading nodes" << std::endl;
	// load nodes
	auto nodes_it = json.object().find("nodes");
	if (nodes_it == json.object().end() || !nodes_it->second.is_array()) {
		throw std::invalid_argument("read_gltf(): glTF does not have any valid nodes");
	} else {
		// load all nodes into an intermediate array, form scenes from nodes later
		for (const auto& node_json : nodes_it->second.array()) {
			nodes.push_back(read_node(node_json));
		}
	}

	new_scene.get().nodes = nodes; // currently we support only one scene per gltf file

	return new_scene;
}

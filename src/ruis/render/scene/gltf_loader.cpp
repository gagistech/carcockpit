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
#include <papki/span_file.hpp>
#include <utki/deserializer.hpp>
#include <utki/string.hpp>
#include <utki/util.hpp>

using namespace std::string_view_literals;
using namespace ruis::render;

accessor::accessor(
	utki::shared_ref<buffer_view> bv,
	uint32_t count,
	uint32_t byte_offset,
	// uint32_t byte_stride,
	type type_,
	component_type component_type_
) :
	bv(bv),
	count(count),
	byte_offset(byte_offset),
	// byte_stride(byte_stride),
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

inline bool gltf_loader::read_int_checked(const jsondom::value& json, const std::string& name, int& value)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) {
		value = it->second.number().to_int32();
		return true;
	}
	return false;
}

inline bool gltf_loader::read_uint32_checked(const jsondom::value& json, const std::string& name, uint32_t& value)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) {
		value = it->second.number().to_uint32();
		return true;
	}
	return false;
}

inline float gltf_loader::read_float(const jsondom::value& json, const std::string& name)
{
	return json.object().at(name).number().to_float();
}

inline const std::string& gltf_loader::read_string(const jsondom::value& json, const std::string& name)
{
	return json.object().at(name).string();
}

inline bool gltf_loader::read_string_checked(const jsondom::value& json, const std::string& name, std::string& value)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) {
		value = it->second.string();
		return true;
	}
	return false;
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

bool gltf_loader::read_uint_array_checked(
	const jsondom::value& json,
	const std::string& name,
	std::vector<uint32_t>& array
)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) {
		array.reserve(it->second.array().size());
		for (const auto& index : it->second.array())
			array.push_back(index.number().to_uint32());

		return true;
	}
	return false;
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
	[[maybe_unused]] bool b;
	uint32_t byte_length = 0;
	uint32_t byte_offset = 0;
	uint32_t byte_stride = 0;
	uint32_t target = 0;
	b = read_uint32_checked(buffer_view_json, "byteLength", byte_length);
	b = read_uint32_checked(buffer_view_json, "byteOffset", byte_offset);
	b = read_uint32_checked(buffer_view_json, "byteStride", byte_stride);
	b = read_uint32_checked(buffer_view_json, "target", target);

	auto new_buffer_view =
		utki::make_shared<buffer_view>(byte_length, byte_offset, byte_stride, static_cast<buffer_view::target>(target));
	return new_buffer_view;
}

template <typename T>
std::shared_ptr<ruis::render::vertex_buffer> gltf_loader::create_vertex_buffer_float(
	utki::span<const uint8_t> buffer,
	uint32_t acc_count, // in elements (e.g. a whole vec3)
	// uint32_t acc_offset, // in bytes
	uint32_t acc_stride // in bytes
)
{
	utki::deserializer d(buffer);
	std::vector<T> vertex_attribute_buffer;
	vertex_attribute_buffer.reserve(acc_count);

	// // skip offset:
	// for (uint32_t skip = 0; skip < acc_offset; skip += sizeof(float))
	// 	d.read_float_le();

	for (uint32_t i = 0; i < acc_count; ++i) {
		T t;

		if constexpr (std::is_same_v<T, float>) {
			t = d.read_float_le();
		} else {
			for (uint32_t j = 0; j < t.size(); ++j)
				t[j] = d.read_float_le();
		}
		// we suppose this [] will also work for matrices, anyway, we don't expect matrices as vert attribs
		// also, only float vectors for now. Need a template<> deserialize function for nice notation
		vertex_attribute_buffer.push_back(std::move(t));

		// skip stride:
		// for (uint32_t skip = 0; skip < acc_stride; skip += sizeof(float))
		//	d.read_float_le();
		[[maybe_unused]] auto skipped_span = d.read_span(acc_stride);
	}

	auto vbo = render_factory_.create_vertex_buffer(utki::make_span(vertex_attribute_buffer));
	return vbo;
}

utki::shared_ref<accessor> gltf_loader::read_accessor(const jsondom::value& accessor_json)
{
	[[maybe_unused]] bool ok = true;
	accessor::type type_ = accessor::type::vec3;
	std::string type_s;
	ok = read_string_checked(accessor_json, "type", type_s);

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

	uint32_t buffer_view_index = 0;
	ok = read_uint32_checked(accessor_json, "bufferView", buffer_view_index);

	uint32_t acc_count = 0;
	uint32_t acc_offset = 0;
	uint32_t component_type = 0;
	// uint32_t acc_stride = 0;
	ok = read_uint32_checked(accessor_json, "count", acc_count);
	ok = read_uint32_checked(accessor_json, "byteOffset", acc_offset);
	ok = read_uint32_checked(accessor_json, "componentType", component_type);
	// read_uint32_checked(accessor_json, "byteStride", acc_stride);

	auto new_accessor = utki::make_shared<accessor>(
		buffer_views[buffer_view_index],
		acc_count,
		acc_offset,
		// acc_stride,
		type_,
		static_cast<accessor::component_type>(component_type)
	);

	// TODO: take count and byteOffset into account

	const uint32_t bv_offset = new_accessor.get().bv.get().byte_offset;
	const uint32_t bv_length = new_accessor.get().bv.get().byte_length;
	const uint32_t bv_stride = new_accessor.get().bv.get().byte_stride;

	auto buf = glb_binary_buffer.subspan(bv_offset + acc_offset, bv_length);

	// const size_t num_components = static_cast<const size_t>(new_accessor.get().type_);

	// if (new_accessor.get().bv.get().target_ == buffer_view::target::array_buffer) {

	if (new_accessor.get().component_type_ == accessor::component_type::act_float) {
		if (new_accessor.get().type_ == accessor::type::scalar)
			new_accessor.get().vbo = create_vertex_buffer_float<float>(buf, acc_count, bv_stride);
		else if (new_accessor.get().type_ == accessor::type::vec2)
			new_accessor.get().vbo = create_vertex_buffer_float<ruis::vec2>(buf, acc_count, bv_stride);
		else if (new_accessor.get().type_ == accessor::type::vec3)
			new_accessor.get().vbo = create_vertex_buffer_float<ruis::vec3>(buf, acc_count, bv_stride);
		else if (new_accessor.get().type_ == accessor::type::vec4)
			new_accessor.get().vbo = create_vertex_buffer_float<ruis::vec4>(buf, acc_count, bv_stride);
		else {
			throw std::logic_error("Matrix vertex attributes currently not supported");
		}

		// create_vertex_buffer_float<r4::vector<float, num_components>>(buf);
	} else if ((new_accessor.get().component_type_ == accessor::component_type::act_unsigned_short ||
				new_accessor.get().component_type_ == accessor::component_type::act_unsigned_int) &&
			   // new_accessor.get().bv.get().target_ == buffer_view::target::element_Array_buffer &&
			   //  there are gltf exporters which don't mark target at all. So we must detect index arrays another way
			   new_accessor.get().type_ == accessor::type::scalar)
	{
		utki::deserializer d(buf);
		// TODO: think about what to do if index count > 65535
		if (new_accessor.get().component_type_ == accessor::component_type::act_unsigned_short) {
			std::vector<uint16_t> index_attribute_buffer;
			index_attribute_buffer.reserve(acc_count);

			for (size_t i = 0; i < acc_count; ++i)
				index_attribute_buffer.push_back(d.read_uint16_le());

			new_accessor.get().ibo = render_factory_.create_index_buffer(utki::make_span(index_attribute_buffer));

		} else if (new_accessor.get().component_type_ == accessor::component_type::act_unsigned_int) {
			if (use_short_indices) { // opengles 2.0 does not support 32-bit indices by default
				std::vector<uint16_t> index_attribute_buffer;
				index_attribute_buffer.reserve(acc_count);

				for (size_t i = 0; i < acc_count; ++i)
					index_attribute_buffer.push_back(static_cast<uint16_t>(d.read_uint32_le()));

				new_accessor.get().ibo = render_factory_.create_index_buffer(utki::make_span(index_attribute_buffer));

			} else {
				std::vector<uint32_t> index_attribute_buffer;
				index_attribute_buffer.reserve(acc_count);

				for (size_t i = 0; i < acc_count; ++i)
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

		int index_accessor = -1;
		int position_accessor = -1;
		int normal_accessor = -1;
		int texcoord_0_accessor = -1;
		int tangent_accessor = -1;

		[[maybe_unused]] bool ok = true;

		ok = read_int_checked(json_primitive, "indices", index_accessor);
		ok = read_int_checked(attributes_json, "POSITION", position_accessor);
		ok = read_int_checked(attributes_json, "NORMAL", normal_accessor);
		ok = read_int_checked(attributes_json, "TEXCOORD_0", texcoord_0_accessor);
		ok = read_int_checked(attributes_json, "TANGENT", tangent_accessor);

		std::cout << name << std::endl;
		std::cout << index_accessor << std::endl;
		std::cout << position_accessor << std::endl;
		std::cout << normal_accessor << std::endl;
		std::cout << texcoord_0_accessor << std::endl;
		std::cout << tangent_accessor << std::endl;
		std::cout << accessors.size() << std::endl;

		// auto vbo_bitangents = render_factory_.create_vertex_buffer(utki::make_span(bitangents));
		//utki::shared_ref<vertex_array> vao; 
		if(tangent_accessor > 0)
		{	
			auto vao = render_factory_.create_vertex_array(
				{
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[position_accessor].get().vbo)
					),
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[texcoord_0_accessor].get().vbo)
					),
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[normal_accessor].get().vbo)
					),
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[tangent_accessor].get().vbo)
					)
					// , vbo_bitangents
				},
				utki::shared_ref<ruis::render::index_buffer>(
					std::shared_ptr<ruis::render::index_buffer>(accessors[index_accessor].get().ibo)
				),
				ruis::render::vertex_array::mode::triangles
			);
			auto material_ = utki::make_shared<material>();
			primitives.push_back(utki::make_shared<primitive>(vao, material_));
		}
		else
		{
			auto vao = render_factory_.create_vertex_array(
				{
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[position_accessor].get().vbo)
					),
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[texcoord_0_accessor].get().vbo)
					),
					utki::shared_ref<ruis::render::vertex_buffer>(
						std::shared_ptr<ruis::render::vertex_buffer>(accessors[normal_accessor].get().vbo)
					)
				},
				utki::shared_ref<ruis::render::index_buffer>(
					std::shared_ptr<ruis::render::index_buffer>(accessors[index_accessor].get().ibo)
				),
				ruis::render::vertex_array::mode::triangles
			);	
			auto material_ = utki::make_shared<material>();
			primitives.push_back(utki::make_shared<primitive>(vao, material_));
		}	
	}

	auto new_mesh = utki::make_shared<mesh>(primitives, name);
	return new_mesh;
}

utki::shared_ref<node> gltf_loader::read_node(const jsondom::value& node_json)
{
	trs transformation = transformation_identity;

	std::string name = read_string(node_json, "name");

	auto it = node_json.object().find("rotation");
	if (it != node_json.object().end())
		transformation.rotation = read_quat(node_json, "rotation");

	it = node_json.object().find("scale");
	if (it != node_json.object().end())
		transformation.scale = read_vec3(node_json, "scale");

	it = node_json.object().find("translation");
	if (it != node_json.object().end())
		transformation.translation = read_vec3(node_json, "translation");

	int mesh_index = -1;
	bool ok = read_int_checked(node_json, "mesh", mesh_index);

	child_indices.push_back(std::vector<uint32_t>());
	read_uint_array_checked(node_json, "children", child_indices.back());

	auto new_node = utki::make_shared<node>(ok ? meshes[mesh_index].to_shared_ptr() : nullptr, name, transformation);
	return new_node;
}

utki::shared_ref<scene> gltf_loader::read_scene(const jsondom::value& scene_json)
{
	auto new_scene = utki::make_shared<scene>();
	std::vector<uint32_t> node_indices;
	new_scene.get().name = read_string(scene_json, "name");
	read_uint_array_checked(scene_json, "nodes", node_indices);

	for (uint32_t ni : node_indices) {
		new_scene.get().nodes.push_back(this->nodes[ni]);
	}

	return new_scene;
}

utki::shared_ref<scene> gltf_loader::load(const papki::file& fi)
{
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

	// hierarchize nodes
	ASSERT(nodes.size() == child_indices.size())
	for (uint32_t i = 0; i < nodes.size(); ++i) {
		for (uint32_t ci : child_indices[i]) {
			nodes[i].get().children.push_back(nodes[ci]);
		}
	}

	// read scenes
	std::cout << "loading scenes" << std::endl;
	auto scenes_it = json.object().find("scenes");
	if (scenes_it == json.object().end() || !scenes_it->second.is_array()) {
		throw std::invalid_argument("read_gltf(): glTF does not have any valid scenes");
	} else {
		// load all nodes into an intermediate array, form scenes from nodes later
		for (const auto& scene_json : scenes_it->second.array()) {
			scenes.push_back(read_scene(scene_json));
		}
	}

	// const papki::span_file fi(data);

	// create scene
	auto active_scene = utki::make_shared<scene>();
	int active_scene_index = -1;
	bool ok = read_int_checked(json, "scene", active_scene_index);
	if (!ok) {
		// this .gltf file is a library
	} else {
		active_scene = scenes[active_scene_index];
	}

	// create cameras (currently generates one default camera)
	std::cout << "create camera" << std::endl;

	auto cam = utki::make_shared<camera>();
	cam.get().pos = {0, 3.5, -8};
	cam.get().target = {0, 1, 0};
	cam.get().up = {0, 1, 0};
	cam.get().near = 0.1;
	cam.get().far = 100;
	cam.get().fovy = 3.1415926 / 3;

	active_scene.get().cameras.push_back(cam);
	active_scene.get().active_camera = cam.to_shared_ptr();

	std::cout << "gltf load finished" << std::endl;
	return active_scene;
}

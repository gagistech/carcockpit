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

#include "scene.hpp"

#include <jsondom/dom.hpp>
#include <utki/string.hpp>
#include <utki/util.hpp>

using namespace std::string_view_literals;
using namespace ruis::render;

scene::scene() //utki::shared_ref<ruis::context> c)
: //context_(c)
{
}

int read_int(const jsondom::value& json, const std::string& name)
{
	int value;
	auto it = json.object().find(name);
	if (it != json.object().end() && it->second.is_number()) {
		value = it->second.number().to_int32();
	} else {
		throw std::invalid_argument("read_gltf(): glTF expected number, found different type");
	}
	return value;
}

float read_float(const jsondom::value& json, const std::string& name)
{
	float value;
	auto it = json.object().find(name);
	if (it != json.object().end() && it->second.is_number()) {
		value = it->second.number().to_float();
	} else {
		throw std::invalid_argument("read_gltf(): glTF expected number, found different type");
	}
	return value;
}

ruis::vec3 read_vec3(const jsondom::value& json, const std::string& name)
{
	ruis::vec3 value;
	auto it = json.object().find(name);
	if (it == json.object().end() || !it->second.is_array()) {
		throw std::invalid_argument("read_gltf(): glTF read error, expected array");
	} else {
		int = 0;
		for (const auto& vec_json : it->second.array()) 
		{
			//value[i++] = read_float(vec_json);
		}
	}
	return value;
}

utki::shared_ref<buffer_view> read_buffer_view(const jsondom::value& buffer_view_json)
{
	// buffer_view_json.object().
	auto new_buffer_view = utki::make_shared<buffer_view>(0, 0, 0);
	return new_buffer_view;
}

utki::shared_ref<accessor> read_accessor(
	const jsondom::value& accessor_json,
	const std::vector<utki::shared_ref<buffer_view>>& buffer_views
)
{
	auto new_accessor = utki::make_shared<accessor>();
	return new_accessor;
}

utki::shared_ref<mesh> read_mesh(
	const scene& s, 
	const jsondom::value& mesh_json,
	const utki::span<uint8_t>& glb_binary_buffer,
	const std::vector<utki::shared_ref<accessor>>& accessors,
	ruis::render::render_factory& rf
)
{
	// ruis::render::vertex_array::buffers_type buffers;
	// buffers.push_back(utki::make_shared<vertex_buffer>(glb_binary_buffer));

	//auto buf_short = reinterpret_cast<utki::span<uint16_t>
	//const utki::span<uint16_t> short_buf(glb_binary_buffer)
	const utki::span<uint16_t> short_buf = 
		utki::make_span<uint16_t>( reinterpret_cast<uint16_t*>(glb_binary_buffer.data()), glb_binary_buffer.size_bytes() / 2 );

	auto indices = rf.create_index_buffer(short_buf);

	// auto vao = utki::make_shared<ruis::render::vertex_array>(
	// 	std::move(buffers),
	// 	indices,
	// 	ruis::render::vertex_array::mode::triangles
	// );

	std::vector<ruis::vec4> positions;
	std::vector<ruis::vec2> texture_coordinates_0;
	std::vector<ruis::vec3> normals;
	std::vector<ruis::vec3> tangents;
	std::vector<ruis::vec3> bitangents;

	auto vbo_positions = rf.create_vertex_buffer(utki::make_span(positions));
	auto vbo_texture_coordinates_0 = rf.create_vertex_buffer(utki::make_span(texture_coordinates_0));
	auto vbo_normals = rf.create_vertex_buffer(utki::make_span(normals));
	auto vbo_tangents = rf.create_vertex_buffer(utki::make_span(tangents));
	auto vbo_bitangents = rf.create_vertex_buffer(utki::make_span(bitangents));

	auto vao =  rf.create_vertex_array(
				{vbo_positions, vbo_texture_coordinates_0, vbo_normals, vbo_tangents, vbo_bitangents},
				indices,
				ruis::render::vertex_array::mode::triangles
			);

	auto new_mesh = utki::make_shared<mesh>(vao);
	return new_mesh;
}

utki::shared_ref<node> read_node(const jsondom::value& node_json, const std::vector<utki::shared_ref<mesh>>& meshes)
{
	trs transformation = transformation_identity;
	auto new_node = utki::make_shared<node>(meshes[0], "tmp_name", transformation);
	return new_node;
}

utki::shared_ref<scene> ruis::render::read_gltf(const papki::file& fi, ruis::render::render_factory& rf)
{
	auto new_scene = utki::make_shared<scene>();
	std::vector<utki::shared_ref<node>>
		nodes; // all nodes that are listed in gltf file overall, without binding to any scene(s)
	std::vector<utki::shared_ref<mesh>> meshes; // order is important on loading stage
	std::vector<utki::shared_ref<accessor>> accessors; // order is important on loading stage
	std::vector<utki::shared_ref<buffer_view>> buffer_views; // order is important on loading stage

	auto gltf = fi.load();
	auto p = utki::make_span(gltf);

	constexpr auto gltf_header_size = 4;

	if (p.size() < gltf_header_size) {
		throw std::invalid_argument("read_gltf(): file too short for glTF header");
	}

	ASSERT(p.size() >= gltf_header_size)
	if (auto header_sv = utki::make_string_view(p.subspan(0, gltf_header_size)); header_sv != "glTF"sv) {
		throw std::invalid_argument(utki::cat("read_gltf(): file header is not 'glTF': ", header_sv));
	}

	p = p.subspan(gltf_header_size);

	constexpr auto num_expected_fields_after_header = 4;
	if (auto es = num_expected_fields_after_header * sizeof(uint32_t); p.size() < es) {
		throw std::invalid_argument(utki::cat("read_gltf(): glTF file is too short: ", es));
	}

	auto version = utki::deserialize32le(p.data());
	p = p.subspan(sizeof(uint32_t));

	constexpr auto expected_gltf_version = 2;
	if (version != expected_gltf_version) {
		throw std::invalid_argument(utki::cat("read_gltf(): glTF file version is not as expected: ", version));
	}

	auto gltf_length = utki::deserialize32le(p.data());
	p = p.subspan(sizeof(uint32_t));

	if (gltf_length != gltf.size()) {
		throw std::invalid_argument(
			utki::cat("read_gltf(): glTF file size ", gltf.size(), " does not match length ", gltf_length)
		);
	}

	auto chunk_length = utki::deserialize32le(p.data());
	p = p.subspan(sizeof(uint32_t));

	if (chunk_length == 0) {
		throw std::invalid_argument("read_gltf(): chunk length = 0");
	}

	constexpr auto chunk_type_length = 4;
	auto chunk_type = utki::make_string_view(p.subspan(0, chunk_type_length));
	p = p.subspan(chunk_type_length);

	if (chunk_type != "JSON"sv) {
		throw std::invalid_argument(
			utki::cat("read_gltf(): unexpected first chunk type: ", chunk_type, ", expected JSON")
		);
	}

	if (p.size() < chunk_length) {
		throw std::invalid_argument("read_gltf(): glTF file too short");
	}

	auto json_span = p.subspan(0, chunk_length);
	auto json = jsondom::read(json_span);
	p = p.subspan(chunk_length); // proceed to binary data, json dom hierarchy is formed
	ASSERT(json.is_object())

	for (const auto& kv : json.object()) {
		std::cout << "key = " << kv.first << ", value type = " << unsigned(kv.second.get_type()) << std::endl;
	}


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

	{
		auto it = json.object().find("accessors");
		if (it == json.object().end() || !it->second.is_array()) {
			throw std::invalid_argument("read_gltf(): glTF does not have any valid accessors");
		} else {
			for (const auto& accessor_json : it->second.array()) {
				accessors.push_back(read_accessor(accessor_json, buffer_views));
			}
		}
	}

	// load meshes
	auto meshes_it = json.object().find("meshes");
	if (meshes_it == json.object().end() || !meshes_it->second.is_array()) {
		throw std::invalid_argument("read_gltf(): glTF does not have any valid nodes");
	} else {
		// load all meshes into the scene object (actually, there should be an uber-object for mesh storage, but we
		// currenetly support only one scene)
		for (const auto& mesh_json : meshes_it->second.array()) {
			meshes.push_back(read_mesh(new_scene.get(), mesh_json, p, accessors, rf));
		}
	}

	// load nodes
	auto nodes_it = json.object().find("nodes");
	if (nodes_it == json.object().end() || !nodes_it->second.is_array()) {
		throw std::invalid_argument("read_gltf(): glTF does not have any valid nodes");
	} else {
		// load all nodes into an intermediate array, form scenes from nodes later
		for (const auto& node_json : nodes_it->second.array()) {
			nodes.push_back(read_node(node_json, meshes));
		}
	}

	new_scene.get().nodes = nodes; // currently we support only one scene per gltf file

	return new_scene;
}

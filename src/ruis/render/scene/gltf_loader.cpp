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

#include "gltf_loader.hxx"

#include <jsondom/dom.hpp>
#include <papki/span_file.hpp>
#include <rasterimage/image_variant.hpp>
#include <utki/deserializer.hpp>
#include <utki/string.hpp>
#include <utki/util.hpp>

using namespace std::string_view_literals;
using namespace ruis::render;

accessor::accessor(
	utki::shared_ref<buffer_view> bv,
	uint32_t count,
	uint32_t byte_offset,
	type type_v,
	component_type component_type_v
) :
	bv(std::move(bv)),
	count(count),
	byte_offset(byte_offset),
	type_v(type_v),
	component_type_v(component_type_v)
{}

gltf_loader::gltf_loader(ruis::render::factory& factory_v) :
	factory_v(factory_v)
{}

namespace {

int read_int(const jsondom::value& json, const std::string& name, const int default_value = -1)
{
	auto it = json.object().find(name);
	if (it != json.object().end())
		return it->second.number().to_int32();

	return default_value;
}

uint32_t read_uint(const jsondom::value& json, const std::string& name, const uint32_t default_value = 0)
{
	auto it = json.object().find(name);
	if (it != json.object().end())
		return it->second.number().to_uint32();

	return default_value;
}

std::string read_string(const jsondom::value& json, const std::string& name, const std::string default_value = "")
{
	auto it = json.object().find(name);
	if (it != json.object().end())
		return it->second.string();

	return default_value;
}

std::vector<uint32_t> read_uint_array(const jsondom::value& json, const std::string& name, uint32_t dafault_value = 0)
{
	std::vector<uint32_t> arr;
	auto it = json.object().find(name);
	if (it != json.object().end() && json.object().at(name).is_array()) {
		arr.reserve(it->second.array().size());

		for (const auto& index : it->second.array())
			arr.push_back(index.number().to_uint32());
	}
	return arr;
}

template <typename tp_type, size_t dimension>
r4::vector<tp_type, dimension> read_vec(
	const jsondom::value& json,
	const std::string& name,
	const r4::vector<tp_type, dimension> default_value
)
{
	auto it = json.object().find(name);

	if (it == json.object().end() || !json.object().at(name).is_array())
		return default_value;

	r4::vector<tp_type, dimension> value;
	int i = 0;
	for (const auto& subjson : json.object().at(name).array()) {
		value[i++] = subjson.number().to_float();
	}
	return value;
}

ruis::quat read_quat(const jsondom::value& json, const std::string& name, const ruis::quat default_value)
{
	ruis::vec4 default_vec = default_value.to_vector4();
	auto vec = read_vec(json, name, default_vec);
	return ruis::quat{vec};
}

} // namespace

utki::shared_ref<buffer_view> gltf_loader::read_buffer_view(const jsondom::value& buffer_view_json)
{
	const uint32_t byte_length = read_uint(buffer_view_json, "byteLength");
	const uint32_t byte_offset = read_uint(buffer_view_json, "byteOffset");
	const uint32_t byte_stride = read_uint(buffer_view_json, "byteStride");
	const uint32_t target = read_uint(buffer_view_json, "target");

	auto new_buffer_view =
		utki::make_shared<buffer_view>(byte_length, byte_offset, byte_stride, static_cast<buffer_view::target>(target));
	return new_buffer_view;
}

template <typename tp_type>
void gltf_loader::create_vertex_buffer_float(
	utki::shared_ref<ruis::render::accessor> new_accessor,
	utki::span<const uint8_t> buffer,
	uint32_t acc_count, // in elements (e.g. a whole vec3)
	uint32_t acc_stride // in bytes
)
{
	utki::deserializer d(buffer);
	std::vector<tp_type> vertex_attribute_buffer;
	vertex_attribute_buffer.reserve(acc_count);

	int n_skip_bytes = int(acc_stride) - int(sizeof(tp_type));
	if (n_skip_bytes < 0)
		n_skip_bytes = 0;

	for (uint32_t i = 0; i < acc_count; ++i) {
		tp_type t;

		if constexpr (std::is_same_v<tp_type, float>) {
			t = d.read_float_le();
		} else {
			for (uint32_t j = 0; j < t.size(); ++j)
				t[j] = d.read_float_le();
		}
		// also, only float vectors for now. Need a template<> deserialize function for nice notation
		vertex_attribute_buffer.push_back(std::move(t));
		d.skip(n_skip_bytes);
	}

	new_accessor.get().vbo = factory_v.create_vertex_buffer(utki::make_span(vertex_attribute_buffer));
	new_accessor.get().data = std::move(vertex_attribute_buffer);
}

utki::shared_ref<accessor> gltf_loader::read_accessor(const jsondom::value& accessor_json)
{
	accessor::type type_v = accessor::type::vec3;
	const std::string type_s = read_string(accessor_json, "type");

	if (type_s == "SCALAR")
		type_v = accessor::type::scalar;
	else if (type_s == "VEC2")
		type_v = accessor::type::vec2;
	else if (type_s == "VEC3")
		type_v = accessor::type::vec3;
	else if (type_s == "VEC4")
		type_v = accessor::type::vec4;
	else if (type_s == "MAT2")
		type_v = accessor::type::mat2;
	else if (type_s == "MAT3")
		type_v = accessor::type::mat3;
	else if (type_s == "MAT4")
		type_v = accessor::type::mat4;

	const uint32_t buffer_view_index = read_uint(accessor_json, "bufferView");
	const uint32_t acc_count = read_uint(accessor_json, "count");
	const uint32_t acc_offset = read_uint(accessor_json, "byteOffset");
	const uint32_t component_type = read_uint(accessor_json, "componentType");

	auto new_accessor = utki::make_shared<accessor>(
		buffer_views[buffer_view_index],
		acc_count,
		acc_offset,
		type_v,
		static_cast<accessor::component_type>(component_type)
	);

	const uint32_t bv_offset = new_accessor.get().bv.get().byte_offset;
	const uint32_t bv_length = new_accessor.get().bv.get().byte_length;
	const uint32_t bv_stride = new_accessor.get().bv.get().byte_stride;

	auto buf = glb_binary_buffer.subspan(bv_offset + acc_offset, bv_length);

	if (new_accessor.get().component_type_v == accessor::component_type::act_float) {
		if (new_accessor.get().type_v == accessor::type::scalar)
			create_vertex_buffer_float<float>(new_accessor, buf, acc_count, bv_stride);
		else if (new_accessor.get().type_v == accessor::type::vec2)
			create_vertex_buffer_float<ruis::vec2>(new_accessor, buf, acc_count, bv_stride);
		else if (new_accessor.get().type_v == accessor::type::vec3)
			create_vertex_buffer_float<ruis::vec3>(new_accessor, buf, acc_count, bv_stride);
		else if (new_accessor.get().type_v == accessor::type::vec4)
			create_vertex_buffer_float<ruis::vec4>(new_accessor, buf, acc_count, bv_stride);
		else {
			throw std::logic_error("Matrix vertex attributes currently not supported");
		}

	} else if ((new_accessor.get().component_type_v == accessor::component_type::act_unsigned_short ||
				new_accessor.get().component_type_v == accessor::component_type::act_unsigned_int) &&
			   new_accessor.get().type_v == accessor::type::scalar)
	{
		utki::deserializer d(buf);
		if (new_accessor.get().component_type_v == accessor::component_type::act_unsigned_short) {
			std::vector<uint16_t> index_attribute_buffer;
			index_attribute_buffer.reserve(acc_count);

			for (size_t i = 0; i < acc_count; ++i)
				index_attribute_buffer.push_back(d.read_uint16_le());

			new_accessor.get().data = index_attribute_buffer;
			new_accessor.get().ibo = factory_v.create_index_buffer(utki::make_span(index_attribute_buffer));

		} else if (new_accessor.get().component_type_v == accessor::component_type::act_unsigned_int) {
			std::vector<uint32_t> index_attribute_buffer;
			index_attribute_buffer.reserve(acc_count);

			for (size_t i = 0; i < acc_count; ++i)
				index_attribute_buffer.push_back(d.read_uint32_le());

			new_accessor.get().data = index_attribute_buffer;
			new_accessor.get().ibo = factory_v.create_index_buffer(utki::make_span(index_attribute_buffer));
		}
		// TODO: in case GLTF says that index type is 32 bit, but still provides less than 65536 vertices, then there is
		// no reason to use 32 bit index, we can convert it to 16 bit index
	}

	return new_accessor;
}

utki::shared_ref<mesh> gltf_loader::read_mesh(const jsondom::value& mesh_json)
{
	std::vector<utki::shared_ref<primitive>> primitives;
	std::string name = read_string(mesh_json, "name");
	auto json_primitives_array = mesh_json.object().at("primitives").array();

	for (const auto& json_primitive : json_primitives_array) {
		auto attributes_json = json_primitive.object().at("attributes");

		int index_accessor = read_int(json_primitive, "indices");
		int position_accessor = read_int(attributes_json, "POSITION");
		int normal_accessor = read_int(attributes_json, "NORMAL");
		int texcoord_0_accessor = read_int(attributes_json, "TEXCOORD_0");
		[[maybe_unused]] int tangent_accessor = read_int(attributes_json, "TANGENT");

		// TOOD: Currently we do not use tangents provided from gltf file, we calculate them instead. But if they are
		// provided in file, we should use them

		int material_index = read_int(json_primitive, "material");

		if (index_accessor < 0)
			continue;

		if (position_accessor < 0)
			continue;

		if (normal_accessor < 0)
			continue;

		if (texcoord_0_accessor < 0)
			continue;

		if (accessors[index_accessor].get().component_type_v == accessor::component_type::act_unsigned_int) {
			auto vao = create_vao_with_tangent_space<uint32_t>(
				accessors[index_accessor],
				accessors[position_accessor],
				accessors[texcoord_0_accessor],
				accessors[normal_accessor]
			);

			auto material_v = material_index >= 0 ? materials[material_index] : utki::make_shared<material>();
			primitives.push_back(utki::make_shared<primitive>(vao, material_v));
		} else if (accessors[index_accessor].get().component_type_v == accessor::component_type::act_unsigned_short) {
			auto vao = create_vao_with_tangent_space<uint16_t>(
				accessors[index_accessor],
				accessors[position_accessor],
				accessors[texcoord_0_accessor],
				accessors[normal_accessor]
			);

			auto material_v = material_index >= 0 ? materials[material_index] : utki::make_shared<material>();
			primitives.push_back(utki::make_shared<primitive>(vao, material_v));
		} else {
			throw std::invalid_argument("gltf: indices data type not supported (only uint32 and uint16 are supported)");
			// TODO: branch all possible combinations if input data
		}
	}

	auto new_mesh = utki::make_shared<mesh>(primitives, name);
	return new_mesh;
}

utki::shared_ref<node> gltf_loader::read_node(const jsondom::value& node_json)
{
	constexpr ruis::vec3 default_scale{1, 1, 1};
	constexpr ruis::vec3 default_translation{0, 0, 0};
	constexpr ruis::quat default_rotation{1, 0, 0, 0};

	trs transformation = transformation_identity;

	std::string name = read_string(node_json, "name");

	auto it = node_json.object().find("rotation");
	if (it != node_json.object().end())
		transformation.rotation = read_quat(node_json, "rotation", default_rotation);

	it = node_json.object().find("scale");
	if (it != node_json.object().end())
		transformation.scale = read_vec(node_json, "scale", default_scale);

	it = node_json.object().find("translation");
	if (it != node_json.object().end())
		transformation.translation = read_vec(node_json, "translation", default_translation);

	int mesh_index = read_int(node_json, "mesh");
	child_indices.push_back(read_uint_array(node_json, "children"));

	auto new_node =
		utki::make_shared<node>(mesh_index >= 0 ? meshes[mesh_index].to_shared_ptr() : nullptr, name, transformation);
	return new_node;
}

utki::shared_ref<scene> gltf_loader::read_scene(const jsondom::value& scene_json)
{
	auto new_scene = utki::make_shared<scene>();
	std::vector<uint32_t> node_indices = read_uint_array(scene_json, "nodes");
	new_scene.get().name = read_string(scene_json, "name");

	for (uint32_t ni : node_indices) {
		new_scene.get().nodes.push_back(this->nodes[ni]);
	}

	constexpr ruis::vec4 default_light_position{4, 4, 4, 1};
	constexpr ruis::vec3 default_light_intensity{4, 4, 4};

	if (new_scene.get().lights.size() == 0)
		new_scene.get().lights.push_back(
			utki::make_shared<ruis::render::light>(default_light_position, default_light_intensity)
		);

	return new_scene;
}

utki::shared_ref<image_l> gltf_loader::read_image(const jsondom::value& image_json)
{
	uint32_t buffer_view_index = read_uint(image_json, "bufferView");
	std::string name = read_string(image_json, "name");
	std::string mime_type_string = read_string(image_json, "mimeType");

	image_l::mime_type mt = mime_type_string == "image/jpeg" ? image_l::mime_type::image_jpeg
		: mime_type_string == "image/png"                    ? image_l::mime_type::image_png
															 : image_l::mime_type::undefined;

	auto new_image = utki::make_shared<image_l>(name, buffer_views[buffer_view_index], mt);
	return new_image;
}

utki::shared_ref<sampler_l> gltf_loader::read_sampler(const jsondom::value& sampler_json)
{
	auto new_sampler = utki::make_shared<sampler_l>(
		static_cast<sampler_l::filter>(read_uint(sampler_json, "minFilter")),
		static_cast<sampler_l::filter>(read_uint(sampler_json, "magFilter")),
		static_cast<sampler_l::wrap>(read_uint(sampler_json, "wrapS")),
		static_cast<sampler_l::wrap>(read_uint(sampler_json, "wrapT"))
	);
	return new_sampler;
}

utki::shared_ref<ruis::render::texture_2d> gltf_loader::read_texture(const jsondom::value& texture_json)
{
	uint32_t image_index = read_uint(texture_json, "source");
	uint32_t sampler_index = read_uint(texture_json, "sampler");

	auto image = images[image_index];
	auto sampler = samplers[sampler_index];

	auto image_span = glb_binary_buffer.subspan(image.get().bv.get().byte_offset, image.get().bv.get().byte_length);
	const papki::span_file fi(image_span);

	rasterimage::image_variant imvar;
	if (image.get().mime_type_v == image_l::mime_type::image_png) {
		imvar = rasterimage::read_png(fi);
	} else if (image.get().mime_type_v == image_l::mime_type::image_jpeg) {
		imvar = rasterimage::read_jpeg(fi);
	} else {
		throw std::invalid_argument("gltf: unknown texture image format");
	}

	ruis::render::factory::texture_2d_parameters tex_params; // TODO: fill texparams properly based on gltf file
	tex_params.mag_filter = ruis::render::texture_2d::filter::linear;
	tex_params.min_filter = ruis::render::texture_2d::filter::linear;
	tex_params.mipmap = texture_2d::mipmap::linear;

	return factory_v.create_texture_2d(std::move(imvar), tex_params);
}

utki::shared_ref<material> gltf_loader::read_material(const jsondom::value& material_json)
{
	auto mat = utki::make_shared<material>();

	mat.get().name = read_string(material_json, "name");
	int diffuse_index = -1;
	int normal_index = -1;
	int arm_index = -1; // ambient metallic roughness

	auto it = material_json.object().find("normalTexture");
	if (it != material_json.object().end() && it->second.is_object()) {
		normal_index = read_int(it->second, "index");
	}

	auto it_pbr = material_json.object().find("pbrMetallicRoughness");
	if (it_pbr != material_json.object().end() && it_pbr->second.is_object()) {
		auto it = it_pbr->second.object().find("baseColorTexture");
		if (it != it_pbr->second.object().end() && it->second.is_object()) {
			diffuse_index = read_int(it->second, "index");
		}
		it = it_pbr->second.object().find("metallicRoughnessTexture");
		if (it != it_pbr->second.object().end() && it->second.is_object()) {
			arm_index = read_int(it->second, "index");
		}
	}

	if (diffuse_index >= 0)
		mat.get().tex_diffuse = textures[diffuse_index];
	if (normal_index >= 0)
		mat.get().tex_normal = textures[normal_index];
	if (arm_index >= 0)
		mat.get().tex_arm = textures[arm_index];

	return mat;
}

template <typename tp_type>
std::vector<utki::shared_ref<tp_type>> gltf_loader::read_root_array(
	std::function<tp_type(const jsondom::value& j)> read_func,
	const jsondom::value& root_json,
	const std::string& name
)
{
	std::vector<utki::shared_ref<tp_type>> all;
	auto it = root_json.object().find(name);
	if (it != root_json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			all.push_back(read_func(sub_json));
		}
	}
	return all;
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
	if (auto chunk_type = d.read_string(chunk_type_length); chunk_type != "BIN\0"sv) {
		throw std::invalid_argument(
			utki::cat("read_gltf(): unexpected first chunk type: ", chunk_type, ", expected JSON")
		);
	}

	glb_binary_buffer = d.read_span(chunk_length);

	auto json = jsondom::read(json_span);
	ASSERT(json.is_object())

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

	std::map<std::string, jsondom::value, std::less<>>::iterator it;

	it = json.object().find("images");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			images.push_back(read_image(sub_json));
		}
	}

	it = json.object().find("samplers");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			samplers.push_back(read_sampler(sub_json));
		}
	}

	it = json.object().find("textures");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			textures.push_back(read_texture(sub_json));
		}
	}
	it = json.object().find("materials");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			materials.push_back(read_material(sub_json));
		}
	}

	it = json.object().find("accessors");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			accessors.push_back(read_accessor(sub_json));
		}
	}

	it = json.object().find("meshes");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			meshes.push_back(read_mesh(sub_json));
		}
	}

	it = json.object().find("nodes");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			nodes.push_back(read_node(sub_json));
		}
	}

	// hierarchize nodes
	ASSERT(nodes.size() == child_indices.size())
	for (uint32_t i = 0; i < nodes.size(); ++i) {
		for (uint32_t ci : child_indices[i]) {
			nodes[i].get().children.push_back(nodes[ci]);
		}
	}

	it = json.object().find("scenes");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			scenes.push_back(read_scene(sub_json));
		}
	}

	auto active_scene = utki::make_shared<scene>();
	int active_scene_index = read_int(json, "scene");
	if (active_scene_index < 0) {
		// this .gltf file is a library
	} else {
		active_scene = scenes[active_scene_index];
	}

	return active_scene;
}

template <typename tp_type>
utki::shared_ref<ruis::render::vertex_array> gltf_loader::create_vao_with_tangent_space(
	utki::shared_ref<accessor> index_accessor,
	utki::shared_ref<accessor> position_accessor,
	utki::shared_ref<accessor> texcoord_0_accessor,
	utki::shared_ref<accessor> normal_accessor
)
{
	uint32_t total_vertices = position_accessor.get().count;
	uint32_t total_triangles = index_accessor.get().count / 3;

	const auto& indices = std::get<std::vector<tp_type>>(index_accessor.get().data);
	const auto& positions = std::get<std::vector<ruis::vec3>>(position_accessor.get().data);
	const auto& texcoords = std::get<std::vector<ruis::vec2>>(texcoord_0_accessor.get().data);
	const auto& normals = std::get<std::vector<ruis::vec3>>(normal_accessor.get().data);

	std::vector<ruis::vec3> tangents;
	std::vector<ruis::vec3> bitangents;

	tangents.resize(total_vertices);
	bitangents.resize(total_vertices);

	ruis::vec2 tex_edge_1 = {0.0f, 0.0f};
	ruis::vec2 tex_edge_2 = {0.0f, 0.0f};

	ruis::vec3 edge1 = {0.0f, 0.0f, 0.0f};
	ruis::vec3 edge2 = {0.0f, 0.0f, 0.0f};

	ruis::vec3 bitangent = {0.0f, 0.0f, 0.0f};

	ruis::vec4 tangent = {0.0f, 0.0f, 0.0f, 0.0f};

	// Calculate the vertex tangents and bitangents.
	for (uint32_t i = 0; i < total_triangles; ++i) {
		edge1 = positions[indices[i * 3 + 1]] - positions[indices[i * 3 + 0]];
		edge2 = positions[indices[i * 3 + 2]] - positions[indices[i * 3 + 0]];

		tex_edge_1 = texcoords[indices[i * 3 + 1]] - texcoords[indices[i * 3 + 0]];
		tex_edge_2 = texcoords[indices[i * 3 + 2]] - texcoords[indices[i * 3 + 0]];

		// Calculate the triangle face tangent and bitangent.
		float det = tex_edge_1[0] * tex_edge_2[1] - tex_edge_2[0] * tex_edge_1[1];

		constexpr float epsilon = 1e-6f;

		using std::abs;
		if (abs(det) < epsilon) {
			tangent = {1.0f, 0.0f, 0.0f, 0.0f};
			bitangent = {0.0f, 1.0f, 0.0f};
		} else {
			det = 1.0f / det;

			tangent[0] = (tex_edge_2[1] * edge1[0] - tex_edge_1[1] * edge2[0]) * det;
			tangent[1] = (tex_edge_2[1] * edge1[1] - tex_edge_1[1] * edge2[1]) * det;
			tangent[2] = (tex_edge_2[1] * edge1[2] - tex_edge_1[1] * edge2[2]) * det;

			bitangent[0] = (-tex_edge_2[0] * edge1[0] + tex_edge_1[0] * edge2[0]) * det;
			bitangent[1] = (-tex_edge_2[0] * edge1[1] + tex_edge_1[0] * edge2[1]) * det;
			bitangent[2] = (-tex_edge_2[0] * edge1[2] + tex_edge_1[0] * edge2[2]) * det;
		}

		// TODO: figure out what is happening here and refactor with vector ops

		// Accumulate the tangents and bitangents.
		tangents[indices[i * 3 + 0]] += tangent;
		bitangents[indices[i * 3 + 0]] += bitangent;

		tangents[indices[i * 3 + 1]] += tangent;
		bitangents[indices[i * 3 + 1]] += bitangent;

		tangents[indices[i * 3 + 2]] += tangent;
		bitangents[indices[i * 3 + 2]] += bitangent;
	}

	// Orthogonalize and normalize the vertex tangents.
	for (uint32_t i = 0; i < total_vertices; ++i) {
		// Gram-Schmidt orthogonalize tangent with normal.

		float n_dot_t = normals[i] * tangents[i]; // dot product

		tangents[i] -= normals[i] * n_dot_t;

		// Normalize the tangent.
		tangents[i].normalize();

		ruis::vec3 bitangent_other = normals[i].cross(tangents[i]);

		float b_dot_b = bitangent_other * bitangents[i];

		float sign = (b_dot_b < 0.0f) ? -1.0f : 1.0f;

		bitangents[i] = bitangent_other * sign;
	}

	auto tangents_vbo = factory_v.create_vertex_buffer(utki::make_span(tangents));
	auto bitangents_vbo = factory_v.create_vertex_buffer(utki::make_span(bitangents));

	auto vao = factory_v.create_vertex_array(
		{utki::shared_ref<ruis::render::vertex_buffer>(position_accessor.get().vbo),
		 utki::shared_ref<ruis::render::vertex_buffer>(texcoord_0_accessor.get().vbo),
		 utki::shared_ref<ruis::render::vertex_buffer>(normal_accessor.get().vbo),
		 tangents_vbo,
		 bitangents_vbo},
		utki::shared_ref<ruis::render::index_buffer>(index_accessor.get().ibo),
		ruis::render::vertex_array::mode::triangles
	);

	return vao;
}

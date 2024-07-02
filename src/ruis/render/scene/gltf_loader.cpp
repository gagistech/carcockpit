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

gltf_loader::gltf_loader(ruis::render::factory& factory_) :
	factory_(factory_)
{}

inline int read_int(const jsondom::value& json, const std::string& name, const int default_value = -1)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) 
		return it->second.number().to_int32();
	
	return default_value;
}

inline uint32_t read_uint(const jsondom::value& json, const std::string& name, const uint32_t default_value = 0)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) 
		return it->second.number().to_uint32();
	
	return default_value;
}

inline float read_float(const jsondom::value& json, const std::string& name, const float default_value = 0.0f)
{
	auto it = json.object().find(name);
	if (it != json.object().end()) 
		return it->second.number().to_float();
	
	return default_value;
}

inline std::string read_string(const jsondom::value& json, const std::string& name, const std::string default_value = "")
{
	auto it = json.object().find(name);
	if (it != json.object().end()) 
		return it->second.string();

	return default_value;
}

std::vector<uint32_t> read_uint_array(
	const jsondom::value& json,
	const std::string& name,
	uint32_t dafault_value = 0
)
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


template <typename T, size_t dimension>
r4::vector<T, dimension> read_vec(const jsondom::value& json, const std::string& name, const r4::vector<T, dimension> default_value)
{
	auto it = json.object().find(name);

	if(it == json.object().end() || !json.object().at(name).is_array())
		return default_value;

	r4::vector<T, dimension> value;
	int i = 0;
	for (const auto& subjson : json.object().at(name).array()) {
		value[i++] = subjson.number().to_float();
	}
	return value;
}

ruis::vec2 read_vec2(const jsondom::value& json, const std::string& name, const ruis::vec2 default_value = {0, 0})
{
	return read_vec<float, 2>(json, name, default_value);
}

ruis::vec3 read_vec3(const jsondom::value& json, const std::string& name, const ruis::vec3 default_value = {0, 0, 0})
{
	return read_vec<float, 3>(json, name, default_value);
}

ruis::vec4 read_vec4(const jsondom::value& json, const std::string& name, const ruis::vec4 default_value = {0, 0, 0, 0})
{
	return read_vec<float, 4>(json, name, default_value);
}

ruis::quat read_quat(const jsondom::value& json, const std::string& name, const ruis::quat default_value = {1, 0, 0, 0})
{
	auto it = json.object().find(name);

	if(it == json.object().end() || !json.object().at(name).is_array())
		return default_value;

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
	const uint32_t byte_length = read_uint(buffer_view_json, "byteLength");
	const uint32_t byte_offset = read_uint(buffer_view_json, "byteOffset");
	const uint32_t byte_stride = read_uint(buffer_view_json, "byteStride");
	const uint32_t target = read_uint(buffer_view_json, "target");

	auto new_buffer_view =
		utki::make_shared<buffer_view>(byte_length, byte_offset, byte_stride, static_cast<buffer_view::target>(target));
	return new_buffer_view;
}

template <typename T>
void gltf_loader::create_vertex_buffer_float(
	utki::shared_ref<ruis::render::accessor> new_accessor,
	utki::span<const uint8_t> buffer,
	uint32_t acc_count, // in elements (e.g. a whole vec3)
	// uint32_t acc_offset, // in bytes
	uint32_t acc_stride // in bytes
)
{
	utki::deserializer d(buffer);
	std::vector<T> vertex_attribute_buffer;
	vertex_attribute_buffer.reserve(acc_count);

	int n_skip_bytes = int(acc_stride) - int(sizeof(T));
	if (n_skip_bytes < 0) // TODO exception
		n_skip_bytes = 0;

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
		// also, only float vectors for now. Need a template<> deserialize function for nice notation
		vertex_attribute_buffer.push_back(std::move(t));
		d.skip(n_skip_bytes);
	}

	// std::shared_ptr<vertex_data_t> var = std::make_shared<vertex_data_t>( std::move(vertex_attribute_buffer) );
	// new_accessor.get().data = var;
	// new_accessor.get().vbo = factory_.create_vertex_buffer(utki::make_span(new_accessor.get().data.get()));

	new_accessor.get().vbo = factory_.create_vertex_buffer(utki::make_span(vertex_attribute_buffer));
	new_accessor.get().data = std::move(vertex_attribute_buffer);
	

	// return std::make_shared<vertex_data_t>(var);
}

utki::shared_ref<accessor> gltf_loader::read_accessor(const jsondom::value& accessor_json)
{
	accessor::type type_ = accessor::type::vec3;
	const std::string type_s = read_string(accessor_json, "type");

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

	const uint32_t buffer_view_index = read_uint(accessor_json, "bufferView");
	const uint32_t acc_count = read_uint(accessor_json, "count");
	const uint32_t acc_offset = read_uint(accessor_json, "byteOffset");
	const uint32_t component_type = read_uint(accessor_json, "componentType");

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
			create_vertex_buffer_float<float>(new_accessor, buf, acc_count, bv_stride);
		else if (new_accessor.get().type_ == accessor::type::vec2)
			create_vertex_buffer_float<ruis::vec2>(new_accessor, buf, acc_count, bv_stride);
		else if (new_accessor.get().type_ == accessor::type::vec3)
			create_vertex_buffer_float<ruis::vec3>(new_accessor, buf, acc_count, bv_stride);
		else if (new_accessor.get().type_ == accessor::type::vec4)
			create_vertex_buffer_float<ruis::vec4>(new_accessor, buf, acc_count, bv_stride);
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
		if (new_accessor.get().component_type_ == accessor::component_type::act_unsigned_short /*|| acc_count < 65536*/)
		{
			std::vector<uint16_t> index_attribute_buffer;
			index_attribute_buffer.reserve(acc_count);

			for (size_t i = 0; i < acc_count; ++i)
				index_attribute_buffer.push_back(d.read_uint16_le());

			new_accessor.get().data = index_attribute_buffer;
			new_accessor.get().ibo = factory_.create_index_buffer(utki::make_span(index_attribute_buffer));

		} else if (new_accessor.get().component_type_ ==
				   accessor::component_type::act_unsigned_int /*|| acc_count >= 65536*/)
		{
			std::vector<uint32_t> index_attribute_buffer;
			index_attribute_buffer.reserve(acc_count);

			for (size_t i = 0; i < acc_count; ++i)
				index_attribute_buffer.push_back(d.read_uint32_le());

			new_accessor.get().data = index_attribute_buffer;
			new_accessor.get().ibo = factory_.create_index_buffer(utki::make_span(index_attribute_buffer));
		}

		// if (use_short_indices) { // opengles 2.0 does not support 32-bit indices by default
		// 	std::vector<uint16_t> index_attribute_buffer;
		// 	index_attribute_buffer.reserve(acc_count);

		// 	for (size_t i = 0; i < acc_count; ++i)
		// 		index_attribute_buffer.push_back(static_cast<uint16_t>(d.read_uint32_le()));

		// 	new_accessor.get().ibo = factory_.create_index_buffer(utki::make_span(index_attribute_buffer));

		// }
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
		int tangent_accessor = read_int(attributes_json, "TANGENT");
		int material_index = read_int(json_primitive, "material");

		if (index_accessor >= 0 && texcoord_0_accessor >= 0 && normal_accessor >= 0 && tangent_accessor < 0) {
			// generate tangents and bitangents
			if (accessors[index_accessor].get().component_type_ == accessor::component_type::act_unsigned_int) {
				auto vao = create_vao_with_tangent_space<uint32_t>(
					accessors[index_accessor],
					accessors[position_accessor],
					accessors[texcoord_0_accessor],
					accessors[normal_accessor]
				);

				auto material_ = material_index >= 0 ? materials[material_index] : utki::make_shared<material>();
				primitives.push_back(utki::make_shared<primitive>(vao, material_));

			} else if (accessors[index_accessor].get().component_type_ == accessor::component_type::act_unsigned_short)
			{
				auto vao = create_vao_with_tangent_space<uint32_t>(
					accessors[index_accessor],
					accessors[position_accessor],
					accessors[texcoord_0_accessor],
					accessors[normal_accessor]
				);

				auto material_ = material_index >= 0 ? materials[material_index] : utki::make_shared<material>();
				primitives.push_back(utki::make_shared<primitive>(vao, material_));
			}
			else
			{
				// TODO: branch all possible combinations if input data 
			}
		}

		// 	auto vao = factory_.create_vertex_array(
		// 		{
		// 			utki::shared_ref<ruis::render::vertex_buffer>(accessors[position_accessor].get().vbo),
		// 			utki::shared_ref<ruis::render::vertex_buffer>(accessors[texcoord_0_accessor].get().vbo),
		// 			utki::shared_ref<ruis::render::vertex_buffer>(accessors[normal_accessor].get().vbo),
		// 			utki::shared_ref<ruis::render::vertex_buffer>(accessors[tangent_accessor].get().vbo)
		// 			// , vbo_bitangents
		// 		},
		// 		utki::shared_ref<ruis::render::index_buffer>(accessors[index_accessor].get().ibo),
		// 		ruis::render::vertex_array::mode::triangles
		// 	);		
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

	int mesh_index = read_int(node_json, "mesh");
	child_indices.push_back( read_uint_array(node_json, "children") );

	auto new_node = utki::make_shared<node>(mesh_index >= 0 ? meshes[mesh_index].to_shared_ptr() : nullptr, name, transformation);
	return new_node;
}

utki::shared_ref<scene> gltf_loader::read_scene(const jsondom::value& scene_json)
{
	auto new_scene = utki::make_shared<scene>();
	std::vector<uint32_t> node_indices = read_uint_array(scene_json, "nodes");;
	new_scene.get().name = read_string(scene_json, "name");
	
	for (uint32_t ni : node_indices) {
		new_scene.get().nodes.push_back(this->nodes[ni]);
	}

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
	if (image.get().mime_type_ == image_l::mime_type::image_png) {
		imvar = rasterimage::read_png(fi);
	} else if (image.get().mime_type_ == image_l::mime_type::image_jpeg) {
		imvar = rasterimage::read_jpeg(fi);
	} else {
		throw std::invalid_argument("gltf: unknown texture image format");
	}

	ruis::render::factory::texture_2d_parameters tex_params; // TODO: fill texparams properly base on gltf file
	tex_params.mag_filter = ruis::render::texture_2d::filter::linear;
	tex_params.min_filter = ruis::render::texture_2d::filter::linear;
	tex_params.mipmap = texture_2d::mipmap::linear;

	return factory_.create_texture_2d(std::move(imvar), tex_params);
}

// "materials": [
//     {
//       "name": "spray_paint_bottles_02",
//       "doubleSided": true,
//       "pbrMetallicRoughness": {
//         "baseColorFactor": [
//           1,
//           1,
//           1,
//           1
//         ],
//         "baseColorTexture": {
//           "index": 0
//         },
//         "metallicRoughnessTexture": {
//           "index": 1
//         }
//       },
//       "normalTexture": {
//         "index": 2
//       }
//     }
//   ],

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

template <typename T>
std::vector<utki::shared_ref<T>> gltf_loader::read_root_array(
	std::function<T(const jsondom::value& j)> read_func,
	const jsondom::value& root_json,
	const std::string& name
)
{
	std::vector<utki::shared_ref<T>> all;
	std::cout << "loading " << name << std::endl;
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

	//////////////////////////////////////////////////////////////////////

	// images = read_root_array<image_l>(read_image, json, "images");
	// samplers = read_root_array<sampler_l>(read_sampler, json, "samplers");

	decltype(json.object().find("")) it;

	std::cout << "loading images" << std::endl;
	it = json.object().find("images");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			images.push_back(read_image(sub_json));
		}
	}

	std::cout << "loading samplers" << std::endl;
	it = json.object().find("samplers");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			samplers.push_back(read_sampler(sub_json));
		}
	}

	std::cout << "loading textures" << std::endl;
	it = json.object().find("textures");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			textures.push_back(read_texture(sub_json));
		}
	}

	std::cout << "loading materials" << std::endl;
	it = json.object().find("materials");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			materials.push_back(read_material(sub_json));
		}
	}

	std::cout << "loading accessors" << std::endl;
	it = json.object().find("accessors");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			accessors.push_back(read_accessor(sub_json));
		}
	}

	std::cout << "loading meshes" << std::endl;
	it = json.object().find("meshes");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			meshes.push_back(read_mesh(sub_json));
		}
	}

	std::cout << "loading nodes" << std::endl;
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

	std::cout << "loading scenes" << std::endl;
	it = json.object().find("scenes");
	if (it != json.object().end() && it->second.is_array()) {
		for (const auto& sub_json : it->second.array()) {
			scenes.push_back(read_scene(sub_json));
		}
	}

	// create scene
	auto active_scene = utki::make_shared<scene>();
	int active_scene_index = read_int(json, "scene");
	if (active_scene_index < 0) {
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

template <typename index_t>
utki::shared_ref<ruis::render::vertex_array> gltf_loader::create_vao_with_tangent_space(
	utki::shared_ref<accessor> index_accessor,
	utki::shared_ref<accessor> position_accessor,
	utki::shared_ref<accessor> texcoord_0_accessor,
	utki::shared_ref<accessor> normal_accessor
)
{
	int total_vertices = position_accessor.get().count;
	int total_triangles = index_accessor.get().count / 3;

	//const index_t* index_data = static_cast<const index_t*>(index_accessor.get().data);
	//const index_t* pTriangle;

	std::cout << "index is " << sizeof(index_t) << "-byte" << std::endl;

	const auto& indices = std::get<std::vector<index_t>>(index_accessor.get().data);

	const auto& positions = std::get<std::vector<ruis::vec3>>(position_accessor.get().data);
	const auto& texcoords = std::get<std::vector<ruis::vec2>>(texcoord_0_accessor.get().data);
	const auto& normals = std::get<std::vector<ruis::vec3>>(normal_accessor.get().data);

	// const float* position_data = static_cast<const float*>(position_accessor.get().data);
	// const float* texcoord_data = static_cast<const float*>(texcoord_0_accessor.get().data);
	// const float* normal_data = static_cast<const float*>(normal_accessor.get().data);

	std::vector<ruis::vec3> tangents;  // or vec4 ?
	std::vector<ruis::vec3> bitangents;

	tangents.resize(total_vertices);
	bitangents.resize(total_vertices);

	ruis::vec2 texEdge1 = {0.0f, 0.0f};
	ruis::vec2 texEdge2 = {0.0f, 0.0f};

	ruis::vec3 edge1 = {0.0f, 0.0f, 0.0f};
	ruis::vec3 edge2 = {0.0f, 0.0f, 0.0f};

	ruis::vec3 bitangent = {0.0f, 0.0f, 0.0f};

	ruis::vec4 tangent = {0.0f, 0.0f, 0.0f, 0.0f};

	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < total_triangles; ++i) {
		
		//pTriangle = &index_data[i * 3];

		edge1 = positions[indices[i*3 + 1]] - positions[indices[i*3 + 0]];
		edge2 = positions[indices[i*3 + 2]] - positions[indices[i*3 + 0]];

		texEdge1 = texcoords[indices[i*3 + 1]] - texcoords[indices[i*3 + 0]];
		texEdge2 = texcoords[indices[i*3 + 2]] - texcoords[indices[i*3 + 0]];

		// edge1[0] = position_data[pTriangle[1] + 0] - position_data[pTriangle[0] + 0];
		// edge1[1] = position_data[pTriangle[1] + 1] - position_data[pTriangle[0] + 1];
		// edge1[2] = position_data[pTriangle[1] + 2] - position_data[pTriangle[0] + 2];

		// edge2[0] = position_data[pTriangle[2] + 0] - position_data[pTriangle[0] + 0];
		// edge2[1] = position_data[pTriangle[2] + 1] - position_data[pTriangle[0] + 1];
		// edge2[2] = position_data[pTriangle[2] + 2] - position_data[pTriangle[0] + 2];

		// texEdge1[0] = texcoord_data[pTriangle[1] + 0] - texcoord_data[pTriangle[0] + 0];
		// texEdge1[1] = texcoord_data[pTriangle[1] + 1] - texcoord_data[pTriangle[0] + 1];

		// texEdge2[0] = texcoord_data[pTriangle[2] + 0] - texcoord_data[pTriangle[0] + 0];
		// texEdge2[1] = texcoord_data[pTriangle[2] + 1] - texcoord_data[pTriangle[0] + 1];

		// Calculate the triangle face tangent and bitangent.

		float det = texEdge1[0] * texEdge2[1] - texEdge2[0] * texEdge1[1];

		using std::abs;
		if (abs(det) < 1e-6f) {
			tangent = {1.0f, 0.0f, 0.0f, 0.0f};
			bitangent = {0.0f, 1.0f, 0.0f};
		} else {
			det = 1.0f / det;

			tangent[0] = (texEdge2[1] * edge1[0] - texEdge1[1] * edge2[0]) * det;
			tangent[1] = (texEdge2[1] * edge1[1] - texEdge1[1] * edge2[1]) * det;
			tangent[2] = (texEdge2[1] * edge1[2] - texEdge1[1] * edge2[2]) * det;

			bitangent[0] = (-texEdge2[0] * edge1[0] + texEdge1[0] * edge2[0]) * det;
			bitangent[1] = (-texEdge2[0] * edge1[1] + texEdge1[0] * edge2[1]) * det;
			bitangent[2] = (-texEdge2[0] * edge1[2] + texEdge1[0] * edge2[2]) * det;
		}

		// TODO: figure out what is happening here and refactor with vector ops

		// Accumulate the tangents and bitangents.
		tangents[indices[i*3 + 0]] += tangent;
		bitangents[indices[i*3 + 0]] += bitangent;

		tangents[indices[i*3 + 1]] += tangent;
		bitangents[indices[i*3 + 1]] += bitangent;

		tangents[indices[i*3 + 2]] += tangent;
		bitangents[indices[i*3 + 2]] += bitangent;
	}

	// Orthogonalize and normalize the vertex tangents.
	for (int i = 0; i < total_vertices; ++i) {
		// Gram-Schmidt orthogonalize tangent with normal.
		//ruis::vec3 normal{normal_data[i * 3], normal_data[i * 3 + 1], normal_data[i * 3 + 2]};

		float nDotT = normals[i] * tangents[i]; // dot product

		tangents[i] -= normals[i] * nDotT;

		// Normalize the tangent.
		tangents[i].normalize();

		// Calculate the handedness of the local tangent space.
		// The bitangent vector is the cross product between the triangle face
		// normal vector and the calculated tangent vector. The resulting
		// bitangent vector should be the same as the bitangent vector
		// calculated from the set of linear equations above. If they point in
		// different directions then we need to invert the cross product
		// calculated bitangent vector. We store this scalar multiplier in the
		// tangent vector's 'w' component so that the correct bitangent vector
		// can be generated in the normal mapping shader's vertex shader.
		//
		// TODO: revise this comment

		ruis::vec3 bitangent_other = normals[i].cross(tangents[i]);

		float bDotB = bitangent_other * bitangents[i];

		//tangents[i][3] = (bDotB < 0.0f) ? 1.0f : -1.0f;
		float sign = (bDotB < 0.0f) ? -1.0f : 1.0f;

		bitangents[i] = bitangent_other * sign;
	}

	auto tangents_vbo = factory_.create_vertex_buffer(utki::make_span(tangents));
	auto bitangents_vbo = factory_.create_vertex_buffer(utki::make_span(bitangents));

	auto vao = factory_.create_vertex_array(
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

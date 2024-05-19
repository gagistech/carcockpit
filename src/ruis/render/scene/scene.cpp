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

utki::shared_ref<scene> ruis::render::read_gltf(const papki::file& fi, ruis::render::render_factory& rf)
{
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

	ASSERT(json.is_object())

	for (const auto& kv : json.object()) {
		std::cout << "key = " << kv.first << ", value type = " << unsigned(kv.second.get_type()) << std::endl;
	}

	p = p.subspan(chunk_length);

	// read binary chunk
	{
		// TODO: bin_span
	}

	// TODO:

	return utki::make_shared<scene>();
}

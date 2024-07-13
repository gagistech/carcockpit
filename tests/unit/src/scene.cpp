#include <papki/fs_file.hpp>
#include <ruis/render/opengles/renderer.hpp>
#include <ruis/render/scene/gltf_loader.hxx>
#include <tst/check.hpp>
#include <tst/set.hpp>

namespace {
const tst::set set("scene", [](tst::suite& suite) {
	suite.add("basic_read", []() {
		ruis::render::opengles::factory rf;

		//{
		// auto sc = ruis::render::read_gltf(papki::fs_file("samples_gltf/kub.glb"), rf);
		// tst::check(!sc.get().nodes.empty(), SL);
		//}

		{
			ruis::render::gltf_loader l(rf);
			auto scene = l.load(papki::fs_file("samples_gltf/parent_and_children.glb"));
			tst::check(!scene.get().nodes.empty(), SL);
		}
	});
});
} // namespace

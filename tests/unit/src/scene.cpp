#include <papki/fs_file.hpp>
#include <ruis/render/null/context.hpp>
#include <ruis/render/scene/gltf_loader.hxx>
#include <ruis/render/scene/scene.hpp>
#include <tst/check.hpp>
#include <tst/set.hpp>

namespace {
const tst::set set("scene", [](tst::suite& suite) {
	suite.add("basic_read", []() {
		auto rc = utki::make_shared<ruis::render::null::context>();
		{
			ruis::render::gltf_loader l(rc.get());
			auto scene = l.load(papki::fs_file("samples_gltf/kub.glb"));
			tst::check(!scene.get().nodes.empty(), SL);
		}
	});

	suite.add("hierarchy", []() {
		auto rc = utki::make_shared<ruis::render::null::context>();
		{
			ruis::render::gltf_loader l(rc.get());
			auto scene = l.load(papki::fs_file("samples_gltf/parent_and_children.glb"));
			tst::check(!scene.get().nodes[0].get().children.empty(), SL);
		}
	});

	suite.add("spray_paint_model", []() {
		auto rc = utki::make_shared<ruis::render::null::context>();
		{
			ruis::render::gltf_loader l(rc.get());
			auto scene = l.load(papki::fs_file("samples_gltf/spray.glb"));
			tst::check(!scene.get().nodes.empty(), SL);
		}
	});

	suite.add("vintage_camera_model", []() {
		auto rc = utki::make_shared<ruis::render::null::context>();
		{
			ruis::render::gltf_loader l(rc.get());
			auto scene = l.load(papki::fs_file("samples_gltf/camera.glb"));
			tst::check(!scene.get().nodes.empty(), SL);
		}
	});
});
} // namespace

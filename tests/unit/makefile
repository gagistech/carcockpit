include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs := $(call prorab-src-dir, src)

this_cxxflags += -isystem ../../src/
this__libruis_render := ../../src/out/$(c)/libruis_render.a
this_ldlibs += $(this__libruis_render)

this_ldlibs += -l tst -l utki -l papki -l ruis-render-opengles
this_ldlibs += -l jsondom
this_ldlibs += -l rasterimage
this_ldlibs += -l m
this_ldlibs += -l ruisapp-opengles-xorg # TODO: remove when move gltf to ruis

this_no_install := true

$(eval $(prorab-build-app))

$(eval $(call prorab-depend, $(prorab_this_name), $(this__libruis_render)))

this_test_cmd := $(prorab_this_name) --jobs=auto --junit-out=junit.xml
this_test_deps := $(prorab_this_name)
this_test_ld_path := ../../src/out/$(c)

$(eval $(prorab-test))

this_src_dir := src
$(eval $(prorab-clang-format))

$(eval $(call prorab-include, ../../src/makefile))

include prorab.mk
include prorab-test.mk
include prorab-clang-format.mk
include prorab-license.mk

# ===============================
# ========= model_obj ===========
$(eval $(prorab-clear-this-vars))

this_name := model_obj

$(eval $(call prorab-config, ../config))

this_srcs += $(call prorab-src-dir, model_obj)

this_static_lib_only := true
this_no_install := true

# no lint for model_obj
this_lint_cmd := 

$(eval $(prorab-build-lib))


# ==========================
# ========= ruis ===========
$(eval $(prorab-clear-this-vars))

this_name := ruis_render

$(eval $(call prorab-config, ../config))

this_srcs += $(call prorab-src-dir, ruis)
#this_ldlibs += `pkg-config --libs glesv2`
#this_ldlibs += -lruisapp-opengles-xorg -l ruis-render-opengles

this_static_lib_only := true
this_no_install := true

$(eval $(prorab-build-lib))

this_src_dir := ruis
$(eval $(prorab-clang-format))


# ===============================
# ========= carcockpit ==========
$(eval $(prorab-clear-this-vars))

this_name := carcockpit

$(eval $(call prorab-config, ../config))

this_srcs += $(call prorab-src-dir, carcockpit)

ifeq ($(os),windows)
    this_ldlibs += -lmingw32 # these should go first, otherwise linker will complain about undefined reference to WinMain
    this_ldlibs += -lglew32 -lopengl32 -lz -lfreetype -mwindows
else ifeq ($(os),macosx)
    this_ldlibs += -lGLEW -framework OpenGL -framework Cocoa -lfreetype
    this_ldflags += -rdynamic
else ifeq ($(os),linux)
    this_ldlibs += -pthread
    this_ldflags += -rdynamic
endif

this__libmodel_obj := $(this_out_dir)libmodel_obj.a
this__libruis_render := $(this_out_dir)libruis_render.a

this_cxxflags += -isystem model_obj
this_ldlibs += $(this__libmodel_obj) $(this__libruis_render)

this_ldlibs += -lruisapp-opengles-xorg -lpapki -lclargs -ltml -lruis -lutki -lm -l ruis-render-opengles
this_ldlibs += -l jsondom
this_ldlibs += -l rasterimage
this_ldlibs += `pkg-config --libs glesv2`

$(eval $(prorab-build-app))

$(eval $(call prorab-depend, $(prorab_this_name), $(this__libmodel_obj) $(this__libruis_render)))

this_run_name := app
this_test_cmd := $(prorab_this_name) --window --res-path=../res
this_test_deps := $(prorab_this_name)
this_test_ld_path := $(prorab_space)
$(eval $(prorab-run))

this_src_dir := $(this_name)
$(eval $(prorab-clang-format))
this_license_file := ../LICENSE
$(eval $(prorab-license))

# install resources
define this_rules
install::
$(.RECIPEPREFIX)$(a)for i in $(patsubst ../res/%,/%,$(shell (cd $(d); find ../res -type f -name "*"))); do \
		install -d $(DESTDIR)$(PREFIX)/share/$(this_name)$$$${i%/*}; \
		install -m 644 $(d)../res$$$$i $(DESTDIR)$(PREFIX)/share/$(this_name)$$$$i; \
	done
endef
$(eval $(this_rules))

include prorab.mk
include prorab-test.mk
include prorab-clang-format.mk
include prorab-license.mk

# ==================================
# ============== ruis ==============
$(eval $(prorab-clear-this-vars))

this_name := ruis_render

$(eval $(call prorab-config, ../config))

this_srcs += $(call prorab-src-dir, ruis)

this_static_lib_only := true
this_no_install := true

$(eval $(prorab-build-lib))

this_src_dir := ruis
$(eval $(prorab-clang-format))
this_license_file := ../LICENSE
$(eval $(prorab-license))

# =====================================
# ============== library ==============
$(eval $(prorab-clear-this-vars))
this_name := carcockpit

$(eval $(call prorab-config, ../config))

this__root_dir_rel := ../

this__version_cpp := $(this_out_dir)version.cpp

this_srcs += $(this__version_cpp) $(filter-out %main.cpp,$(call prorab-src-dir, $(this_name)))

this_no_install := true
this_static_lib_only := true

$(eval $(prorab-build-lib))

define this__rules
$(abspath $(d)$(this__version_cpp)): $(abspath $(d)$(this__root_dir_rel)build/debian/changelog)
$(.RECIPEPREFIX)$(a)mkdir -p $$(dir $$@)
$(.RECIPEPREFIX)$(a)(cd $(d)$(this__root_dir_rel); echo "extern const char* const program_version = \"$$$$(myci-deb-version.sh)\";" > $$@)
endef
$(eval $(this__rules))

# ================================================
# ============== format and license ==============
this_src_dir := $(this_name)
$(eval $(prorab-clang-format))
this_license_file := ../LICENSE
$(eval $(prorab-license))

# ===============================================
# ============== install resources ==============
define this_rules
install::
$(.RECIPEPREFIX)$(a)for i in $(patsubst ../res/%,/%,$(shell (cd $(d); find ../res -type f -name "*"))); do \
		install -d $(DESTDIR)$(PREFIX)/share/$(this_name)$$$${i%/*}; \
		install -m 644 $(d)../res$$$$i $(DESTDIR)$(PREFIX)/share/$(this_name)$$$$i; \
	done
endef
$(eval $(this_rules))

# =========================================
# ============== application ==============
define carcockpit_rules
    $$(eval $$(prorab-clear-this-vars))

    this_name := carcockpit-$1-$2

    $$(eval $$(call prorab-config, ../config))

    this_srcs += carcockpit/main.cpp

    this_ldlibs += -pthread
    this_ldflags += -rdynamic

    this_ldlibs += $$(this_out_dir)libcarcockpit.a
    this_ldlibs += $$(this_out_dir)libruis_render.a

    ifeq ($(config), emsc)
        this_name := $$(this_name).html

        this_ldflags += --embed-file $(d)../res@res
    else
        this_ldlibs += -l clargs
    endif

    this_ldlibs += -l jsondom
    this_ldlibs += -l rasterimage
    this_ldlibs += -l papki
    this_ldlibs += -l tml
    this_ldlibs += -l ruis
    this_ldlibs += -l utki
    this_ldlibs += -l m

    this_ldlibs += -l ruisapp-$1-$2

    this_ldlibs += -l ruis-render-$1
    this_ldlibs += `pkg-config --libs glesv2`

    $$(eval $$(prorab-build-app))

    this_run_name := app-$1-$2
    this_test_cmd := $$(prorab_this_name) --window --res-path=../res
    this_test_deps := $$(prorab_this_name)
    this_test_ld_path := $$(prorab_space)
    $$(eval $$(prorab-run))

endef

ifneq ($(config), emsc)
    # non-Emscripten

    # $(eval $(call carcockpit_rules,opengl,xorg))
    $(eval $(call carcockpit_rules,opengles,xorg))
    # $(eval $(call carcockpit_rules,opengl,wayland))
    $(eval $(call carcockpit_rules,opengles,wayland))
else
    # Emscripten

    $(eval $(call carcockpit_rules,opengles,sdl))
endif

# clean up
carcockpit_rules :=

# ==================================================
# = targets for building with conan and emscripten =

define this__rules
conan_install:
$(.RECIPEPREFIX)@echo install conan dependencies
$(.RECIPEPREFIX)$(a)conan install $(d)../build/emscripten --update --profile:host $(d)../build/emscripten/conan.profile --build=missing --output-folder=$(d)out/emsc --deployer-package="*"
endef
$(eval $(this__rules))

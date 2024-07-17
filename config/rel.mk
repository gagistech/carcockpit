include $(config_dir)base/base.mk

# TODO: set optimization back to -O3 when Debian Bookworm GCC is fixed
this_cxxflags += -O2

this_lint_cmd = $(prorab_lint_cmd_clang_tidy)

# WORKAROUND: on ubuntu jammy dpkg-buildpackage passes -ffat-lto-objects compilation flag
# which is not supported by clang and clang-tidy complains about it:
# error: optimization flag '-ffat-lto-objects' is not supported [clang-diagnostic-ignored-optimization-argument]
# Thus, suppress this warning.
this_cxxflags += -Wno-ignored-optimization-argument

this_cxxflags += -isystem /usr/include/c++/11
this_cxxflags += -isystem /usr/include/x86_64-linux-gnu/c++/11/

ifeq ($(os),macosx)
    # WORKAROUND:
    # clang-tidy on macos doesn't use /usr/local/include as default place to
    # search for header files, so we add it explicitly
    this_cxxflags += -isystem /usr/local/include
endif


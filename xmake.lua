add_requires(
  "fmtlog",
  "vcpkg::libmodbus",
  "vcpkg::crow",
  "vcpkg::nlohmann-json",
  "vcpkg::libpqxx",
  "vcpkg::libpq"
)

set_policy( "build.across_targets_in_parallel", false)
add_rules("mode.debug", "mode.release")
set_languages("c23", "cxx23")
-- set_optimize("none")
set_optimize("fast")
add_defines("NDEBUG", "_GNU_SOURCE=1")
set_warnings("all")
set_rundir("./")
set_toolchains("gcc")

target("sensord")
add_packages(
  "fmtlog",
  "vcpkg::libmodbus",
  "vcpkg::crow",
  "vcpkg::nlohmann-json",
  "vcpkg::libpqxx",
  "vcpkg::libpq"
)
-- add_ldflags("-static")
set_kind("binary")
add_files(
  "src/**.cpp"
)
add_includedirs(
  "src/"
)

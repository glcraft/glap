add_requires("glap")
add_requires("yaml-cpp")

target("make-my-glap")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("glap", "yaml-cpp")

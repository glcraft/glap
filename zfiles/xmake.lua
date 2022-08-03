
target("zfiles")
    set_kind("shared")
    set_languages("cxxlatest", "clatest")
    add_packages("libarchive")
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_includedirs("include")
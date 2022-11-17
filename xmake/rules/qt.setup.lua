

rule("qt.setup")
    add_deps("qt.quickapp")
    on_load(function (target, opt)
        target:add("frameworks", 
            "QtCore",
            "QtGui",
            "QtQml",
            "QtQuick",
            "QtQuickControls2"
        )
        target:set("qt.deploy.qmldir", "qml/")
        if is_plat("macosx") then
            import("lib.detect.find_path")
            local qt_include = find_path("QtGui/QGuiApplication", { 
                    "/usr/include", 
                    "/usr/local/include", 
                    "/opt/homebrew/include", 
                    "$($HOME)/6.*/macos/include" 
                })
            target:add("includedirs", qt_include)
            target:add("qt.deploy.flags", "-no-plugins")
        end
    end)
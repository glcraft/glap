rule("qt.setup")
    add_deps("qt.quickapp")
    before_build(function (target, opt)
        target:add("frameworks", 
            "QtCore",
            "QtGui",
            "QtQml",
            "QtQuick",
            "QtQuickControls2"
        )
        target:set("qt.deploy.qmldir", "qml/")
        if is_plat("macosx") then
            target:add("includedirs", "/opt/homebrew/include")
            target:add("qt.deploy.flags", "-no-plugins")
        end
    end)
HEADERS       = mainwindow.h
 SOURCES       = main.cpp \
                 mainwindow.cpp
 RESOURCES     = application.qrc

 # install
 target.path = $$[QT_INSTALL_EXAMPLES]/mainwindows/application
 sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS application.pro images
 sources.path = $$[QT_INSTALL_EXAMPLES]/mainwindows/application
 INSTALLS += target sources

 symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
 maemo5: include($$QT_SOURCE_TREE/examples/maemo5pkgrules.pri)

 symbian: warning(This example might not fully work on Symbian platform)
 simulator: warning(This example might not fully work on Simulator platform)

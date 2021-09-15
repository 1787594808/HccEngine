cmake_minimum_required (VERSION 3.1.0)
#project(demo)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
#set(CMAKE_PREFIX_PATH "/opt/Qt5.9.0/5.9/gcc_64/lib/cmake")
#打开全局moc
set(CMAKE_AUTOMOC ON)
#打开全局uic
set(CMAKE_AUTOUIC ON)
#打开全局rcc
set(CMAKE_AUTORCC ON)


find_package(Qt5 REQUIRED Core)
find_package(Qt5 REQUIRED Gui)
find_package(Qt5 REQUIRED Xml)
find_package(Qt5 REQUIRED Sql)
find_package(Qt5 REQUIRED Widgets)
find_package(OpenCV 3.0 QUIET)
if(NOT OpenCV_FOUND)
    find_package(OpenCV 2.4.3 QUIET)
    if(NOT OpenCV_FOUND)
        message(FATAL_ERROR "OpenCV > 2.4.3 not found.")
    endif()
endif()
set(IBUS_VER 1.3)
find_package(PkgConfig REQUIRED)


PKG_CHECK_MODULES(IBUS REQUIRED ibus-1.0>=${IBUS_VER})

set(TENSORFLOW_INCLUDES
	/usr/local/include/tensorflow/
)

set(JSON_INCLUDES
       ../include/)

set(TENSORFLOW_LIBS
#        /usr/local/lib/libtensorflow_cc.so
#        /usr/local/lib/libtensorflow_framework.so
	/usr/local/lib/libtensorflow.so
	/usr/local/lib/libtensorflow_framework.so
#	/lib/libtensorflow.so
#	/lib/libtensorflow_framework.so
)

#add headers mulu
include_directories(
        ${TENSORFLOW_INCLUDES}
        ${JSON_INCLUDES}
	${IBUS_INCLUDE_DIRS}
	./libgooglepinyin/
	./libgooglepinyin/include/
)

SET(Xyinput_SOURCES
    main.cpp
    getHan.cpp
    tf_utils.cpp
    engine.cpp
    drawwidget.cpp
    xydragablewidget.cpp
    xyhdragabletranslateview.cpp
    xykeyboardfilter.cpp
    xypushbutton.cpp
    xyqstringview.cpp
    xyskin.cpp
    xytempwindows.cpp
    xyvdragabletranslateview.cpp
    xyvirtualkeyboard.cpp
    chineseInput/xydatabaseoperation.cpp
    chineseInput/xyinputsearchinterface.cpp
    chineseInput/xytranslateitem.cpp
    libgooglepinyin/src/dictbuilder.cpp
    libgooglepinyin/src/dictlist.cpp
    libgooglepinyin/src/dicttrie.cpp
    libgooglepinyin/src/lpicache.cpp
    libgooglepinyin/src/matrixsearch.cpp
    libgooglepinyin/src/mystdlib.cpp
    libgooglepinyin/src/ngram.cpp
    libgooglepinyin/src/pinyinime.cpp
    libgooglepinyin/src/searchutility.cpp
    libgooglepinyin/src/spellingtable.cpp
    libgooglepinyin/src/spellingtrie.cpp
    libgooglepinyin/src/splparser.cpp
    libgooglepinyin/src/sync.cpp
    libgooglepinyin/src/userdict.cpp
    libgooglepinyin/src/utf16char.cpp
    libgooglepinyin/src/utf16reader.cpp
    libgooglepinyin/xygooglepinyin.cpp
)

SET(Xyinput_HEADERS
    engine.h
    getHan.h
    tf_utils.hpp
    drawwidget.h
    xydragablewidget.h
    xyhdragabletranslateview.h
    xykeyboardfilter.h
    xypushbutton.h
    xyqstringview.h
    xyskin.h
    xytempwindows.h
    xyvdragabletranslateview.h
    xyvirtualkeyboard.h
    chineseInput/xydatabaseoperation.h
    chineseInput/xyinputsearchinterface.h
    chineseInput/xytranslateitem.h
    libgooglepinyin/include/atomdictbase.h
    libgooglepinyin/include/dictbuilder.h
    libgooglepinyin/include/dictdef.h
    libgooglepinyin/include/dictlist.h
    libgooglepinyin/include/dicttrie.h
    libgooglepinyin/include/lpicache.h
    libgooglepinyin/include/matrixsearch.h
    libgooglepinyin/include/mystdlib.h
    libgooglepinyin/include/ngram.h
    libgooglepinyin/include/pinyinime.h
    libgooglepinyin/include/searchutility.h
    libgooglepinyin/include/spellingtable.h
    libgooglepinyin/include/spellingtrie.h
    libgooglepinyin/include/splparser.h
    libgooglepinyin/include/sync.h
    libgooglepinyin/include/userdict.h
    libgooglepinyin/include/utf16char.h
    libgooglepinyin/include/utf16reader.h
    libgooglepinyin/xygooglepinyin.h
)

#添加资源文件
SET(Xyinput_RESOURCES
    images/images.qrc
    data/files.qrc
)

configure_file(myengine.xml.in myengine.xml)
install(FILES myengine.xml DESTINATION /usr/share/ibus/component)

set(engine_executable_name ibus-engine-myengine)
add_executable(${engine_executable_name} ${Xyinput_SOURCES} ${Xyinput_HEADERS} ${Xyinput_RESOURCES})


target_link_libraries(${engine_executable_name} ${IBUS_LIBRARIES} ${OpenCV_LIBS} ${TENSORFLOW_LIBS} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Xml Qt5::Sql)

install(TARGETS ${engine_executable_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

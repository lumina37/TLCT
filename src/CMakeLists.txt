set(TLCT_INCLUDE_DIR
        ${OpenCV_INCLUDE_DIR}
        "${pugixml_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src/include"
        PARENT_SCOPE)

set(TLCT_LINK_LIBS
        ${OpenCV_LIBS}
        pugixml
        PARENT_SCOPE)

add_subdirectory(bin)
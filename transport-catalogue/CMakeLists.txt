cmake_minimum_required(VERSION 3.5)

project(transport-catalogue_poligon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

# Команда вызова protoc.
# Ей переданы названия переменных, в которые будут сохранены
# списки сгенерированных файлов, а также сам proto-файл.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
    svg.proto
    graph.proto
    map_renderer.proto
    transport_router.proto
    transport_catalogue.proto
    )

set(CORE_FILES
    main.cpp
    libs/json.cpp
    libs/json.h
    transport_catalogue.cpp
    transport_catalogue.h
    libs/geo.h
    libs/svg.h
    libs/svg.cpp
    json_reader.h
    json_reader.cpp
    map_renderer.h
    map_renderer.cpp
    request_handler.h
    request_handler.cpp
    domain.h
    domain.cpp
    json_builder.h
    json_builder.cpp
    graph.h
    ranges.h
    router.h
    transport_router.h
    transport_router.cpp
    serialization.h
    serialization.cpp
    )

add_executable(transport_catalogue
    ${CORE_FILES}
    ${PROTO_SRCS}
    ${PROTO_HDRS})

# find_package определила переменную Protobuf_INCLUDE_DIRS,
# которую нужно использовать как include-путь.
# Также нужно добавить как include-путь директорию, куда
# protoc положит сгенерированные файлы.
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Также find_package определила Protobuf_LIBRARY.
# Protobuf зависит от библиотеки Threads. Добавим и её при компоновке.
target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads)
string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

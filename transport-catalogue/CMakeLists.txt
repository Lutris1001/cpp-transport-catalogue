cmake_minimum_required(VERSION 3.11)

project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)


protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
        map_renderer.proto
        graph.proto
        svg.proto
        transport_router.proto
        transport_catalogue.proto
        )

set(TRANSPORT_CATALOGUE_FILES
        main.cpp
        domain.h
        domain.cpp
        geo.cpp
        geo.h
        json.cpp
        json.h
        json_reader.cpp
        json_reader.h
        json_builder.cpp
        json_builder.h
        map_renderer.cpp
        map_renderer.h
        request_handler.cpp
        request_handler.h
        svg.cpp
        svg.h
        transport_catalogue.cpp
        transport_catalogue.h
        graph.h
        ranges.h
        router.h
        transport_router.h
        transport_router.cpp
        serialization.h
        serialization.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
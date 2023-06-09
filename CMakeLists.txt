cmake_minimum_required(VERSION 3.15)

project(ssh_project)

set(CMAKE_C_STANDARD 11 REQUIRED)

add_executable(client client/main.c client/send_spawn.c)
add_executable(server server/main.c server/listener.c server/spawn.c)
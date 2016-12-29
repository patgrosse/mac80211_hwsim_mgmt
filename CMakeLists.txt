cmake_minimum_required(VERSION 3.6)
project(mac80211_hwsim_mgmt)

set(CMAKE_CXX_STANDARD 11)

# find libraries
include(${CMAKE_SOURCE_DIR}/cmake/FindNetLink.cmake)

# include libnl directories
include_directories(${NL_INCLUDE_DIRS})

# set source files
set(SOURCE_FILES hwsim_mgmt_cli.c hwsim_mgmt_cli.h hwsim_mgmt_func.c hwsim_mgmt_func.h)

# add executables
add_executable(mac80211_hwsim_mgmt ${SOURCE_FILES})

# link required libraries
target_link_libraries(mac80211_hwsim_mgmt ${M_LIB})
target_link_libraries(mac80211_hwsim_mgmt ${NL_LIBRARIES})
target_link_libraries(mac80211_hwsim_mgmt ${LIBEVENT_LIB})
target_link_libraries(mac80211_hwsim_mgmt ${CMAKE_THREAD_LIBS_INIT})
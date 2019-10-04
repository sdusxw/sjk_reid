include_directories(
	${SOLUTION_INCLUDE_DIRS}
    /usr/local/include
)
link_directories(
    ${src_dir}/lib
    /usr/local/lib
)

add_library(lpr_alg
	rv_anpr_interface.h
	lpr_alg.h
	lpr_alg.cpp
)

add_executable(lpr_test
	lpr_alg.h
	lpr_test.cpp
)

target_link_libraries(lpr_alg pthread common jpeg yuv vanprhardv3)

target_link_libraries(lpr_test pthread common jpeg yuv)


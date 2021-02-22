cmake_minimum_required(VERSION 3.0)

project(ck C)

include_directories(../ck/include)

set(ck_src
	"../ck/src/ck_array.c"
	"../ck/src/ck_barrier_centralized.c"
	"../ck/src/ck_barrier_combining.c"
	"../ck/src/ck_barrier_dissemination.c"
	"../ck/src/ck_barrier_mcs.c"
	"../ck/src/ck_barrier_tournament.c"
	"../ck/src/ck_epoch.c"
	"../ck/src/ck_hp.c"
	"../ck/src/ck_hs.c"
	"../ck/src/ck_ht.c"
	"../ck/src/ck_rhs.c"
)

add_library(ck STATIC ${ck_src})

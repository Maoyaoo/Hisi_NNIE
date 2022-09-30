cmake_minimum_required(VERSION 3.6)


SET(CMAKE_CXX_STANDARD 11)

#ADD_DEFINITIONS(-D __DEBUG__)  # is Debug?


#告知当前使用的是交叉编译方式，必须配置
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm)


# =============== COMPILER ========================
#设置交叉编译工具链，一定要设置,方式1：交叉编译器使用绝对地址，方式2：指定编译器名字
#方式1：
SET(TOOLCHAIN_DIR /opt/hisi-linux/x86-arm/arm-himix200-linux)
SET(CMAKE_C_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-gcc)
SET(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-g++)
SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_DIR}
 ${TOOLCHAIN_DIR}/arm-himix200-linux/include
 ${TOOLCHAIN_DIR}/arm-himix200-linux/lib )
 
#方式2：（不建议？）
#SET(CMAKE_C_COMPILER arm-himix200-linux-gcc)     #指定C交叉编译器
#SET(CMAKE_CXX_COMPILER arm-himix200-linux-g++)   #指定C++交叉编译器
#SET(CMAKE_C_STRIP arm-himix200-linux-strip)
# =======================================================

# ==================== FLAGS ============================
# search for programs in the build host directories (not necessary)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
#海思编译的CFLAGS
SET(CMAKE_C_FLAGS "-Wall -g -Dhi3516cv500 -DHI_XXXX -lpthread -lm -ldl -DISP_V2 -lstdc++ -Wall -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations -ldl -ffunction-sections -fdata-sections -O2 -fstack-protector-strong -fPIC -Wall -DHI_RELEASE -Wno-error=implicit-function-declaration -DVER_X=1 -DVER_Y=0 -DVER_Z=0 -DVER_P=0 -DVER_B=10 -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time -DSENSOR0_TYPE=GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT -DSENSOR1_TYPE=GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT -DHI_ACODEC_TYPE_INNER -DHI_ACODEC_TYPE_HDMI")
SET(CMAKE_C_FLAGS_DEBUG "-Wall -ggdb3 -DNM_DEBUG ${CMAKE_C_FLAGS}")
SET(CMAKE_C_FLAGS_RELEASE  "-Wall -O3  ${CMAKE_C_FLAGS}")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)
# ===========================================

# ==================THIRD PARYT==============
SET(THIRD_PART ${CMAKE_SOURCE_DIR}/third_party)
MESSAGE("###Third_part is: ${THIRD_PART}")

# Hisi
SET(HI_ROOT ${THIRD_PART}/hisi)
SET(HI_INCLUDE ${HI_ROOT}/include)
SET(HI_LIB_DIR ${HI_ROOT}/lib)
#尝试用set设置海思库
SET(HI_LIBS 
	${HI_LIB_DIR}/libmpi.a 
	${HI_LIB_DIR}/libhdmi.a
	${HI_LIB_DIR}/lib_hiae.a 
	${HI_LIB_DIR}/libisp.a 
	${HI_LIB_DIR}/lib_hidehaze.a 
	${HI_LIB_DIR}/lib_hidrc.a 
	${HI_LIB_DIR}/lib_hildci.a 
	${HI_LIB_DIR}/lib_hicalcflicker.a 
	${HI_LIB_DIR}/lib_hiawb.a 
	${HI_LIB_DIR}/libsns_imx327.a 
	${HI_LIB_DIR}/libsns_imx327_2l.a 
	${HI_LIB_DIR}/libsns_imx307.a 
	${HI_LIB_DIR}/libsns_imx307_2l.a 
	${HI_LIB_DIR}/libsns_imx458.a 
	${HI_LIB_DIR}/libsns_mn34220.a 
	${HI_LIB_DIR}/libsns_os05a.a 
	${HI_LIB_DIR}/libsns_os08a10.a 
	${HI_LIB_DIR}/libsns_gc2053.a 
	${HI_LIB_DIR}/libsns_sc4210.a 
	${HI_LIB_DIR}/libsns_ov12870.a 
	${HI_LIB_DIR}/libsns_os04b10.a 
	${HI_LIB_DIR}/libsns_imx415.a
	${HI_LIB_DIR}/libsns_imx219.a
	${HI_LIB_DIR}/libsns_imx274.a
	${HI_LIB_DIR}/libsns_imx335.a
	${HI_LIB_DIR}/libVoiceEngine.a 
	${HI_LIB_DIR}/libupvqe.a 
	${HI_LIB_DIR}/libdnvqe.a
	${HI_LIB_DIR}/libsecurec.a
	${HI_LIB_DIR}/libtde.a
	${HI_LIB_DIR}/libive.a
	${HI_LIB_DIR}/libmd.a
	${HI_LIB_DIR}/libnnie.a
)


#Opencv
SET(OPENCV_ROOT ${THIRD_PART}/opencv_3.4.1)
SET(OPENCV_INCLUDE ${OPENCV_ROOT}/include)
SET(OPENCV_LIB_DIR ${OPENCV_ROOT}/lib)
FILE(GLOB OPENCV_LIBS "${OPENCV_LIB_DIR}/*")




INCLUDE_DIRECTORIES(include ${HI_INCLUDE} ${OPENCV_INCLUDE})
LINK_DIRECTORIES(${OPENCV_LIB_DIR} ${HI_LIB_DIR})

# ===========================================

project(NNIE-lite)

ADD_SUBDIRECTORY(src/hi_common)
ADD_SUBDIRECTORY(src/hi_svp)
ADD_SUBDIRECTORY(src/cv_kalman)
ADD_SUBDIRECTORY(main)




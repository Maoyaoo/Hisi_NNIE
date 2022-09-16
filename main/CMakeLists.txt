PROJECT(wk_mian)
# project(NNIE-lite)



# For YOLOv3
ADD_EXECUTABLE(yolov3_Exe ./nnie_yolov3_main.c)


LINK_DIRECTORIES(${HI_LIB_DIR})

TARGET_LINK_LIBRARIES(yolov3_Exe pthread dl m
    hi_common
    hi_svp
)

TARGET_LINK_LIBRARIES(yolov3_Exe pthread dl m
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
    ${HI_LIB_DIR}/libnnie.a)




# For KCF
ADD_EXECUTABLE(kcf_Exe ./ive_kcf_main.c)


LINK_DIRECTORIES(${HI_LIB_DIR})

TARGET_LINK_LIBRARIES(kcf_Exe pthread dl m
    hi_common
    hi_svp
)

TARGET_LINK_LIBRARIES(kcf_Exe pthread dl m
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
    ${HI_LIB_DIR}/libnnie.a)
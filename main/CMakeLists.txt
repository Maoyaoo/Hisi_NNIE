PROJECT(wk_mian)


# For YOLOv3
ADD_EXECUTABLE(yolov3_Exe ./nnie_yolov3_main.c)

TARGET_LINK_LIBRARIES(yolov3_Exe pthread dl m
    hi_common
    hi_svp
)

TARGET_LINK_LIBRARIES(yolov3_Exe pthread dl m
	${HI_LIBS}
)


# For KCF
ADD_EXECUTABLE(kcf_Exe ./ive_kcf_main.c)

TARGET_LINK_LIBRARIES(kcf_Exe pthread dl m
    hi_common
    hi_svp
)

TARGET_LINK_LIBRARIES(kcf_Exe pthread dl m
	${HI_LIBS}
)


# For KF YOLOv3
ADD_EXECUTABLE(kf_yolov3_Exe ./kf_yolov3_main.c)

TARGET_LINK_LIBRARIES(kf_yolov3_Exe pthread dl m
    hi_common
    hi_svp
	cv_kalman
)

TARGET_LINK_LIBRARIES(kf_yolov3_Exe pthread dl m
	${HI_LIBS}
	${OPENCV_LIBS}
)



# For SOT
SET(SOURCES
    ./sot_main.c
    ./sot_thread.c
)

ADD_EXECUTABLE(sot_Exe ${SOURCES})

target_include_directories(sot_Exe
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)


TARGET_LINK_LIBRARIES(sot_Exe pthread dl m
    hi_common
    hi_svp
	cv_kalman
)

TARGET_LINK_LIBRARIES(sot_Exe pthread dl m
	${HI_LIBS}
	${OPENCV_LIBS}
)
# Set the project name
PROJECT(cv_kalman)


#Add a library with the avbove sources
AUX_SOURCE_DIRECTORY(./src CV_KALMAN_SRC)
ADD_LIBRARY(${PROJECT_NAME} STATIC ${CV_KALMAN_SRC})



TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)


TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    hi_common
    hi_svp
)
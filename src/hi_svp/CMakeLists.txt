# Set the project name
PROJECT(hi_svp)


#Add a library with the avbove sources
AUX_SOURCE_DIRECTORY(./src HI_SVP_SRC)
ADD_LIBRARY(${PROJECT_NAME} STATIC ${HI_SVP_SRC})



TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
    hi_common
)

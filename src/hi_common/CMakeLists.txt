# Set the project name
PROJECT(hi_common)


#Add a library with the avbove sources
AUX_SOURCE_DIRECTORY(./src HI_COMMON_SRC)
MESSAGE("######project name: ${PROJECT_NAME}")
MESSAGE("######HI_COMMON_SRC: ${HI_COMMON_SRC}")
ADD_LIBRARY(${PROJECT_NAME} STATIC ${HI_COMMON_SRC})



TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
include(CTest)

find_package(GTest REQUIRED)

# Ajoutez de l'exécutable des tests
add_executable (${PROJECT_NAME}-tests "${PROJECT_NAME}-tests.cpp")

target_link_libraries(${PROJECT_NAME}-tests PRIVATE GTest::gtest GTest::gtest_main ${PROJECT_NAME})

#add_test(NAME ${PROJECT_NAME}-gtest COMMAND ${PROJECT_NAME}-tests)

gtest_discover_tests(${PROJECT_NAME}-tests)

if (UNIX)
	install(TARGETS ${PROJECT_NAME}-tests DESTINATION ${PROJECT_SOURCE_DIR}/bin CONFIGURATIONS Debug)
endif()

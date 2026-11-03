cmake_minimum_required(VERSION 3.5)
project(csms_cpp_test CXX)

add_executable(${PROJECT_NAME} 
    test_main.cc
    ResidentTests.cc
    ResidentValidatorTests.cc
    ResidentRepositoryTests.cc
    ResidentRegistrationServiceTests.cc
    ResidentQueryServiceTests.cc
    ResidentUpdateServiceTests.cc
    ResidentDeactivationServiceTests.cc
    ServiceRequestTests.cc
    ServiceRequestSubmissionServiceTests.cc
    ServiceRequestStatusServiceTests.cc
    ServiceRequestQueryServiceTests.cc
    ResidentUiTests.cc
    HttpTestSupport.cc
    ../models/Resident.cc
    ../models/ResidentValidator.cc
    ../database/Database.cc
    ../repositories/ResidentRepository.cc
    ../services/ResidentRegistrationService.cc
    ../services/ResidentQueryService.cc
    ../services/ResidentUpdateService.cc
    ../services/ResidentDeactivationService.cc
    ../models/ServiceRequest.cc
    ../models/ServiceRequestValidator.cc
    ../repositories/ServiceRequestRepository.cc
    ../services/ServiceRequestSubmissionService.cc
    ../services/ServiceRequestStatusService.cc
    ../services/ServiceRequestQueryService.cc
    ../app/AppContext.cc
    ../controllers/ResidentController.cc
)

# Repo root, so generated views can include "models/Resident.h" like the app does.
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
target_link_libraries(${PROJECT_NAME} PRIVATE
    Drogon::Drogon
    unofficial::sqlite3::sqlite3
)

# The controller renders .csp views, so the test executable needs its own copy
# of the generated view code.
drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../views
                    ${CMAKE_CURRENT_BINARY_DIR})

ParseAndAddDrogonTests(${PROJECT_NAME})
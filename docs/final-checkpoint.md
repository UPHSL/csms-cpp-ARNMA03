# CSMS Final Development Checkpoint (T16)

## 1. Developer Information

- **Name:** Arden Roland Nicholai M. Austria
- **GitHub Username:** ARNMA03
- **Technology Stack:** C++20, Drogon web framework, SQLite database, CMake, DROGON_TEST
- **T16 Branch:** feature/t16-final-release
- **Final Release Version:** 1.0.0

## 2. My Final Release Implementation

I put the release version in one small file, app/AppInfo.h. It stores the system name and the version number 1.0.0, and nothing else in the project keeps its own copy. I chose this place because the project already uses a similar small file for shared values, and both the main application and the tests can use it without extra setup. The /health page reads the name and version from this file, so it reports CSMS, version 1.0.0 and a status of OK, and it shows nothing private. In the browser, the footer shared by every page reads the version from the same file, so all pages show v1.0.0. The home page used to have the old starter version 0.1.0 typed into it, so I changed it to read from the same file too. Because of this, changing the version later means editing just one line. This matches how the project is already organized: controllers handle the pages, views handle what users see, and shared values live in one simple file.

## 3. Files I Changed

- app/AppInfo.h (new): the one place where the system name and version are stored.
- controllers/HealthController.cc: /health now shows CSMS, version 1.0.0 and OK instead of the old starter version.
- controllers/HealthController.h: updated so it matches the new health page code.
- controllers/HomeController.cc: the home page now shows the version from AppInfo.h instead of the old 0.1.0.
- views/LayoutFooter.csp: shows the version at the bottom of every page.
- views/LayoutHeader.csp: a small style rule so the footer looks neat.
- test/FinalReleaseTests.cc (new): the tests that check the version and the final integration test.
- test/CMakeLists.txt: adds the new tests and the health and home page code to the test program.
- docs/release-v1.0.0.md (new): the official release notes.
- docs/final-checkpoint.md (new): this document.

## 4. My Final Examination Test

**Test Name:** ResidentLifecycleRequestWorkflowAndDashboardAgreeWithPersistedData

**Modules/Responsibilities Covered:** Resident registration, Resident update, Resident deactivation, Service Request submission and Resident eligibility, controlled Service Request workflow, Service Request search and status filtering, Dashboard aggregation, and real SQLite persistence.

**Starting State:** A brand new, empty database with a Dashboard that shows zero Residents and zero Service Requests.

**Operations Performed:**
1. Register two Residents (Ana Reyes and Ben Cruz).
2. Update Ana's address.
3. Submit three Service Requests (two for Ana, one for Ben) while both Residents are Active.
4. Move Ana's first request from Pending to In Progress to Completed, move her second request to In Progress, and cancel Ben's request.
5. Deactivate Ben after his request exists.
6. Try actions that should be rejected: a request for the Inactive Resident, a request for a Resident that does not exist, three invalid status changes, and a Resident update with an invalid contact number.
7. Check the Dashboard and run a search combined with a status filter.

**Expected Final State:** Two Residents (Ana is Active with her new address, Ben is Inactive) and three Service Requests (one Completed, one In Progress, one Cancelled). The Dashboard shows 2 total, 1 active and 1 inactive Residents, and 3 total, 1 open, 0 pending, 1 in progress, 1 completed and 1 cancelled Service Requests. The saved data is exactly the same before and after the rejected actions. When the data is read again through a separate connection to the database, it shows the same Residents and requests.

**Why This Test Is Meaningful:** A simple test only checks one small part on its own. This test goes through the real pages and actions of the system, from the screens down to the saved data, and checks that every part still agrees with the others after a realistic series of steps. It also compares all saved data before and after the rejected actions, which proves that a rejected action does not save anything by accident. Reading the data again through a separate connection shows it was truly saved and not just remembered temporarily.

## 5. My Final Technical Decision

**Decision:** I stored the release version in one small file (app/AppInfo.h) and made /health, the footer and the home page all read from it.

**Alternative Considered:** Setting the version in the CMake build file and generating the file from it, or keeping the version in a configuration file.

**Reason for My Decision:** The project already uses a small file for shared values, ServiceRequestStatus.h, so this follows an approach I had already used. The tests are built as a separate program, and a small file works for both the application and the tests without extra setup. The config.json and config.yaml files are unused starter files that the application never loads, so using them would add a new way of doing things just for one value.

## 6. Problem or Verification Issue Encountered

**Problem:** My new test that checks the home page for the version failed. The home page came back as "not found" (error 404) instead of showing v1.0.0.

**Cause or Investigation:** The home page code, HomeController.cc, was not included in the test build, so the test program did not have a home page to show. Only the health page had been added. I found this by checking the list of files in test/CMakeLists.txt. I also ran into two smaller problems on the way. First, my test file did not compile at first because of a wrong way of writing an "or" condition inside the test check, and since the build failed, the test run quietly used the old test program, which made the early passing results misleading. Second, after I renamed a test, the test runner still looked for the old name because the list of tests is created when CMake is configured, so it said it could not find the test. From this I learned to read the build output for errors before running the tests, and to run the CMake configure step again after renaming a test.

**Resolution:** I added HomeController.cc to test/CMakeLists.txt, fixed the condition in the test, ran the CMake configure step again, rebuilt, and ran the tests. The home page test then passed, and the full suite passed with 183 tests.

## 7. Final Regression Results

- **Test Command:** ctest --test-dir build -C Debug --output-on-failure
- **Final Regression Run 1:** 100% tests passed, 0 tests failed out of 183
- **Final Regression Run 2:** 100% tests passed, 0 tests failed out of 183
- **Total Passing Tests Reported:** 183

## 8. Tools and References Used

- Existing CSMS repository documentation, including docs/t15-release-readiness.md.
- Git and GitHub documentation and workflow (branching, pull request, annotated tag, GitHub Release).
- Claude (AI assistant by Anthropic): used to plan the T16 steps and to help draft the release-version code, the tests and the documentation.

## 9. Final Release Declaration

CSMS v1.0.0 - READY FOR RELEASE
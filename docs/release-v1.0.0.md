# CSMS v1.0.0 Release Notes

## 1. Release

- **Application:** Community Services Management System
- **Release:** v1.0.0
- **Release Type:** Final Course Release

## 2. Release Summary

CSMS v1.0.0 is a browser-based system for managing community services, built with C++, the Drogon web framework and a SQLite database. Staff can register and maintain Residents, accept Service Requests only from Active Residents, move each request through a controlled workflow, and watch the overall picture on a Dashboard. All data is saved in the database, so it is still there after the application restarts. The system reports its name and version on the /health page and shows the version at the bottom of every page, and both read the same single version value.

## 3. Resident Management Capabilities

- Register a Resident with first name, last name, address, contact number and email. The contact number must be 11 digits starting with 09, and the email must be in a valid format.
- List all Residents, sorted by last name and then first name.
- Search Residents by first or last name. The search ignores capital letters and matches part of a name.
- Update a Resident's name, address, contact number and email. The Resident ID and status cannot be changed from the form.
- Deactivate a Resident. This changes the status from Active to Inactive and does not delete anything.
- Inactive Residents stay visible and searchable, and they cannot submit new Service Requests.

## 4. Service Request Management Capabilities

- Submit a Service Request with a Resident ID, service type, description and date requested. Every new request starts as Pending.
- A request is rejected if the Resident does not exist or is Inactive.
- List all requests, newest date first.
- Search by service type or description, filter by status, or use both together.
- Open a details page for any request.
- Controlled workflow: Pending can move to In Progress or Cancelled, and In Progress can move to Completed or Cancelled.
- Completed and Cancelled are final. Any other change, such as going from Pending straight to Completed, is rejected and nothing is saved.
- A request that already exists can keep moving through its workflow even if its Resident is deactivated later.

## 5. Dashboard Capabilities

- Resident totals: total, active and inactive Residents.
- Service Request totals: total, pending, in progress, completed and cancelled.
- Open Service Requests, which means the pending and in progress requests together.
- The five most recent Service Requests, each with a link to its details page.
- The Dashboard only shows information. It has no buttons that change data.

## 6. Quality and Hardening

In T15, the system was checked for bad input, for rejected actions that must not change saved data, for pages that only read data, and for edge cases such as the recent requests list at exactly five and six items. The T15 result was 180 passing tests.

In T16, I added one place for the release version (app/AppInfo.h) that /health, the home page and the page footer all use. I added tests that check /health reports CSMS, version 1.0.0 and OK, and that the home page and the main pages show v1.0.0. I also added a Final Examination test that registers Residents, submits and moves Service Requests, deactivates a Resident, tries actions that must be rejected, and then checks the Dashboard and the saved data, including through a second database connection. The full suite now has 183 tests.

## 7. Final Verification

- **Final Regression Run 1:** PASS (100% tests passed, 0 tests failed out of 183)
- **Final Regression Run 2:** PASS (100% tests passed, 0 tests failed out of 183)
- **Application Startup:** PASS
- **/health:** PASS
- **Dashboard:** PASS
- **Resident Management:** PASS
- **Service Request Management:** PASS
- **Final Release Version:** PASS (1.0.0)

## 8. Known Limitations

These were identified during T15 and still apply:

- The Date Requested field accepts any text that is not blank, so the date format is not checked.
- SQLite foreign key checking is not turned on, so the link between a request and its Resident is enforced by the application and not by the database.
- The ports 8080 (application) and 18080 (tests) are set in the code and cannot be changed without editing it.
- The config.json and config.yaml files are unused starter files. The application does not read them.
- The docs/release-workflow.md file is empty.

None of these block the release within the defined CSMS v1.0.0 scope.
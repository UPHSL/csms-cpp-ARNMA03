# T15 Release Readiness

## 1. Developer Information

- **Name:** Arden Roland Nicholai M. Austria
- **GitHub Username:** ARNMA03 
- **Technology Stack:** C++20, Drogon (HTTP framework and CSP views), SQLite3 (C API), CMake, CTest / `DROGON_TEST`
- **T15 Branch:** `feature/t15-system-hardening`

## 2. Release Candidate Baseline

Baseline taken on clean `main` (T14 merged) before any T15 change.

| Item | Before T15 |
|---|---|
| Application startup | Working |
| `/health` | Working (`{"application":"Community Services Management System","status":"ok","version":"0.1.0"}`) |
| Dashboard | Working  |
| Resident Management | Working |
| Service Request Management | Working |
| Starter through T14 automated tests | 100% passed, 175/175 |

No pre-existing failures were found. One observation: the home page (`/`) had no links to the other modules (see Section 3).

## 3. Hardening Changes

| Area | Issue or Risk Found | Action Taken | Files Changed | Test/Evidence |
|---|---|---|---|---|
| Interface and navigation | The home page (`/`) had no links to Dashboard, Residents or Service Requests, and still showed "Starter application" and "Sprint 0" text. | Added three navigation links and updated the stale text. | `controllers/HomeController.cc` | Manual Scenario 12: reached all three modules from `/` by clicking. |
| Unexpected errors | Only `DashboardController` caught exceptions. A repository exception in other routes depended on framework defaults, and a failed database open at startup ended in an uncaught exception. | Added a central exception handler that returns a generic 500 message and logs details server-side only; added a startup guard that prints one clear error line. | `main.cc` | Code review; application starts and all pages load normally. |
| Resource safety | `Database` owns a raw `sqlite3*` but was copyable, so a copy would close the same connection twice. | Deleted the copy constructor and copy assignment. | `database/Database.h` | Compiler confirms nothing copies `Database`; full suite passes. |
| Duplicated business rule | The list of four supported Service Request statuses was defined separately in `ServiceRequestStatusService` and `ServiceRequestQueryService`. | Moved the rule into one shared header used by both; transition rules unchanged. | `models/ServiceRequestStatus.h`, `services/ServiceRequestStatusService.cc`, `services/ServiceRequestQueryService.cc` | T10 unsupported-status and T11 unsupported-filter tests still pass. |
| Invalid and missing input | Reviewed route identifiers, form values and missing records. | Reviewed - no defect identified. IDs are parsed as digits only (1-9 characters, greater than zero); anything else gives the controlled not-found or validation response. | none | `ResidentUiMissingOrMalformedResidentReturnsNotFound`, `ServiceRequestUiMissingRequestViewIsHandledSafely`, new test 1 |
| Persistence integrity | Reviewed update, deactivation, submission and status paths. | Reviewed - no defect identified. Validation runs before any write; Resident update is one SQL `UPDATE`, so no partial update occurs. | none | New test 2 compares full database snapshots after rejected operations. |
| HTTP and mutation safety | Reviewed all T12-T14 routes. | Reviewed - no defect identified. All mutations are POST routes and answer with 303 redirects (Post/Redirect/Get). | none | `...GetRequestsDoNot...` tests, new test 3 |
| Query and database safety | Reviewed all SQL in both repositories. | Reviewed - no defect identified. All user-supplied values use `sqlite3_bind_*`; LIKE wildcards are escaped; Dashboard uses `COUNT(*)` and `LIMIT`. | none | Code review; T05 and T11 search tests |
| Workflow and lifecycle | Reviewed Resident and Service Request lifecycles. | Reviewed - no defect identified. Transition rules are enforced in `ServiceRequestStatusService`, independent of the UI. | none | T10 tests, `ServiceRequestUiInvalidDirectTransitionIsStillRejected`, new tests 2 and 4 |
| Test reliability | Reviewed isolation and leftovers. | Reviewed - no defect identified. Each test uses its own database file and cleans up; no `.db` files remained after the suite. | none | Two consecutive full runs passed (Section 5). |
| Template output safety | Reviewed all views. | Reviewed - no defect identified. User text goes through `htmlTranslate`; no raw HTML output of user input. | none | `ResidentUiInvalidRegistrationShowsErrorsAndKeepsInput` |

## 4. Five New T15 Hardening Tests

All in `test/HardeningTests.cc`. Shared helper `databaseSnapshot()` added to `test/HttpTestSupport.{h,cc}`.

### Test 1: `ServiceRequestUiRejectsOutOfRangeAndSignedResidentIds`
- **Category:** 1 - Invalid or malformed input
- **What it verifies:** Resident IDs `9999999999`, `-5`, `1.5`, `+3` and `0` posted to the submission form each return 400 with the validation message, no database text, and leave the database unchanged.
- **Added value:** The existing test only covered a non-numeric ID (`abc`).

### Test 2: `RejectedOperationsLeaveWholeDatabaseSnapshotUnchanged`
- **Category:** 2 - Rejected mutation protects persistence
- **What it verifies:** A mixed valid/invalid Resident update (valid first name, invalid phone), a submission for an Inactive Resident, Pending to Completed, an unsupported status, and missing-record updates all fail, and a snapshot of both tables is identical afterwards.
- **Added value:** Older tests checked individual fields; this compares the whole database after every rejected operation, including the partial-update case.

### Test 3: `ReadOnlyGetsAcrossAllModulesDoNotMutatePersistence`
- **Category:** 3 - Read-only request safety
- **What it verifies:** Every page across Dashboard, Residents and Service Requests (list, search, filter, forms, details, error and 404 paths), repeated twice, leaves the snapshot unchanged. GET on action URLs does not run the action.
- **Added value:** Older read-only tests covered one module at a time and did not compare persistence across modules.

### Test 4: `DeactivatedResidentFlowStaysConsistentAcrossModules`
- **Category:** 4 - Cross-feature regression
- **What it verifies:** Through the real routes: deactivating a Resident updates Dashboard counts, new submissions are rejected, the existing request can still be started and completed with Dashboard counts following, and the Resident and request stay searchable.
- **Added value:** Combines T07, T09, T10, T11, T13 and T14 in one flow.

### Test 5: `DashboardRecentListBoundaryAtFiveAndSix`
- **Category:** 5 - Boundary behavior
- **What it verifies:** With exactly 5 requests all appear in Recent Service Requests; a 6th newest request removes only the oldest, which remains persisted and searchable.
- **Added value:** Existing tests used 6 or 7 records and never checked the exact boundary.

## 5. Full Regression Test Results

- **Test command used:** `ctest --test-dir build -C Debug --output-on-failure`
- **Run 1:** 100% tests passed, 0 failed out of 180 (67.60 sec)
- **Run 2:** 100% tests passed, 0 failed out of 180 (66.41 sec)
- **Total passing tests reported:** 180 (175 existing + 5 new)
- **Run from merged `main`:** <<FILL IN after merge>>

## 6. Manual Release Verification

| Verification | Result | Notes |
|---|---|---|
| Application starts | PASS | Started from the repo root with `.\build\Debug\csms_cpp.exe`. |
| /health works | PASS | Returned status `ok`. |
| Dashboard loads | PASS | |
| Resident listing/search works | PASS | |
| Valid Resident registration works | PASS | Appeared as Active with a generated ID. |
| Invalid Resident registration is rejected | PASS | Errors shown; record not added. |
| Resident update works | PASS | ID and status unchanged. |
| Resident deactivation works and record remains visible | PASS | Status Inactive; still listed and searchable. |
| Valid Service Request submission for Active Resident works | PASS | Created as Pending with the correct Resident ID. |
| Missing/Inactive Resident submission is rejected | PASS | No request created in either case. |
| Service Request search/filter/details work | PASS | Listing, text search, status filter, combined filter and details verified. |
| Valid Service Request workflow works | PASS | Pending, In Progress, Completed. |
| Invalid/terminal transition is rejected | PASS | Pending to Completed rejected; Completed and Cancelled offer no actions. |
| Dashboard reflects Resident/Service Request changes | PASS | Active/Inactive and status counts updated after changes. |
| Read-only GET operations do not modify persistence | PASS | Repeated reads changed nothing. |

Also verified manually: missing and malformed IDs (`/residents/99999/edit`, `/residents/abc/edit`, `/service-requests/99999`, `/service-requests/abc`) show a controlled not-found page, and navigation among Dashboard, Residents and Service Requests works from the home page and the top bar.

## 7. Repository Hygiene Review

| Check | Result |
|---|---|
| Debug output | `git grep` for `std::cout`, `printf`, `TODO`, `FIXME` found none in tracked source. The only console output added is the deliberate startup-failure message in `main.cc`. |
| Temporary files | None tracked (`git ls-files`). |
| Generated build/runtime artifacts | `build/`, `out/`, `.vs/`, `data/` are gitignored and untracked. |
| Accidental local database files | No `*.db` tracked; none left in the test output folder after the suite. |
| Commented-out abandoned code | None; the only comment-pattern match is an explanatory comment in `ResidentUpdateService.cc`. |
| Secrets/credentials | Searched for password, token, secret and key patterns. Only empty template fields in Drogon-generated config files; no real credentials or key files committed. |
| Broken navigation/links | Home page links added; all links verified by clicking (Scenario 12). |
| Direct SQL parameter binding | All repository queries bind user values with `sqlite3_bind_*`. |

**Cleanup performed:** none needed beyond the home page navigation fix.

## 8. Known Limitations

- Date Requested accepts any non-blank text; no T01-T14 requirement defines a date format.
- SQLite foreign key enforcement is not enabled; Resident existence and eligibility are enforced in `ServiceRequestSubmissionService`.
- The listener ports (8080 for the app, 18080 for tests) are hardcoded.
- `config.json` and `config.yaml` are the Drogon defaults and are not loaded by `main.cc`.
- The home page and `/health` have no automated tests (the test executable does not compile those controllers).
- `docs/release-workflow.md` is an empty placeholder file.

## 9. Release Candidate Statement

READY FOR T16 FINAL EXAM
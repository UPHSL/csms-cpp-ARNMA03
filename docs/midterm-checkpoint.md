# Midterm Checkpoint - T10: Manage Service Request Status

## 1. Developer Information

* **Name:** Austria, Arden Roland Nicholai M.
* **GitHub Username:** ARNMA03
* **Primary Technology Stack:** C++ with Drogon, SQLite3, Drogon test framework, and CMake
* **T10 Branch:** `feature/t10-service-request-status`

## 2. My T10 Implementation

The status workflow is managed by a new `ServiceRequestStatusService` in the services layer. It depends only on `ServiceRequestRepository`. Its `changeStatus(id, requestedStatus)` method returns a result struct containing an outcome (`Success`, `NotFound`, `UnsupportedStatus`, or `InvalidTransition`) and an optional `ServiceRequest`.

The service first retrieves the existing request using the repository's `findById` method. If the request is not found, it returns `NotFound` without creating anything. The current status is taken from the retrieved record, ensuring that every decision is based on the persisted state rather than information supplied by the caller.

Next, the service checks whether the requested status matches one of the four supported values. If it does not, the service returns `UnsupportedStatus`. It then uses a private `isAllowedTransition` function to check whether the requested status is allowed under the workflow rules. If the transition is not allowed, it returns `InvalidTransition`.

All checks happen before any database write, so rejected requests do not modify the database. Only a valid transition calls the repository's new `updateStatus` method. This method uses a parameterized `UPDATE service_requests SET status = ? WHERE id = ?` statement to change only the status column of the specified record. Finally, the service retrieves the request again using `findById` and returns the updated, persisted record in the result.

## 3. My Transition Rules

| Current Status | Allowed Next Status    |
| -------------- | ---------------------- |
| Pending        | In Progress, Cancelled |
| In Progress    | Completed, Cancelled   |
| Completed      | None                   |
| Cancelled      | None                   |

* **Pending to Completed is rejected because:** A request must first move to In Progress before it can be completed. The `isAllowedTransition` function only allows In Progress and Cancelled as the next statuses for Pending. Therefore, requesting Completed returns `InvalidTransition`, and the database remains unchanged.

* **Completed is terminal because:** It represents a finished workflow. The function does not allow any transitions from Completed, so every attempt to change it returns `InvalidTransition` and leaves the record unchanged.

* **Cancelled is terminal because:** Reopening cancelled requests is outside the scope of T10. Since no next statuses are allowed, every attempt to change a Cancelled request returns `InvalidTransition` and leaves the record unchanged.

* **Same-status requests are handled by:** None of the statuses list themselves as allowed targets. Therefore, Pending to Pending, In Progress to In Progress, Completed to Completed, and Cancelled to Cancelled all return `InvalidTransition` without changing the database.

The other outcomes are:

* **`Success`:** The transition is valid, the change is saved, and the updated request is returned.
* **`NotFound`:** No request exists with the given ID, and no record is created or changed.
* **`UnsupportedStatus`:** The requested value is not exactly Pending, In Progress, Completed, or Cancelled. For example, `Approved` or `in progress` returns this outcome, and the database remains unchanged.
* **`InvalidTransition`:** Both statuses are recognized, but the requested transition is not allowed. The database remains unchanged.

## 4. Files I Changed

**File:** `repositories/ServiceRequestRepository.h`

**Purpose:** Declares the new `updateStatus(int, const std::string&)` method.

**File:** `repositories/ServiceRequestRepository.cc`

**Purpose:** Implements `updateStatus` using a parameterized `UPDATE` statement that changes only the status of the request with the given ID and reports whether a row was changed.

**File:** `services/ServiceRequestStatusService.h`

**Purpose:** Declares the outcome enum, result struct, and `ServiceRequestStatusService` class.

**File:** `services/ServiceRequestStatusService.cc`

**Purpose:** Implements request lookup, not-found handling, supported-status validation, transition rules, and persistence of valid transitions.

**File:** `tests/ServiceRequestStatusServiceTests.cc`

**Purpose:** Contains the 13 required T10 scenarios and my student-designed test.

**File:** `tests/CMakeLists.txt`

**Purpose:** Adds the new service source file and test file to the test build.

**File:** `docs/midterm-checkpoint.md`

**Purpose:** Contains this midterm checkpoint document.

## 5. Problem I Encountered

**What happened:** When I tried to run the CSMS application to verify that it worked and check the `/health` endpoint, the application failed to start. It stopped immediately with this error:

```text
20261009 08:10:22.515000 UTC 4964 FATAL , Bind address failed at 127.0.0.1:8080 - Socket.cc:67
```

**What caused it:** The application was configured to listen on `127.0.0.1:8080`, but another program on my PC was already using port 8080. Since only one process can bind to the same address and port at a time, Drogon could not start its listener and exited.

**How I investigated it:** The error message identified `127.0.0.1:8080` as the address where binding failed. This pointed to a port conflict rather than an issue with my T10 code, since T10 does not modify the controllers or server configuration.

I ran the following command to check which process was using the port:

```powershell
netstat -ano | findstr :8080
```

The command showed that a process was listening on port 8080, along with its PID (`26732`). I then used the following command to identify the process:

```powershell
tasklist /FI "PID eq 26732"
```

The output identified the process as:

```text
Image Name          PID
steamwebhelper.exe  26732
```

This showed that `steamwebhelper.exe`, a process associated with Steam, was using port 8080 instead of another instance of my application.

**How I resolved it:** I closed Steam and ended its `steamwebhelper.exe` process, which released port 8080. I did not change any project files or the port configuration. After that, the application started normally on `127.0.0.1:8080`, and the `/health` endpoint returned the expected JSON response.

**A second, smaller problem:** Running `ctest --test-dir build --output-on-failure` did not run my tests properly on my Windows machine. The project uses a multi-configuration generator, which places the test executable inside a configuration folder such as `Debug`. Therefore, CTest needs to know which configuration to use.

I fixed this by running:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

After specifying the Debug configuration, all tests ran and passed.

## 6. My Student-Designed Test

**Test Name:** `ExistingServiceRequestKeepsProcessingAfterResidentIsDeactivated`

**What the Test Verifies:** After a Resident is deactivated, their existing Service Request can still move from Pending to In Progress and then to Completed. The request keeps the same `residentId`, while the Resident remains Inactive and their other information stays unchanged.

**Why I Added This Test:** T09 checks whether a Resident is Active when a new request is submitted, while T10 manages the status of an existing request. None of the 13 required T10 tests involve the Resident. I added this test to confirm that deactivating a Resident does not block existing requests and that T10 does not modify the Resident's information.

## 7. Tools and References Used

* Drogon documentation, particularly the test framework documentation
* SQLite C API documentation, particularly prepared statements and `sqlite3_changes`
* Visual Studio Code
* **Claude (AI assistant):** Used to plan the status workflow design and draft the repository method, service, automated tests, and this document. I then built, ran, and reviewed the implementation. I can explain the submitted code.
* **ChatGPT:** Used to fix grammar and improve the wording of my papers while keeping the original meaning of my work.

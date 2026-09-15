# Preliminary Examination Checkpoint

## Developer Information

**Name:** Austria, Arden Roland Nicholai 


**GitHub Username:** ARNMA03 


**Primary Technology Stack:** C++ with Drogon 


**T03 Branch:** feature/t03-resident-persistence 



## My T03 Implementation

My SQLite database is stored in the data/csms.db file. The Database class handles opening the SQLite database and creating the residents table when the application starts. The ResidentRepository class handles Resident persistence through the save() and findById() methods.

When save() is called, the repository prepares an INSERT statement and binds the Resident information to the SQL placeholders. SQLite generates the Resident ID using the INTEGER PRIMARY KEY AUTOINCREMENT column. After the insert succeeds, sqlite3_last_insert_rowid() is used to get the generated ID and return a Resident with that ID.

The findById() method prepares a SELECT statement, binds the Resident ID, and converts the returned database row into a Resident object. If the Resident does not exist, it returns std::nullopt.

## My Persistence Design Decision

One persistence-related design decision I made was storing the Resident contact number as TEXT instead of an integer. I chose this because contact numbers can contain a leading zero, such as 09171234567. Storing it as text keeps the original value when it is saved and retrieved from the database.

I considered using an integer because the contact number only contains digits, but I decided that TEXT was more appropriate because contact numbers are not used for mathematical calculations.

## My Database Resource Management

The SQLite database is opened in the Database constructor using sqlite3_open(). The database connection is closed in the Database destructor using sqlite3_close().

Prepared statements are created using sqlite3_prepare_v2() and finalized using sqlite3_finalize() after the database operation is completed. If an error occurs while preparing, binding, or executing a statement, the statement is finalized before an exception is thrown.

Database errors are reported using SQLite's error message and handled through std::runtime_error.

## My Resident Table Design

**Primary key design:** id INTEGER PRIMARY KEY AUTOINCREMENT

**Contact number column type:** TEXT NOT NULL

**Status column type:** TEXT NOT NULL

I selected INTEGER PRIMARY KEY AUTOINCREMENT so SQLite can generate the Resident ID instead of the application manually assigning IDs.

I selected TEXT for the contact number so values such as 09171234567 keep their leading zero. I also selected TEXT for the status because the existing Resident class represents the status as a string, such as Active.

## Files I Changed

**File:** database/Database.h

**Purpose:** Declares the Database class and its SQLite database connection and initialization methods.

**File:** database/Database.cc

**Purpose:** Opens and closes the SQLite database and creates the residents table.

**File:** repositories/ResidentRepository.h

**Purpose:** Declares the ResidentRepository class and its save() and findById() operations.

**File:** repositories/ResidentRepository.cc

**Purpose:** Implements Resident persistence using SQLite prepared statements, parameter binding, insertion, ID retrieval, and retrieval by ID.

**File:** test/test_main.cc

**Purpose:** Contains the automated tests for the Resident model, validation, and persistence.

**File:** test/CMakeLists.txt

**Purpose:** Configures the test executable and includes the required source files.

**File:** CMakeLists.txt

**Purpose:** Configures the project build and includes the database and repository source files.

**File:** main.cc

**Purpose:** Creates the data directory, initializes the file-backed SQLite database, and starts the Drogon application.

**File:** .gitignore

**Purpose:** Prevents generated SQLite database files such as .db, .sqlite, and .sqlite3 from being committed.

## SQL I Can Explain

```sql
INSERT INTO residents (
    first_name,
    last_name,
    address,
    contact_number,
    email,
    status
)
VALUES (?, ?, ?, ?, ?, ?);
```

This statement inserts a Resident into the residents table. The six placeholders represent the Resident's first name, last name, address, contact number, email, and status.

The ResidentRepository::save() method uses this statement. The values are safely bound using SQLite parameter-binding functions before the statement is executed. The ID is not included because SQLite generates it automatically.

## My Resident Mapping

A SQLite database row is converted into an instance of the existing Resident class inside ResidentRepository::findById().

The repository reads the values from the SQLite result and passes them to the existing Resident constructor.

The database uses snake_case column names while the C++ Resident class uses camelCase names. For example, the SQLite first_name column is mapped to the C++ firstName property. Similarly, contact_number is mapped to contactNumber.

This allows the repository to handle the difference between the database representation and the existing C++ model.


## Problem I Encountered

**Problem or error:** CMake could not initially find the SQLite3 package configuration.

**Cause:** CMake was not using the vcpkg toolchain file needed to locate the installed SQLite3 package.

**How I resolved it:** I configured CMake using the vcpkg toolchain file. After configuring with the correct toolchain, the project successfully generated the build files. I also learned that because the project uses the Visual Studio generator, CTest needs the -C Debug option to run the Debug test executable. I then ran ctest --test-dir build -C Debug --output-on-failure, which successfully ran all 21 tests.

After configuring CMake with the correct vcpkg toolchain, the project successfully configured and generated the build files.

I also encountered a CTest configuration issue where running CTest without specifying the Visual Studio configuration caused the tests to be marked as Not Run. I resolved this by running:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

This allowed all of the configured tests to run successfully.

## My Student-Designed Test

**Test name:** MultipleResidentsReceiveUniqueIds

**What it verifies:** It verifies that when two different residents are saved to the SQLite database, each resident receives a different generated ID.

**Why I chose this scenario:** I chose this test because the database uses an AUTOINCREMENT primary key for residents. The test confirms that SQLite generates unique identifiers for separate resident records and that the repository correctly returns those generated IDs.

## Tools and References Used

* **Visual Studio Code** — used to edit and manage the project files.
* **CMake** — used to configure and build the project.
* **MSVC** — used to compile the C++ project.
* **vcpkg** — used to manage the Drogon and SQLite3 dependencies.
* **SQLite3 documentation/reference** — used to understand SQLite database connections, prepared statements, parameter binding, statement execution, and generated IDs.
* **Drogon** — used as the project's C++ web framework.
* **CTest** — used to run the automated tests.
* **Git and GitHub** — used for version control and managing the project branches.
* **AI assistance / ChatGPT** — used as a coding assistant from Sprint 0 up to the current T03 ticket. I used AI to help me understand the requirements, C++ and SQLite concepts, troubleshoot errors, and review parts of the implementation. 

At the same time, I can explain the general purpose and concept of each file, including how the model, database, and repository together.

Check the compiler:

c++ --version

or:

clang++ --version

Check CMake:

cmake --version

Check Drogon:

drogon_ctl version
Clone the Repository

After accepting the GitHub Classroom assignment:

git clone <your-repository-url>

Then enter the project:

cd <your-repository-name>
Configure the Project

Create the build files:

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
Build the Application
cmake --build build --parallel

A successful build should complete without compilation errors.

Run the Application
./build/csms_cpp

The application should start using the configured HTTP port.

The starter project uses:

http://127.0.0.1:8080
Verify the Home Page

Open:

http://127.0.0.1:8080

The page should display:

Community Services Management System

and:

C++ with Drogon
Verify the Health Endpoint

Open:

http://127.0.0.1:8080/health

Expected response:

{
  "application": "Community Services Management System",
  "status": "ok",
  "version": "0.1.0"
}

You may also test it using:

curl http://127.0.0.1:8080/health
Run Automated Tests

After building:

ctest --test-dir build --output-on-failure

All tests should pass before committing completed work.

Development Workflow

For normal development tickets:

main
 ↓
feature branch
 ↓
development
 ↓
test
 ↓
commit
 ↓
push
 ↓
pull request
 ↓
review
 ↓
merge

Do not implement sprint work directly on main unless specifically instructed.

Branch Naming

Use meaningful branch names.

Examples:

feature/csms-101-navigation
feature/csms-201-resident-model
fix/csms-204-error-handling
docs/update-developer-guide
Commit Messages

Use the commit convention established for the course:

<type>: <short description>

Common types:

chore:
feat:
fix:
test:
refactor:
docs:

Examples:

chore: establish Drogon starter application
feat: establish CSMS starter interface
test: add application smoke tests
refactor: simplify controller routing
docs: complete C++ project documentation
Before Every Commit

Check:

git status

Run:

cmake --build build --parallel

Run:

ctest --test-dir build --output-on-failure

Then stage only the intended files.

Important Rules

Do not commit:

build output;
operating-system files;
IDE temporary files;
generated binaries;
local secrets;
machine-specific configuration.

Do not modify unrelated code when completing a ticket.

Do not combine several unrelated tickets into one commit.

Keep commits focused and understandable.

Sprint 0 Completion

Before completing Sprint 0, verify that:

the repository has been cloned successfully;
the C++ environment works;
the project configures successfully;
the project builds successfully;
the home page loads;
/health returns the expected JSON;
automated tests pass;
ABOUT_THE_DEVELOPER.md is completed;
all required changes are committed and pushed.

---

# `docs/git-cheatsheet.md`

```md
# Git Cheatsheet

## Check Repository Status

```bash
git status
View Commit History
git log --oneline

Example:

git log --oneline -10
Check Remote Repository
git remote -v
Download Remote Changes
git fetch
Update Current Branch
git pull
Create a New Branch
git switch -c feature/csms-101-navigation
View Branches
git branch
Switch Branch
git switch main
Stage a Specific File
git add filename
Stage a Directory
git add controllers/
Stage All Intended Changes
git add .

Always review git status before committing.

Commit
git commit -m "feat: implement resident model"
Push a New Branch
git push -u origin feature/csms-101-navigation
Push an Existing Branch
git push
Return to Main
git switch main
Update Main
git pull
Remove a Local Branch

Only after the branch is merged:

git branch -d feature/csms-101-navigation
Recommended Ticket Workflow
git switch main
git pull
git switch -c feature/csms-101-navigation

Develop and test.

Then:

git status
git add .
git commit -m "feat: implement application navigation"
git push -u origin feature/csms-101-navigation

Create the Pull Request in GitHub.

Commit Types
chore   project setup or maintenance
feat    new functionality
fix     defect correction
test    automated test changes
refactor code restructuring without changing behavior
docs    documentation
Golden Rule

Before every commit:

git status

Before every push:

ctest --test-dir build --output-on-failure

---

# `docs/release-workflow.md`

```md
# Release Workflow

## Purpose

The Community Services Management System is developed incrementally throughout the semester.

Each sprint contributes to a final working application.

## Normal Development Cycle

```text
Ticket
  ↓
Branch
  ↓
Development
  ↓
Build
  ↓
Automated Tests
  ↓
Commit
  ↓
Push
  ↓
Pull Request
  ↓
Code Review
  ↓
Merge
Step 1 – Start from Main
git switch main
git pull
Step 2 – Create a Ticket Branch

Example:

git switch -c feature/csms-201-resident-model
Step 3 – Implement the Ticket

Change only the files required by the assigned ticket.

Follow the project architecture and coding conventions.

Step 4 – Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

The build must succeed.

Step 5 – Run Tests
ctest --test-dir build --output-on-failure

Fix failing tests before continuing.

Step 6 – Review Changes
git status
git diff

Confirm that only intended files were modified.

Step 7 – Commit

Example:

git add .
git commit -m "feat: implement resident model"
Step 8 – Push

For a new branch:

git push -u origin feature/csms-201-resident-model
Step 9 – Create Pull Request

Open GitHub and create a Pull Request from the ticket branch into:

main

The Pull Request should contain:

ticket reference;
summary of changes;
test evidence;
notes for the reviewer.
Step 10 – Review

Review may include:

correctness;
architecture;
readability;
naming;
testing;
ticket compliance;
Git quality.

Required changes should be committed to the same branch.

Step 11 – Merge

Merge only when:

requirements are satisfied;
the application builds;
automated tests pass;
review comments are resolved.
Step 12 – Synchronize Local Main

After merging:

git switch main
git pull
Releases

Formal version releases will be created when instructed.

The starter application begins at:

0.1.0

Future release tags may follow semantic versioning:

v0.1.0
v0.2.0
v0.3.0
v1.0.0

Do not create release tags unless required by the sprint or instructor.

Final Rule

A completed ticket is not simply code that runs.

A completed ticket should have:

Requirement satisfied
+
Build successful
+
Tests passing
+
Meaningful Git history
+
Pull Request reviewed
+
Changes merged

---

# 3. Update the root `README.md`

If your C++ repo still does not have one, I strongly recommend adding it now. Use:

```md
# Community Services Management System

## C++ with Drogon

This repository is the C++ starter implementation of the Community Services Management System for the Programming Languages Laboratory.

**Current Sprint:** Sprint 0 – Developer Onboarding  
**Technology:** C++ with Drogon  
**Version:** 0.1.0

## Requirements

Install:

- Git
- C++ compiler
- CMake
- Drogon

Verify:

```bash
git --version
c++ --version
cmake --version
drogon_ctl version
Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
Build
cmake --build build --parallel
Run
./build/csms_cpp

Open:

http://127.0.0.1:8080
Health Check
http://127.0.0.1:8080/health

Expected:

{
  "application": "Community Services Management System",
  "status": "ok",
  "version": "0.1.0"
}
Tests
ctest --test-dir build --output-on-failure


Build and test again

Before committing:

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure


Check exactly what you're about to commit

Run:

git status

Ideally you should see:

Untracked files:
  ABOUT_THE_DEVELOPER.md
  README.md
  docs/

Possibly .gitignore as modified if you updated it.

Then:

git diff

and:

git diff --stat
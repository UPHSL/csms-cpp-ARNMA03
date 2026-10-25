Development Philosophy

Students should not attempt to implement the complete system immediately.

Development will be performed incrementally through sprint tickets.

Each ticket should normally follow this workflow:

Issue
↓
Branch
↓
Implementation
↓
Local Testing
↓
Commit
↓
Push
↓
Pull Request
↓
Review
↓
Merge

The Git history forms part of the student's development evidence throughout the semester.


---

# `docs/architecture.md`

```md
# C++ Application Architecture

## Overview

The C++ implementation of the Community Services Management System uses the Drogon web framework.

The starter repository establishes the application structure only. Business functionality will be introduced gradually through sprint tickets.

## Project Structure

```text
csms-cpp-template/
│
├── controllers/
│   ├── HomeController.h
│   ├── HomeController.cc
│   ├── HealthController.h
│   └── HealthController.cc
│
├── models/
│
├── filters/
│
├── plugins/
│
├── views/
│
├── uploads/
│
├── test/
│   ├── CMakeLists.txt
│   └── test_main.cc
│
├── docs/
│
├── config.json
├── config.yaml
├── CMakeLists.txt
├── main.cc
└── ABOUT_THE_DEVELOPER.md
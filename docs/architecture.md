Architectural Responsibilities
main.cc

Application entry point.

Responsibilities include:

loading application configuration;
initializing Drogon;
starting the HTTP server.
controllers/

Contains HTTP request handlers.

Examples:

HomeController
HealthController

Future sprint tickets will introduce additional controllers.

models/

Contains domain and data-related structures.

Future examples may include:

Resident
ServiceRequest
filters/

Contains Drogon request filters where authentication, authorization, validation, or request-processing logic may later be introduced.

plugins/

Contains Drogon plugins and reusable application services when required by future tickets.

views/

Contains presentation templates or related rendering resources when used by the project.

test/

Contains automated application tests.

Tests should be executed before code is pushed.

Request Flow

A simplified request flow is:

Browser / Client
       ↓
Drogon Router
       ↓
Controller
       ↓
Service / Domain Logic
       ↓
Model / Repository
       ↓
Response

Not all layers are implemented during Sprint 0.

They will be introduced as the project evolves.

Design Principle

Keep responsibilities separated.

Controllers should handle HTTP concerns.

Business logic should not be concentrated inside controllers.

Data access should remain separate from presentation and request-handling logic.

This makes the project easier to test, maintain, and compare with implementations using other technology stacks.


---

# `docs/developer-handbook.md`

```md
# C++ Developer Handbook

## Purpose

This handbook defines the development practices for the C++ / Drogon implementation of the Community Services Management System.

Students should use these practices throughout the semester.

## Required Tools

Install the following before starting development:

- Git
- C++ compiler
- CMake
- Drogon
- VS Code or another approved IDE
- GitHub account

## Verify the Development Environment

Check Git:

```bash
git --version
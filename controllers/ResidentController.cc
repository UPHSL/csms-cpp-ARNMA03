#include "ResidentController.h"

#include "../app/AppContext.h"
#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../services/ResidentUpdateService.h"

#include <charconv>
#include <optional>
#include <string>

using namespace drogon;

namespace
{
HttpResponsePtr applicationNotReady()
{
    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(k500InternalServerError);
    response->setBody("Application is not ready.");
    return response;
}

HttpResponsePtr residentNotFound()
{
    HttpViewData data;
    data.insert("title", std::string("Not Found"));
    data.insert("message", std::string("Resident not found."));
    auto response = HttpResponse::newHttpViewResponse("NotFound", data);
    response->setStatusCode(k404NotFound);
    return response;
}

// Digits only, 1 to 9 characters, greater than zero. Anything else
// (letters, signs, empty, absurdly long) is treated as "no such resident".
std::optional<int> parseResidentId(const std::string& text)
{
    if (text.empty() || text.size() > 9)
    {
        return std::nullopt;
    }
    for (const char c : text)
    {
        if (c < '0' || c > '9')
        {
            return std::nullopt;
        }
    }

    int value = 0;
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(text.data(), end, value);
    if (result.ec != std::errc() || result.ptr != end || value <= 0)
    {
        return std::nullopt;
    }
    return value;
}

// Only whitelisted codes ever become on-screen text; arbitrary query
// text is never echoed.
std::string successMessageFor(const std::string& code)
{
    if (code == "registered")
    {
        return "Resident registered successfully.";
    }
    if (code == "updated")
    {
        return "Resident updated successfully.";
    }
    if (code == "deactivated")
    {
        return "Resident deactivated successfully.";
    }
    if (code == "already-inactive")
    {
        return "Resident is already inactive.";
    }
    return "";
}

// Presentation only: turns the validator's flags into messages.
std::string messageIf(bool valid, const char* message)
{
    return valid ? std::string() : std::string(message);
}

// Every value inserted must be a std::string (not a const char*),
// because the view reads them back with get<std::string>.
// residentIdText/statusText are empty for the registration form.
HttpResponsePtr renderResidentForm(
    const std::string& title,
    const std::string& formAction,
    const std::string& submitLabel,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& address,
    const std::string& contactNumber,
    const std::string& email,
    const std::string& residentIdText,
    const std::string& statusText,
    const ResidentValidationResult& validation,
    HttpStatusCode status)
{
    HttpViewData data;
    data.insert("title", title);
    data.insert("formAction", formAction);
    data.insert("submitLabel", submitLabel);
    data.insert("firstName", firstName);
    data.insert("lastName", lastName);
    data.insert("address", address);
    data.insert("contactNumber", contactNumber);
    data.insert("email", email);
    data.insert("residentIdText", residentIdText);
    data.insert("statusText", statusText);
    data.insert("firstNameError",
        messageIf(validation.firstNameValid, "First name is required."));
    data.insert("lastNameError",
        messageIf(validation.lastNameValid, "Last name is required."));
    data.insert("addressError",
        messageIf(validation.addressValid, "Address is required."));
    data.insert("contactNumberError",
        messageIf(validation.contactNumberValid,
            "Contact number must be 11 digits starting with 09."));
    data.insert("emailError",
        messageIf(validation.emailValid, "Enter a valid email address."));

    auto response = HttpResponse::newHttpViewResponse("ResidentForm", data);
    response->setStatusCode(status);
    return response;
}

const ResidentValidationResult kNoErrors{true, true, true, true, true, true};
}

void ResidentController::list(
    const HttpRequestPtr& request,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    const std::string query = request->getParameter("q");

    HttpViewData data;
    data.insert("title", std::string("Residents"));
    data.insert("query", query);
    data.insert("successMessage",
        successMessageFor(request->getParameter("success")));
    data.insert("residents", context->residentQuery().searchResidents(query));

    callback(HttpResponse::newHttpViewResponse("ResidentList", data));
}

void ResidentController::newForm(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    callback(renderResidentForm(
        "Register Resident", "/residents", "Register",
        "", "", "", "", "",
        "", "",
        kNoErrors, k200OK));
}

void ResidentController::create(
    const HttpRequestPtr& request,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    const std::string firstName = request->getParameter("firstName");
    const std::string lastName = request->getParameter("lastName");
    const std::string address = request->getParameter("address");
    const std::string contactNumber = request->getParameter("contactNumber");
    const std::string email = request->getParameter("email");

    const Resident resident(firstName, lastName, address, contactNumber, email);
    const auto result = context->residentRegistration().registerResident(resident);

    if (!result.success)
    {
        callback(renderResidentForm(
            "Register Resident", "/residents", "Register",
            firstName, lastName, address, contactNumber, email,
            "", "",
            result.validationResult, k400BadRequest));
        return;
    }

    callback(HttpResponse::newRedirectionResponse(
        "/residents?success=registered", k303SeeOther));
}

void ResidentController::editForm(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    const auto residentId = parseResidentId(id);
    if (!residentId)
    {
        callback(residentNotFound());
        return;
    }

    const auto resident = context->residentQuery().findResident(*residentId);
    if (!resident)
    {
        callback(residentNotFound());
        return;
    }

    const std::string idText = std::to_string(*residentId);
    callback(renderResidentForm(
        "Edit Resident", "/residents/" + idText, "Save Changes",
        resident->getFirstName(), resident->getLastName(),
        resident->getAddress(), resident->getContactNumber(),
        resident->getEmail(),
        idText, resident->getStatus(),
        kNoErrors, k200OK));
}

void ResidentController::update(
    const HttpRequestPtr& request,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    const auto residentId = parseResidentId(id);
    if (!residentId)
    {
        callback(residentNotFound());
        return;
    }

    // Only these five fields are ever read from the form. id comes from the
    // URL and status is not editable, so a forged "status" parameter is ignored.
    const ResidentUpdateInformation information{
        request->getParameter("firstName"),
        request->getParameter("lastName"),
        request->getParameter("address"),
        request->getParameter("contactNumber"),
        request->getParameter("email")
    };

    const auto result =
        context->residentUpdate().updateResident(*residentId, information);

    if (result.notFound)
    {
        callback(residentNotFound());
        return;
    }

    if (!result.success)
    {
        // Read-only lookup, just to show the unchanged status on the form.
        const auto existing = context->residentQuery().findResident(*residentId);
        if (!existing)
        {
            callback(residentNotFound());
            return;
        }

        const std::string idText = std::to_string(*residentId);
        callback(renderResidentForm(
            "Edit Resident", "/residents/" + idText, "Save Changes",
            information.firstName, information.lastName,
            information.address, information.contactNumber, information.email,
            idText, existing->getStatus(),
            result.validationResult, k400BadRequest));
        return;
    }

    callback(HttpResponse::newRedirectionResponse(
        "/residents?success=updated", k303SeeOther));
}

void ResidentController::deactivate(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    const auto residentId = parseResidentId(id);
    if (!residentId)
    {
        callback(residentNotFound());
        return;
    }

    const auto result = context->residentDeactivation().deactivate(*residentId);

    switch (result.outcome)
    {
    case ResidentDeactivationOutcome::Deactivated:
        callback(HttpResponse::newRedirectionResponse(
            "/residents?success=deactivated", k303SeeOther));
        return;
    case ResidentDeactivationOutcome::AlreadyInactive:
        callback(HttpResponse::newRedirectionResponse(
            "/residents?success=already-inactive", k303SeeOther));
        return;
    case ResidentDeactivationOutcome::NotFound:
        break;
    }

    callback(residentNotFound());
}
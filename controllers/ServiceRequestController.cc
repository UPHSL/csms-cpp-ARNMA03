#include "ServiceRequestController.h"

#include "../app/AppContext.h"
#include "../models/ServiceRequest.h"
#include "../models/ServiceRequestValidator.h"
#include "../services/ServiceRequestQueryService.h"
#include "../services/ServiceRequestStatusService.h"
#include "../services/ServiceRequestSubmissionService.h"

#include <charconv>
#include <optional>
#include <string>
#include <vector>

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

HttpResponsePtr serviceRequestNotFound()
{
    HttpViewData data;
    data.insert("title", std::string("Not Found"));
    data.insert("message", std::string("Service Request not found."));
    auto response =
        HttpResponse::newHttpViewResponse("ServiceRequestNotFound", data);
    response->setStatusCode(k404NotFound);
    return response;
}

// Digits only, 1 to 9 characters, greater than zero. Anything else
// (letters, signs, empty, absurdly long) yields nullopt.
std::optional<int> parsePositiveId(const std::string& text)
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
    if (code == "submitted")
    {
        return "Service Request submitted successfully.";
    }
    if (code == "started")
    {
        return "Service Request is now In Progress.";
    }
    if (code == "completed")
    {
        return "Service Request completed successfully.";
    }
    if (code == "cancelled")
    {
        return "Service Request cancelled successfully.";
    }
    return "";
}

// Presentation only: turns the validator's flags into messages.
std::string messageIf(bool valid, const char* message)
{
    return valid ? std::string() : std::string(message);
}

// Every value inserted must be a std::string (not a const char*).
HttpResponsePtr renderSubmissionForm(
    const std::string& residentIdText,
    const std::string& serviceType,
    const std::string& description,
    const std::string& dateRequested,
    const ServiceRequestValidationResult& validation,
    const std::string& formError,
    HttpStatusCode status)
{
    HttpViewData data;
    data.insert("title", std::string("Submit Service Request"));
    data.insert("residentIdText", residentIdText);
    data.insert("serviceType", serviceType);
    data.insert("description", description);
    data.insert("dateRequested", dateRequested);
    data.insert("formError", formError);
    data.insert("residentIdError",
        messageIf(validation.residentIdValid,
            "Resident ID must be a positive whole number."));
    data.insert("serviceTypeError",
        messageIf(validation.serviceTypeValid, "Service type is required."));
    data.insert("descriptionError",
        messageIf(validation.descriptionValid, "Description is required."));
    data.insert("dateRequestedError",
        messageIf(validation.dateRequestedValid, "Date requested is required."));

    auto response = HttpResponse::newHttpViewResponse("ServiceRequestForm", data);
    response->setStatusCode(status);
    return response;
}

HttpResponsePtr renderDetails(
    const ServiceRequest& serviceRequest,
    const std::string& errorMessage,
    HttpStatusCode status)
{
    HttpViewData data;
    data.insert("title", std::string("Service Request Details"));
    data.insert("idText",
        std::to_string(serviceRequest.getId().value_or(0)));
    data.insert("residentIdText",
        std::to_string(serviceRequest.getResidentId()));
    data.insert("serviceType", serviceRequest.getServiceType());
    data.insert("description", serviceRequest.getDescription());
    data.insert("dateRequested", serviceRequest.getDateRequested());
    data.insert("status", serviceRequest.getStatus());
    data.insert("errorMessage", errorMessage);

    auto response =
        HttpResponse::newHttpViewResponse("ServiceRequestDetails", data);
    response->setStatusCode(status);
    return response;
}

// Shared by start / complete / cancel. This only translates the button
// into a requested status and the T10 outcome into an HTTP response.
// Whether the transition is allowed is decided entirely by T10.
void changeStatusAndRespond(
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& idText,
    const std::string& requestedStatus,
    const std::string& successCode)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    const auto serviceRequestId = parsePositiveId(idText);
    if (!serviceRequestId)
    {
        callback(serviceRequestNotFound());
        return;
    }

    const auto result = context->serviceRequestStatus().changeStatus(
        *serviceRequestId, requestedStatus);

    switch (result.outcome)
    {
    case ServiceRequestStatusOutcome::Success:
        callback(HttpResponse::newRedirectionResponse(
            "/service-requests?success=" + successCode, k303SeeOther));
        return;
    case ServiceRequestStatusOutcome::NotFound:
        callback(serviceRequestNotFound());
        return;
    case ServiceRequestStatusOutcome::InvalidTransition:
    case ServiceRequestStatusOutcome::UnsupportedStatus:
        break;
    }

    // Rejected by T10: nothing was changed. Show the unchanged request.
    const auto current =
        context->serviceRequestQuery().viewServiceRequest(*serviceRequestId);
    if (!current)
    {
        callback(serviceRequestNotFound());
        return;
    }
    callback(renderDetails(
        *current,
        "That action is not allowed for this Service Request's current status.",
        k409Conflict));
}

const ServiceRequestValidationResult kNoErrors{true, true, true, true, true, true};
}

void ServiceRequestController::list(
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
    const std::string statusFilter = request->getParameter("status");

    // T11 does the trimming, blank handling, searching and filtering.
    const auto result =
        context->serviceRequestQuery().searchServiceRequests(query, statusFilter);

    const bool unsupported =
        result.outcome == ServiceRequestSearchOutcome::UnsupportedStatusFilter;

    HttpViewData data;
    data.insert("title", std::string("Service Request Management"));
    data.insert("query", query);
    data.insert("selectedStatus", statusFilter);
    data.insert("successMessage",
        successMessageFor(request->getParameter("success")));
    data.insert("errorMessage",
        unsupported ? std::string("Unsupported status filter.") : std::string());
    data.insert("serviceRequests", result.serviceRequests);

    auto response = HttpResponse::newHttpViewResponse("ServiceRequestList", data);
    if (unsupported)
    {
        response->setStatusCode(k400BadRequest);
    }
    callback(response);
}

void ServiceRequestController::newForm(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    callback(renderSubmissionForm("", "", "", "", kNoErrors, "", k200OK));
}

void ServiceRequestController::create(
    const HttpRequestPtr& request,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    // Only these four fields are ever read. The id is generated by
    // persistence and the status always starts as Pending, so forged
    // "id" or "status" parameters are ignored.
    const std::string residentIdText = request->getParameter("residentId");
    const std::string serviceType = request->getParameter("serviceType");
    const std::string description = request->getParameter("description");
    const std::string dateRequested = request->getParameter("dateRequested");

    // An unparsable Resident ID becomes 0, which T09's validator rejects.
    const int residentId = parsePositiveId(residentIdText).value_or(0);

    const ServiceRequest serviceRequest(
        residentId, serviceType, description, dateRequested);
    const auto result =
        context->serviceRequestSubmission().submit(serviceRequest);

    switch (result.outcome)
    {
    case ServiceRequestSubmissionOutcome::Success:
        callback(HttpResponse::newRedirectionResponse(
            "/service-requests?success=submitted", k303SeeOther));
        return;
    case ServiceRequestSubmissionOutcome::ValidationFailure:
        callback(renderSubmissionForm(
            residentIdText, serviceType, description, dateRequested,
            result.validationResult, "", k400BadRequest));
        return;
    case ServiceRequestSubmissionOutcome::ResidentNotFound:
        callback(renderSubmissionForm(
            residentIdText, serviceType, description, dateRequested,
            kNoErrors, "Resident not found.", k400BadRequest));
        return;
    case ServiceRequestSubmissionOutcome::ResidentInactive:
        callback(renderSubmissionForm(
            residentIdText, serviceType, description, dateRequested,
            kNoErrors,
            "Inactive Residents cannot submit new Service Requests.",
            k400BadRequest));
        return;
    }

    callback(applicationNotReady());
}

void ServiceRequestController::details(
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

    const auto serviceRequestId = parsePositiveId(id);
    if (!serviceRequestId)
    {
        callback(serviceRequestNotFound());
        return;
    }

    // T11 view-by-id.
    const auto serviceRequest =
        context->serviceRequestQuery().viewServiceRequest(*serviceRequestId);
    if (!serviceRequest)
    {
        callback(serviceRequestNotFound());
        return;
    }

    callback(renderDetails(*serviceRequest, "", k200OK));
}

void ServiceRequestController::start(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string id)
{
    changeStatusAndRespond(std::move(callback), id, "In Progress", "started");
}

void ServiceRequestController::complete(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string id)
{
    changeStatusAndRespond(std::move(callback), id, "Completed", "completed");
}

void ServiceRequestController::cancel(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string id)
{
    changeStatusAndRespond(std::move(callback), id, "Cancelled", "cancelled");
}
#include "HomeController.h"

void HomeController::asyncHandleHttpRequest(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Community Services Management System</title>

    <style>
        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            background: #f4f5f7;
            font-family: Arial, Helvetica, sans-serif;
            color: #111;
        }

        .container {
            width: min(860px, 90%);
            background: white;
            padding: 48px;
            border: 1px solid #d9dde3;
            border-radius: 14px;
            box-shadow: 0 20px 45px rgba(0, 0, 0, 0.08);
        }

        .label {
            margin: 0;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.08em;
        }

        h1 {
            margin: 1em 0 0.5em;
            font-size: clamp(2.2rem, 5vw, 4rem);
            line-height: 1.05;
        }

        .message {
            font-size: 1.15rem;
        }

        .details {
            display: grid;
            gap: 1rem;
            margin: 2rem 0;
        }

        .detail {
            background: #f6f7f8;
            padding: 20px;
            border-radius: 12px;
        }

        .detail strong {
            display: block;
            margin-bottom: 8px;
            text-transform: uppercase;
            font-size: 0.9rem;
        }

        .status {
            margin-bottom: 0;
            font-weight: 700;
        }
    </style>
</head>

<body>
    <main class="container">
        <p class="label">Programming Languages Laboratory</p>

        <h1>Community Services<br>Management System</h1>

        <p class="message">
            Starter application initialized successfully.
        </p>

        <section class="details">
            <div class="detail">
                <strong>Current Sprint</strong>
                Sprint 0 - Developer Onboarding
            </div>

            <div class="detail">
                <strong>Technology</strong>
                C++ with Drogon
            </div>

            <div class="detail">
                <strong>Version</strong>
                0.1.0
            </div>
        </section>

        <p class="status">Environment status: Ready</p>
    </main>
</body>
</html>
)";

    auto response = HttpResponse::newHttpResponse();
    response->setContentTypeCode(CT_TEXT_HTML);
    response->setBody(html);

    callback(response);
}
#pragma once

#include <string>

struct Response {
    std::string status;
    std::string contentType;
    std::string location; // uri
    std::string transferEncoding;
    std::string contentLength;
    std::string body;
};
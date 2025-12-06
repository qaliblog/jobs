#include "request_handler.h"
#include <iostream>
#include <sstream>
#include <regex>

RequestHandler::RequestHandler() {
}

void RequestHandler::setOnRecruitRequest(std::function<bool(const ConnectionRequest&)> callback) {
    on_recruit_request_ = callback;
}

void RequestHandler::setOnWorkRequest(std::function<bool(const ConnectionRequest&)> callback) {
    on_work_request_ = callback;
}

bool RequestHandler::handleRecruitRequest(const std::string& json) {
    ConnectionRequest request = parseRequest(json);
    request.requestType = "recruit";
    
    if (on_recruit_request_) {
        return on_recruit_request_(request);
    }
    
    // Default: show prompt
    return promptUser("Recruit request from " + request.requesterName + " (" + request.requesterAddress + ")");
}

bool RequestHandler::handleWorkRequest(const std::string& json) {
    ConnectionRequest request = parseRequest(json);
    request.requestType = "work";
    
    if (on_work_request_) {
        return on_work_request_(request);
    }
    
    // Default: auto-accept
    return true;
}

void RequestHandler::showRequest(const ConnectionRequest& request) {
    std::cout << "\n=== Connection Request ===" << std::endl;
    std::cout << "Type: " << request.requestType << std::endl;
    std::cout << "From: " << request.requesterName << std::endl;
    std::cout << "Address: " << request.requesterAddress << ":" << request.requesterPort << std::endl;
    std::cout << "===========================" << std::endl;
}

bool RequestHandler::promptUser(const std::string& message) {
    std::cout << message << std::endl;
    std::cout << "Accept? (y/n): ";
    
    std::string response;
    std::getline(std::cin, response);
    
    return (response == "y" || response == "Y" || response == "yes");
}

ConnectionRequest RequestHandler::parseRequest(const std::string& json) {
    ConnectionRequest request;
    
    std::regex idPattern("\"requesterId\"\\s*:\\s*\"([^\"]+)\"");
    std::regex namePattern("\"requesterName\"\\s*:\\s*\"([^\"]+)\"");
    std::regex addrPattern("\"requesterAddress\"\\s*:\\s*\"([^\"]+)\"");
    std::regex portPattern("\"requesterPort\"\\s*:\\s*([0-9]+)");
    
    std::smatch match;
    if (std::regex_search(json, match, idPattern)) {
        request.requesterId = match[1].str();
    }
    if (std::regex_search(json, match, namePattern)) {
        request.requesterName = match[1].str();
    }
    if (std::regex_search(json, match, addrPattern)) {
        request.requesterAddress = match[1].str();
    }
    if (std::regex_search(json, match, portPattern)) {
        request.requesterPort = std::stoi(match[1].str());
    }
    
    return request;
}


#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <functional>

struct ConnectionRequest {
    std::string requesterId;
    std::string requesterName;
    std::string requesterAddress;
    int requesterPort;
    std::string requestType; // "recruit" or "work"
};

class RequestHandler {
public:
    RequestHandler();
    
    void setOnRecruitRequest(std::function<bool(const ConnectionRequest&)> callback);
    void setOnWorkRequest(std::function<bool(const ConnectionRequest&)> callback);
    
    bool handleRecruitRequest(const std::string& json);
    bool handleWorkRequest(const std::string& json);
    
    void showRequest(const ConnectionRequest& request);
    bool promptUser(const std::string& message);
    
private:
    std::function<bool(const ConnectionRequest&)> on_recruit_request_;
    std::function<bool(const ConnectionRequest&)> on_work_request_;
    
    ConnectionRequest parseRequest(const std::string& json);
};

#endif // REQUEST_HANDLER_H


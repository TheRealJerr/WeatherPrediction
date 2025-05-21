#include "../include/Url.h"
#include "../include/Server.h"
using Weather::UrlRsp_t;
namespace wt = Weather;
#include <boost/beast/http.hpp>
#include <string>
#include "../include/Protocal.h"
#include "../include/Handler.h"

namespace http = boost::beast::http;

std::string make_http_date() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::gmtime(&time);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

http::response<http::string_body> createResponse(const std::string& content) {
    http::response<http::string_body> res{http::status::ok, 11}; // HTTP/1.1
    
    // 必需的头字段
    res.set(http::field::server, "hrj sever");
    res.set(http::field::content_type, "text/plain; charset=utf-8");
    res.set(http::field::date, make_http_date());
    res.set(http::field::connection, "close");
    
    // 可选但推荐的头字段
    res.set(http::field::cache_control, "no-cache");
    
    // 设置响应内容
    res.body() = content;
    
    // 自动设置Content-Length
    res.prepare_payload();
    
    return res;
}
void onMsgCallBack(reactor::Connection::Ptr con, std::string& msg) {
    UrlRsp_t rsp;
    wt::UrlRspBuilder::getUrlRsp("https://restapi.amap.com/v3/weather/weatherInfo?city=110101&key=38ad1d65140d41cce5a6b26931b2818b", rsp);
    
    std::stringstream ssm;
    wt::printJsonValue(rsp, ssm);
    
    std::cout << msg << std::endl;

    auto response = createResponse(ssm.str());
    
    // 直接序列化HTTP响应
    std::string http_response;
    http_response = "HTTP/1.1 " + std::to_string(response.result_int()) + " " + response.reason().to_string() + "\r\n";
    for (const auto& header : response.base()) {
        http_response += header.name_string().to_string() + ": " + header.value().to_string() + "\r\n";
    }
    http_response += "\r\n" + response.body();
    
    con->sendMsg(http_response);
}

void onMsgCb(reactor::Connection::Ptr con,std::string& msg)
{
    // 拿到数据
    // 分割出http请求
    auto reqs = wt::Protocol::parseHttpRequests({ msg.begin(), msg.end() });
    for(auto& req : reqs.first)
    {
        boost::beast::http::response<boost::beast::http::string_body> rsp;
        handleRequest(req.request,rsp);
        // 构建返回的报文返回
        std::stringstream ssm;
        ssm << rsp;
        con->sendMsg(ssm.str());
    }
}
int main()
{
    reactor::Server server(8080);
    
    server.setOnMsgCallBack(onMsgCallBack);
    server.start();
    return 0;
}
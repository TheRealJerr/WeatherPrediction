#pragma once 
// url模块, 针对url请求和相应通过JsonCpp实现序列化和反序列化
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
#include <optional>
namespace Weather
{
    // 面相对象封装libcurl
    using UrlRsp_t = std::optional<Json::Value>;
    struct ResponseData {
        std::string data;
    };
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t realsize = size * nmemb;
        ResponseData *response = static_cast<ResponseData *>(userp);
        response->data.append(static_cast<char *>(contents), realsize);
        return realsize;
    }
    class Url
    {
    public:
        Url(const std::string &);
        ~Url();

        bool getJsonMsg(std::optional<Json::Value>&); //输出请求获取到的结果

    private:
        std::string _url;
        CURL *_curl;
        CURLcode _res;
        ResponseData _response;
        std::string _error;
    };
    void printJsonValue(UrlRsp_t&,std::ostream&);

    class UrlRspBuilder
    {
    public:
        static bool getUrlRsp(const std::string& url,UrlRsp_t& rsp)
        {
            std::unique_ptr<Url> usl = std::make_unique<Url>(url);
            return usl->getJsonMsg(rsp);
        }
    };
}
#include "../include/Url.h"

namespace Weather
{
    Url::Url(const std::string &url) : _url(url)
    {
        _curl = curl_easy_init();
        // 设置curl的选项
        curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_response);
        curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
    }

    bool Url::getJsonMsg(std::optional<Json::Value> &value)
    {
        _res = curl_easy_perform(_curl);
        if (_res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(_res) << std::endl;
            return false;
        }
        // 解析JSON响应
        Json::Value root;
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
        std::string errors;

        bool parsingSuccessful = reader->parse(_response.data.c_str(),
                                               _response.data.c_str() + _response.data.size(),
                                               &root,
                                               &errors);
        if (!parsingSuccessful)
        {
            std::cerr << "Failed to parse JSON: " << errors << std::endl;
            return 1;
        }
        
        value.emplace(root);
        return true;
    }
    Url::~Url()
    {
        curl_easy_cleanup(_curl);
    }

    void printJsonValue(UrlRsp_t& value, std::ostream& stream)
    {
        auto& root = value.value();
        if (root["status"].asString() == "1") {
                // 解析天气数据
                const Json::Value& lives = root["lives"];
                if (!lives.empty()) {
                    const Json::Value& weatherInfo = lives[0];
                    
                    stream << "城市: " << weatherInfo["city"].asString() << std::endl;
                    stream << "天气: " << weatherInfo["weather"].asString() << std::endl;
                    stream << "温度: " << weatherInfo["temperature"].asString() << "°C" << std::endl;
                    stream << "风向: " << weatherInfo["winddirection"].asString() << std::endl;
                    stream << "风力: " << weatherInfo["windpower"].asString() << std::endl;
                    stream << "湿度: " << weatherInfo["humidity"].asString() << "%" << std::endl;
                    stream << "报告时间: " << weatherInfo["reporttime"].asString() << std::endl;
                }
            } else {
                std::cerr << "API错误: " << root["info"].asString() 
                          << " (错误码: " << root["infocode"].asString() << ")" << std::endl;
            }
    }
}
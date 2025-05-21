//
#pragma once
#include "Common.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;


namespace Weather
{
    class Protocol
    {
    public:
        // HTTP请求结构体
        struct HttpRequest
        {
            http::request<http::string_body> request;
            size_t bytes_consumed; // 实际消耗的字节数
        };

        // 解析HTTP请求
        static std::pair<std::vector<HttpRequest>, std::vector<char>>
        parseHttpRequests(const std::vector<char> &data)
        {
            std::vector<HttpRequest> requests;
            std::vector<char> remaining_data = data;
            beast::flat_buffer buffer;

            while (!remaining_data.empty())
            {
                // 只提交当前剩余数据
                buffer.commit(asio::buffer_copy(
                    buffer.prepare(remaining_data.size()),
                    asio::buffer(remaining_data)));

                http::request_parser<http::string_body> parser;
                beast::error_code ec;

                // 尝试解析请求
                auto const buffer_seq = boost::beast::net::buffer(buffer.data());  // 转换为 ConstBufferSequence
                size_t bytes_parsed = parser.put(buffer_seq, ec);  // 正确
                

                if (ec == http::error::need_more)
                {
                    // 需要更多数据才能完成解析
                    break;
                }
                else if (ec)
                {
                    // 解析错误 - 记录日志但不抛出异常
                    std::cerr << "HTTP解析错误: " << ec.message() << std::endl;
                    
                    // 消耗已处理的数据
                    remaining_data.erase(
                        remaining_data.begin(),
                        remaining_data.begin() + bytes_parsed);
                        
                    buffer.consume(bytes_parsed);
                    
                    // 可以选择返回错误响应
                    continue;
                }
                else
                {
                    // 成功解析一个完整请求
                    HttpRequest req;
                    req.request = parser.release();
                    req.bytes_consumed = bytes_parsed;

                    requests.push_back(std::move(req));

                    // 移除已处理的数据
                    remaining_data.erase(
                        remaining_data.begin(),
                        remaining_data.begin() + bytes_parsed);

                    // 消耗已处理的缓冲区数据
                    buffer.consume(bytes_parsed);
                }
            }

            return {requests, remaining_data};
        }

        // 生成HTTP响应
        static std::string createHttpResponse(
            http::status status,
            const std::string &body,
            const std::string &content_type = "text/plain")
        {
            http::response<http::string_body> res{status, 11}; // HTTP/1.1
            res.set(http::field::server, "MyServer");
            res.set(http::field::content_type, content_type);
            res.body() = body;
            res.prepare_payload();

            // 使用Beast的序列化器
            std::stringstream ssm;
            ssm << res;
            
            return ssm.str();
        }
    };
}

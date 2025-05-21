#pragma once
#include "../include/Common.h"
#include "../include/Protocal.h"
// 定义业务处理的函数接口 


#include <string>
#include <unordered_map>
#include <iostream>

namespace http = beast::http;

// 读取文件到字符串（二进制模式）
std::string readFileToString(const std::string& filePath);

// 处理 HTTP 请求的函数
void handleRequest(http::request<http::string_body>& req, 
                  http::response<http::string_body>& res);

// 处理 GET 请求
void handleGetRequest(const std::string& path, 
                     const std::unordered_map<std::string, std::string>& params,
                     const http::request<http::string_body>& re,
                     http::response<http::string_body>& res);
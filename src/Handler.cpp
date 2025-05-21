#include "../include/Handler.h"

// 读取文件到字符串（二进制模式）
std::string readFileToString(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 读取内容到字符串
    std::string content(size, '\0');
    file.read(content.data(), size);

    return content;
}

// 处理 HTTP 请求的函数
void handleRequest(http::request<http::string_body>& req, 
                  http::response<http::string_body>& res) {
    // 1. 解析请求路径和查询参数
    std::string target = req.target().to_string();
    // /index.html
    std::string path;
    std::unordered_map<std::string, std::string> queryParams;
    
    // 简单解析路径和查询参数（实际项目中可能需要更复杂的解析）
    size_t queryPos = target.find('?');
    if (queryPos != std::string::npos) {
        path = target.substr(0, queryPos);
        // 解析查询参数（此处省略具体实现）
    } else {
        path = target;
    }
    
    // 2. 根据请求方法执行不同逻辑
    switch (req.method()) {
        case http::verb::get:
            handleGetRequest(path, queryParams, req, res);
            break;
            
        default:
            res.result(http::status::method_not_allowed);
            res.set(http::field::allow, "GET, POST, PUT, DELETE");
            res.body() = "Method Not Allowed";
            break;
    }
    
    // 3. 设置通用响应头部
    res.version(req.version());
    res.keep_alive(false);  // 简单示例中关闭长连接
    res.prepare_payload();  // 自动计算 Content-Length
}

// 处理 GET 请求
void handleGetRequest(const std::string& path, 
                     const std::unordered_map<std::string, std::string>& params,
                     const http::request<http::string_body>& req,
                     http::response<http::string_body>& res) {
    // if (path == "/") {
    //     // 返回主页
    //     res.result(http::status::ok);
    //     res.set(http::field::content_type, "text/html");
    //     res.body() = "<html><body>Hello, World!</body></html>";
    // } 
    // else if (path == "/api/data") {
    //     // 返回 JSON 数据
    //     res.result(http::status::ok);
    //     res.set(http::field::content_type, "application/json");
    //     res.body() = R"({"message": "This is sample data"})";
    // }
    // else {
    //     res.result(http::status::not_found);
    //     res.body() = "Resource Not Found";
    // }
    std::string tmp;
    if(path == "/") tmp = gprojectPath + "/wwwroot" + "/index.html";
    else tmp = gprojectPath + "/wwwroot" + path;
    // 打开path路径的文件并且直接返回给body
    // std::cout << tmp;
    if(std::filesystem::exists(tmp) == false)
    {
        res.result(http::status::not_found);
        res.body() = readFileToString(gprojectPath + "/wwwroot" + "/404page.html");
    }
    else
    {
        res.result(http::status::ok);
        res.body() = readFileToString(tmp);
    }
}

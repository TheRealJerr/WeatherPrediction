# cpp后端版本

## 依赖库

libcurl pthread boost.asio boost.beast

## 后端设计

后端基于one thread one loop, 通过epoll模型实现高效非阻塞异步IO, 通信协议采取http, 
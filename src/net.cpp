
#include "../include/Common.h"
#include "../include/net.h"

namespace reactor
{
    void setSockOpt()
    {
    }

    void ServerSocket::createSocket()
    {
        _sock_listen = ::socket(AF_INET, SOCK_STREAM, 0);
        if (_sock_listen < 0)
        {
            LOG(LogLevel::ERROR) << "套接字创建失败";
            ::exit(1);
        }
        int opt = 1;  
        setsockopt(_sock_listen, SOL_SOCKET,  SO_REUSEADDR, (const void *)&opt, sizeof(opt)); 
    }

    void ServerSocket::bindSocket()
    {
        // 将套接字进行bind
        // struct sockaddr_in sock;
        // sock.sin_addr.s_addr = INADDR_ANY;
        // sock.sin_family = AF_INET;
        // sock.sin_port = ::htons(_port);
        SockAddrInfo sock_info(_port);
        if (::bind(_sock_listen, SOCKINTOSOCK(sock_info.createSockAddrIn()), sock_info.sockLen()) < 0)
        {
            LOG(LogLevel::ERROR) << "bind失败:" << strerror(errno);
            ::exit(1);
        }

        LOG(LogLevel::DEBUG) << "bind成功";
    }

    void ServerSocket::listen(int backlog)
    {
        if (_sock_listen < 0)
        {
            LOG(LogLevel::ERROR) << "error sock";
            ::exit(1);
        }

        int n = ::listen(_sock_listen, backlog);
        if (n < 0)
        {
            LOG(LogLevel::ERROR) << "listen error";
            ::exit(1);
        }

        LOG(LogLevel::DEBUG) << "listen成功";
    }
    Connection::Ptr ServerSocket::accept(SockAddrInfo *peer_info)
    {
        struct sockaddr_in peer;
        socklen_t peer_len = 0;
        int sock_communicate = ::accept(_sock_listen, SOCKINTOSOCK(&peer), &peer_len);
        if (sock_communicate < 0)
        {
            LOG(LogLevel::ERROR) << "accept失败" << strerror(errno);
            return nullptr;
        }
        else
        {
            peer_info->acceptPeerInfo(&peer);
            LOG(LogLevel::DEBUG) << "ip:" << peer_info->ip()
                                 << " port:" << peer_info->port() << " accept成功";
        }
        LOG(LogLevel::DEBUG) << "得到了通信套接字:" << sock_communicate;
        // 拿到了communicate
        // 拿到了peer的信息传递出去
        // 我们返回一个通信连接

        return std::make_shared<Connection>(sock_communicate);
    }

    // 后续将改成我们自己定义的msg类型
    void Connection::sendMsg(const std::string &msg)
    {
        size_t totalSent = 0;
        const char *data = msg.c_str();
        size_t remaining = msg.size();

        while (remaining > 0)
        {
            int n = ::send(_sock_communicate, data + totalSent, remaining, 0);

            if (n > 0)
            {
                totalSent += n;
                remaining -= n;
            }
            else if (n == 0)
            {
                LOG(LogLevel::WARNING) << "send返回0，连接可能已关闭";
                return;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 对于非阻塞socket，可以稍后重试
                    LOG(LogLevel::DEBUG) << "发送暂时阻塞，稍后重试";
                    continue;
                }
                else
                {
                    LOG(LogLevel::ERROR) << "发送失败: " << strerror(errno);
                    return;
                }
            }
        }

        LOG(LogLevel::INFO) << "成功发送" << totalSent << "字节数据";
        return;
    }
    // 关闭通信连接
    void Connection::shutDown()
    {
        if (_is_close == false)
        {
            // log(LogLevel::DEBUG) << "关闭了" << _sock_communicate << "套接字";
            ::close(_sock_communicate);
        }
    }

    void ClientSocket::createSocket()
    {
        _sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            LOG(LogLevel::ERROR) << "套接字创建失败";
            ::exit(1);
        }
        int optval = 1;
        // 由于我们现在是debug阶段，直接将socket设置成REUSE模式
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    }

    void Connection::readMsg(std::string *msg)
    {
        // log(LogLevel::DEBUG) << "通过" << _sock_communicate << "读取数据";
        if (_sock_status == SockStatus::NON_BLOCK)
        {
            // 非阻塞IO
            char buffer[READ_MAX_SIZE] = {0};
            while (true)
            {
                int n = ::recv(_sock_communicate, buffer, READ_MAX_SIZE - 1, 0);
                if (n == 0)
                {
                    // shutDown(); // 关闭连接
                    //
                    LOG(LogLevel::INFO) << "对端关闭了连接";
                    return;
                }
                else if (n > 0)
                {
                    buffer[n] = 0;
                    *msg += buffer;
                }
                else
                {
                    // n < 0
                    // 读取错误
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        LOG(LogLevel::INFO) << "读取完毕";
                        return;
                    }
                    else
                    {
                        LOG(LogLevel::ERROR) << "读取出现错误" << strerror(errno);
                        return;
                    }
                }
            }
        }
        // 阻塞IO
        else
        {
            char buffer[READ_MAX_SIZE] = {0};
            int n = ::read(_sock_communicate, buffer, READ_MAX_SIZE - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                *msg += buffer;
            }
            else if (n == 0)
            {
                LOG(LogLevel::INFO) << "对端关闭了连接";
                shutDown();
            }
            else
            {
                //
                LOG(LogLevel::ERROR) << "读取出现了错误" << strerror(errno);
            }
        }
    }
    Connection::Ptr ClientSocket::connect()
    {
        // 进行连接
        SockAddrInfo sock(_ip, _port);
        // 直接进行connect
        int n = ::connect(_sockfd, SOCKINTOSOCK(sock.createSockAddrIn()), sock.sockLen());
        if (n < 0)
        {
            LOG(LogLevel::ERROR) << "connect " << _ip << ":" << _port << " error " << strerror(errno);
            return nullptr;
        }
        else
            LOG(LogLevel::DEBUG) << "connect成功";
        return std::make_shared<Connection>(_sockfd);
    }

    void ServerSocket::start()
    {
        // 主线程
    }

    int Accept(SockAddrInfo *info, int sockfd)
    {
        struct sockaddr_in peer;
        socklen_t peer_len = 0;
        int sock_communicate = ::accept(sockfd, SOCKINTOSOCK(&peer), &peer_len);
        if (sock_communicate < 0)
        {
            LOG(LogLevel::ERROR) << "accept失败" << strerror(errno);
            return -1;
        }
        else
        {
            if (info)
                info->acceptPeerInfo(&peer);
        }
        LOG(LogLevel::DEBUG) << "得到了通信套接字:" << sock_communicate;

        return sock_communicate;
    }
    void Connection::setBlock()
    {
        FileCtlClass::setBlock(_sock_communicate);
        _sock_status = SockStatus::BLOCK;
    }

    void Connection::setNonBlock()
    {
        FileCtlClass::setNonBlock(_sock_communicate);
        _sock_status = SockStatus::NON_BLOCK;
    }

    // version2.0 修改
    // 添加了readBuffer 和 writeBuffer

    // std::optional<boost::asio::ip::tcp::socket> Connection::createFromPosixSocket(int sockfd)
    // {
    //     std::optional<boost::asio::ip::tcp::socket> ret;
    //     if (sockfd < 0)
    //     {
    //         LOG(LogLevel::ERROR) << "无效的sockfd";
    //         return ret;
    //     }
    //     ret.emplace(_io_context, boost::asio::ip::tcp::v4(), sockfd);
    //     return ret;
    // }

    // void Connection::shutDown_v2()
    // {
    //     if (_is_close == false)
    //     {
    //         if (_socket.has_value())
    //         {
    //             boost::system::error_code ec;

    //             // 关闭套接字（优雅关闭）
    //             _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    //             if (ec)
    //             {
    //                 // 处理错误，但通常可以忽略关闭时的错误
    //             }

    //             // 关闭底层套接字
    //             _socket->close(ec);
    //             if (ec)
    //             {
    //                 // 处理错误
    //             }

    //             // 重置optional（可选）
    //         }
    //         _is_close = true; // 关闭成功了
    //     }
    // }

    // // 接收二进制数据
    // void Connection::readBuffer(Buffer *pbuffer)
    // {
    //     // 确保socket是非阻塞模式

    //     std::vector<char> buffer;
    //     boost::system::error_code error;

    //     // 初始缓冲区大小
    //     constexpr size_t initial_size = 4096;
    //     buffer.resize(initial_size);

    //     size_t total_bytes_read = 0;

    //     while (true)
    //     {
    //         // 读取可用数据
    //         size_t bytes_read = _socket->read_some(
    //             boost::asio::buffer(buffer.data() + total_bytes_read,
    //                                 buffer.size() - total_bytes_read),
    //             error);

    //         total_bytes_read += bytes_read;

    //         // 如果缓冲区已满，扩大缓冲区
    //         if (total_bytes_read == buffer.size())
    //         {
    //             buffer.resize(buffer.size() * 2);
    //         }

    //         // 检查读取结束条件
    //         if (error == boost::asio::error::would_block)
    //         {
    //             // 没有更多数据可读
    //             break;
    //         }

    //         if (error)
    //         {
    //             // 真正的错误发生
    //             throw std::runtime_error("读取数据失败: " + error.message());
    //         }
    //     }


    //     // 将数据存入提供的Buffer对象
    //     if (pbuffer)
    //     {
    //         pbuffer->assign(buffer.begin(), buffer.end());
    //     }
    // }

    // // 发送二进制数据
    // void Connection::sendBuffer(Buffer &)
    // {
    // }
}
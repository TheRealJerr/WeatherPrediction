#include "Common.h"

// 自定义缓冲区
// 待完善
// 假装自己设计了一个buffer
namespace reactor
{
    class Buffer
    {
    public:
        std::vector<char>& getBuffer() { return _buffer; }

        operator std::vector<char>&() { return _buffer; }

        void resize(int size) { _buffer.resize(size); }

        void assign(std::vector<char>::iterator begin,std::vector<char>::iterator end)
        {
            _buffer.assign(begin,end);
        }
    private:
        std::vector<char> _buffer;
    };

}
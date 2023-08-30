#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <algorithm>


/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode


class Buffer
{
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize) : 
            buffer_(kCheapPrepend + initialSize),
            readIndex_(kCheapPrepend),
            writeIndex_(kCheapPrepend)
        {}

        size_t readableBytes() 
        {
            return writeIndex_ - readIndex_;
        }

        size_t writeableBytes() 
        {
            return buffer_.size() - writeIndex_;
        }

        size_t prependableBytes() {return readIndex_;}

        // 返回缓冲区中可读数据的首地址
        const char* peek() const 
        {
            return begin() + readIndex_;
        }

        void retrieve(size_t len)
        {
            assert(len <= readableBytes());
            if (len < readableBytes())
            {
                readIndex_ += len;
            }
            else
            {
                retrieveAll();
            }
        }   

        void retrieveAll()
        {
            readIndex_ = writeIndex_ = kCheapPrepend;
        }

        // 把onMessage函数上报的Buffer数据转成string
        std::string retriveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        std::string retrieveAsString(size_t len)
        {
            std::string result(peek(), len);
            retrieve(len);
            return result;
        }

        void ensureWriteableBytes(size_t len)
        {
            if (writeableBytes() < len)
            {
                makeSpace(len); // 扩容函数
            }
        }

        void append(const char* data, size_t len)
        {
            ensureWriteableBytes(len);
            std::copy(data, data + len, beginWrite());
            writeIndex_ += len;
        }

        // 从fd上读取数据到Buffer
        ssize_t readFd(int fd, int* savedErrno); 

        // 向fd写数据
        ssize_t writeFd(int sockfd, int* savedErrno);

    
    private:
        char* begin()
        {
            return &*buffer_.begin();
        }

        const char* begin() const 
        {
            return &*buffer_.begin();
        }

        void makeSpace(size_t len)
        {
            if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                buffer_.resize(writeableBytes() + len);
            }
            else
            {  
                size_t readable = readableBytes();
                // 未读取的数据前移
                std::copy(buffer_.begin() + readIndex_,
                          buffer_.begin() + writeIndex_,
                          buffer_.begin() + kCheapPrepend);
                // 更新两个指针的位置
                readIndex_ = kCheapPrepend;
                writeIndex_ = readIndex_ + readable;
            }
        }

        char* beginWrite()
        {
            return begin() + writeIndex_;
        }

        const char* beginWrite() const 
        {
            return begin() + writeIndex_;
        }
        
    private:
        std::vector<char> buffer_;
        size_t readIndex_;
        size_t writeIndex_;

};
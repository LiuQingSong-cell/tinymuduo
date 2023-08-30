#include "Buffer.h"

#include <cerrno>
#include <sys/uio.h>
#include <unistd.h>

/*
    这个函数读取fd上的数据到buffer中， 但是我们无法保证buffer中的可写空间一定
    足够，所以在栈上开一块内存作为备用 使用系统调用readv一次性填充多个
    内存区域
*/
ssize_t Buffer::readFd(int fd, int *savaedErrno)
{
    char extrabuf[65536] = {0}; // 栈上数据

    struct iovec vec[2];
    size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writeable < sizeof extrabuf)? 2 : 1;

    // readv 可以连续写入多个缓冲区块 即结构体iovec描述的区域
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savaedErrno = errno;
    }
    else if (n <= writeable) // 当前的Buffer足够
    {
        writeIndex_ += n;
    }
    else 
    {
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }
    return n;
}

ssize_t Buffer::writeFd(int sockfd, int* savedErrno)
{
    ssize_t n = ::write(sockfd, peek(), readableBytes());
    if (n < 0)
    {
        *savedErrno = errno;
    }
    return n;
}
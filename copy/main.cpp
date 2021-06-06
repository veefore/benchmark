#include <sys/stat.h> // open(), open flags
#include <fcntl.h> // open(), open flags
#include <unistd.h> // close()
#include <sys/uio.h> // readv(), writev()
#include <stdexcept>
#include <memory>
#include <cstdint>



#define QD 8
#define RS 4096
#define BufSize 1024 // == (RS / sizeof(ui32))



/*
#define QD 4
#define RS 4096
#define BufSize 1024 // == (RS / sizeof(ui32))
*/


using ui64 = uint64_t;
using ui32 = uint32_t;
using i32 = int32_t;


int main() {
    const char* filepath_in = "/home/veefore/Documents/rpi/image/28sep.img.gz";
    const char* filepath_out = "out";

    i32 fd_in;
    if ((fd_in = open(filepath_in, O_RDONLY, S_IRWXU)) == -1)
        throw std::runtime_error("Couldn't not open input file");

    i32 fd_out;
    if ((fd_out = open(filepath_out, O_WRONLY | O_CREAT, S_IRWXU)) == -1)
        throw std::runtime_error("Couldn't not open output file");

    struct iovec iov[QD];
    ui32 iovData[BufSize * QD];

    // Allocate data for buffers.
    for (ui32 i = 0; i < QD; i++) {
        iov[i].iov_base = iovData + i * BufSize;
        iov[i].iov_len = RS;
    }

    ssize_t bytesProcessed = 0;
    int iovcnt = QD;
    do {
        bytesProcessed = readv(fd_in, iov, iovcnt);
        writev(fd_out, iov, iovcnt);
    } while (bytesProcessed > 0);

    close(fd_in);
    close(fd_out);

    return 0;
}

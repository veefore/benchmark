# benchmark
File system benchmark that allows to
- generate an I/O workload (read()/readv()/write()/writev())
- design and run a system of performance tests
- make a plot out of the results

The intended use of the benchmark is to monitor file system performance to find better I/O parameters (block size, queue depth, direct I/O) based on file parameters (size, fill, location) and workload pattern (a model characterizing the read/write operations ratio and sequential/random memory access type ratio).

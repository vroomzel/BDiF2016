# BDiF2016

## Assignment A

### On Penzia, need to load higher version of g++


1. `module load g++/4.7`
2. `module unload openmpi`
3. `module load openmpi/1.8.4_gcc`
4. `mpic++ -v`

Using built-in specs.
COLLECT_GCC=/opt/centos/devtoolset-1.0/root/usr/bin//g++
COLLECT_LTO_WRAPPER=/opt/centos/devtoolset-1.0/root/usr/libexec/gcc/x86_64-redhat-linux/4.7.0/lto-wrapper
Target: x86_64-redhat-linux
Configured with: ../configure --prefix=/opt/centos/devtoolset-1.0/root/usr --mandir=/opt/centos/devtoolset-1.0/root/usr/share/man --infodir=/opt/centos/devtoolset-1.0/root/usr/share/info --with-bugurl=http://bugzilla.redhat.com/bugzilla --enable-bootstrap --enable-shared --enable-threads=posix --enable-checking=release --disable-build-with-cxx --disable-build-poststage1-with-cxx --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --enable-gnu-unique-object --enable-linker-build-id --enable-languages=c,c++,lto --enable-plugin --with-linker-hash-style=gnu --enable-initfini-array --disable-libgcj --with-ppl --with-cloog --with-mpc=/home/centos/rpm/BUILD/gcc-4.7.0-20120507/obj-x86_64-redhat-linux/mpc-install --with-tune=generic --with-arch_32=i686 --build=x86_64-redhat-linux
Thread model: posix
gcc version 4.7.0 20120507 (Red Hat 4.7.0-5) (GCC) 

### Now build and run program

1. Build executable:
 `make all`
2. Specify path to input/output files in params.txt
3. (Optional) Adjust memory usage in params.txt by changing  -io_memory parameter (in bytes). Set it to 1/3 of total available memory that this program is allowed to use.
4. Start an mpi run:
 `mpirun -n 16 main.x params.txt`

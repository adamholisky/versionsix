
## GCC config for AVSOS toolchain
../gcc-15.2.0/configure --target=x86_64-elf-avsos --prefix=/usr/local/osdev --disable-nsl --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx --with-sysroot=/avsos --enable-shared --disable-gcov

## Libsupc++ config and commands for AVSOSr
CPP=/usr/local/osdev/bin/x86_64-elf-avsos-cpp ./configure -target=x86_64-elf-avsos --prefix=/usr/local/osdev -disable-hosted-libstdcxx --disable-nls
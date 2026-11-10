FROM ubuntu:26.04
RUN ln -fs /usr/share/zoneinfo/Europe/London /etc/localtime
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt dist-upgrade --auto-remove --purge -y && \
    DEBIAN_FRONTEND=noninteractive apt-get -y install clang lld git make ninja-build cmake llvm bash curl wasi-libc libc++-dev-wasm32 libclang-rt-dev-wasm32 libc++-dev-wasm32

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get update && apt-get install -y \
    gcc-16 \
    g++-16 \
    build-essential \
    cmake \
    ninja-build \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set GCC 16 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

# Pre-clone Kokkos mdspan for std::mdspan fallback
RUN git clone https://github.com/kokkos/mdspan.git /tmp/mdspan

WORKDIR /workspace
COPY . /workspace

CMD ["/bin/bash"]

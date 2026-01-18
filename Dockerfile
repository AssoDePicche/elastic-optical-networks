FROM debian:trixie-slim AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    python3-pip \
    python3-venv \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
RUN pip install conan

WORKDIR /src
COPY . .

RUN conan profile detect --force && \
    conan install application/conanfile.txt \
    --output-folder=build \
    --build=missing \
    -s build_type=Release \
    -c tools.system.package_manager:mode=install

RUN cmake -S application -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/src/build/conan_toolchain.cmake
RUN cmake --build build

RUN cd build && ctest --output-on-failure

RUN cmake --install build --prefix /src/install
RUN strip /src/install/App

FROM scratch AS exporter
COPY --from=builder /src/install/App /App-linux-x86_64

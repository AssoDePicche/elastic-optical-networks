FROM debian:trixie-slim AS builder

ARG BUILD_TYPE=Release
ENV BUILD_TYPE=${BUILD_TYPE}

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    python3-pip \
    python3-venv \
    pkg-config \
    ccache \
    && rm -rf /var/lib/apt/lists/*

ENV CCACHE_DIR=/root/.cache/ccache
ENV CCACHE_MAXSIZE=2G

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
RUN pip install --no-cache-dir conan

WORKDIR /src

COPY application/conanfile.txt application/conanfile.txt

RUN --mount=type=cache,target=/root/.conan2/p \
    conan profile detect --force && \
    conan install application/conanfile.txt \
    --output-folder=build \
    --build=missing \
    -s build_type=${BUILD_TYPE} \
    -c tools.system.package_manager:mode=install

COPY . .

RUN --mount=type=cache,target=/root/.cache/ccache \
    --mount=type=cache,target=/root/.conan2/p \
    cmake -S application -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_TOOLCHAIN_FILE=/src/build/conan_toolchain.cmake \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
    cmake --build build && \
    ctest --test-dir build --output-on-failure && \
    cmake --install build --prefix /src/install && \
    ccache -s

RUN if [ "$BUILD_TYPE" = "Release" ]; then strip /src/install/bin/App || strip /src/install/App; fi

FROM scratch AS exporter
COPY --from=builder /src/install/ /

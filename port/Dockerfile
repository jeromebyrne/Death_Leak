FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential pkg-config libsdl2-dev libpng-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src

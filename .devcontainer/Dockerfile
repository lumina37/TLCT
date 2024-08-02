FROM debian:12

RUN sed -i 's/deb\.debian\.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list.d/debian.sources && \
    apt-get update && \
    apt-get install -y --no-install-recommends build-essential cmake unzip tar wget && \
    apt-get install -y git && \
    wget https://github.com/opencv/opencv/archive/refs/tags/4.10.0.tar.gz && \
    wget https://github.com/opencv/opencv_contrib/archive/refs/tags/4.10.0.tar.gz

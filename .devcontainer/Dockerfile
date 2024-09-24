FROM mcr.microsoft.com/devcontainers/cpp

RUN sed -i 's|deb.debian.org|mirrors.tuna.tsinghua.edu.cn|g' /etc/apt/sources.list.d/debian.sources
RUN apt-get update && \
    apt-get install -y --no-install-recommends make cmake unzip ca-certificates git wget tar && \
    apt-get clean

ARG OPENCV_VERSION=4.10.0

# OpenCV
RUN wget -O opencv.tar.gz https://github.com/opencv/opencv/archive/refs/tags/${OPENCV_VERSION}.tar.gz && \
    tar -xf opencv.tar.gz && \
    cd opencv-${OPENCV_VERSION} && \
    cmake -S . -B build -DBUILD_LIST="imgcodecs,imgproc" -DBUILD_SHARED_LIBS=OFF -DCV_TRACE=OFF -DENABLE_PRECOMPILED_HEADERS=OFF -DCPU_BASELINE=AVX2 -DCPU_DISPATCH=AVX2 -DBUILD_OpenCV_apps=OFF -DWITH_ADE=OFF -DWITH_DSHOW=OFF -DWITH_FFMPEG=OFF -DWITH_FLATBUFFERS=OFF -DWITH_GSTREAMER=OFF -DWITH_IMGCODEC_HDR=OFF -DWITH_IMGCODEC_PFM=OFF -DWITH_IMGCODEC_PXM=OFF -DWITH_IMGCODEC_SUNRASTER=OFF -DWITH_IPP=OFF -DWITH_JASPER=OFF -DWITH_JPEG=OFF -DWITH_LAPACK=OFF -DWITH_MSMF=OFF -DWITH_MSMF_DXVA=OFF -DWITH_OPENCL=OFF -DWITH_OPENEXR=OFF -DWITH_OPENJPEG=OFF -DWITH_PROTOBUF=OFF -DWITH_VTK=OFF -DWITH_WEBP=OFF -DWITH_TIFF=OFF && \
    make -C build -j$($(nproc)-1) && \
    make -C build install && \
    rm -rf opencv

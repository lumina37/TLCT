OCV_VERSION=$1
INSTALL_PREFIX=${2:-/usr/local}

wget -qO opencv.tar.gz https://github.com/opencv/opencv/archive/refs/tags/$OCV_VERSION.tar.gz && \
    tar -xf opencv.tar.gz && \
    cd opencv-$OCV_VERSION && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX -DBUILD_LIST="imgproc" -DBUILD_SHARED_LIBS=OFF -DCV_TRACE=OFF -DCPU_BASELINE=AVX2 -DCPU_DISPATCH=AVX2 -DOPENCV_ENABLE_ALLOCATOR_STATS=OFF -DWITH_ADE=OFF -DWITH_DSHOW=OFF -DWITH_FFMPEG=OFF -DWITH_IMGCODEC_HDR=OFF -DWITH_IMGCODEC_PFM=OFF -DWITH_IMGCODEC_PXM=OFF -DWITH_IMGCODEC_SUNRASTER=OFF -DWITH_IPP=OFF -DWITH_ITT=OFF -DWITH_JASPER=OFF -DWITH_JPEG=OFF -DWITH_LAPACK=OFF -DWITH_OPENCL=OFF -DWITH_OPENEXR=OFF -DWITH_OPENJPEG=OFF -DWITH_PNG=OFF -DWITH_PROTOBUF=OFF -DWITH_TIFF=OFF -DWITH_WEBP=OFF && \
    make -C build && \
    make -C build install && \
    rm -rf opencv*

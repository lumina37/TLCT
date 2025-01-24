FROM mcr.microsoft.com/devcontainers/cpp

COPY ./.devcontainer/install-opencv.sh /tmp/

ARG OPENCV_VERSION=4.11.0

RUN chmod +x /tmp/install-opencv.sh && /tmp/install-opencv.sh $OPENCV_VERSION && rm /tmp/install-opencv.sh

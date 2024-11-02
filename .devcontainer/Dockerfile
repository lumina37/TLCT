FROM mcr.microsoft.com/devcontainers/cpp

COPY ./.devcontainer/install_opencv.sh /tmp/

ARG OCV_VERSION=4.10.0

RUN chmod +x /tmp/install-opencv.sh && /tmp/install-opencv.sh $OCV_VERSION && rm /tmp/install-opencv.sh

FROM mcr.microsoft.com/devcontainers/cpp

COPY ./.devcontainer/install_opencv.sh /tmp/

RUN chmod +x /tmp/install-opencv.sh && /tmp/install-opencv.sh && rm /tmp/install-opencv.sh

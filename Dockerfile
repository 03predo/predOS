FROM ubuntu:22.04

RUN apt-get update -y && \
    apt-get upgrade -y

RUN apt-get install \
    software-properties-common \
    dos2unix \
    ripgrep \
    curl \
    cmake \
    ruby \
    fdisk \
    mtools \
    dosfstools \
    bsdmainutils \
    man-db \
    pip -y && \
    python3 -m pip install gcovr

RUN yes | unminimize

RUN apt-add-repository ppa:git-core/ppa -y && \
    apt-get update -y && \
    apt-get install git -y && \
    git config --global core.autocrlf false

RUN curl -LO https://github.com/neovim/neovim/releases/latest/download/nvim.appimage && \
    chmod u+x nvim.appimage && \
    ./nvim.appimage --appimage-extract && \
    ln -s $PWD/squashfs-root/AppRun /usr/bin/nvim && \
    update-alternatives --install /usr/bin/editor editor /usr/bin/nvim 0

RUN git clone https://github.com/03predo/nvimSetup.git

RUN mkdir -p /root/.config/nvim/lua

RUN cp /nvimSetup/init.vim /root/.config/nvim && \
    cp /nvimSetup/plugins.lua /root/.config/nvim/lua

RUN dos2unix /root/.config/nvim/init.vim && \
    dos2unix /root/.config/nvim/lua/plugins.lua

ARG ARM_TOOLCHAIN_VERSION=9.2-2019.12
ARG ARM_COMPILER=gcc-arm-$ARM_TOOLCHAIN_VERSION-x86_64-arm-none-eabi
ARG ARM_ARCHIVE=${ARM_COMPILER}.tar.xz
ARG ARM_SOURCE=https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/9.2-2019.12/binrel/$ARM_ARCHIVE

RUN curl --fail -o /$ARM_ARCHIVE $ARM_SOURCE 
RUN tar -C / -xf /$ARM_ARCHIVE && rm -rf /$ARM_ARCHIVE

WORKDIR root

RUN echo 'alias m="make"' > .bash_aliases

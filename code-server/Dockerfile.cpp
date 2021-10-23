FROM soulmachine/code-server:base

ARG USERNAME=coder

# Install common tools
RUN sudo apt -qy update && sudo apt -qy --no-install-recommends install \
    cmake \
    make \
    ninja-build

ARG LLVM_VERSION=14

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++
ENV CMAKE_EXPORT_COMPILE_COMMANDS=on

RUN echo "Installing LLVM toolchain" \
 && wget https://apt.llvm.org/llvm.sh \
 && chmod +x llvm.sh \
 && sudo ./llvm.sh $LLVM_VERSION \
 && rm ./llvm.sh \
 && sudo apt -qy update && sudo apt -qy --no-install-recommends install \
    clang-format-$LLVM_VERSION \
    clang-tidy-$LLVM_VERSION \
 && echo "Uninstalling clangd-$LLVM_VERSION because ms-vscode.cpptools has its own language server" \
 && sudo apt -qy remove --purge clangd-$LLVM_VERSION \
 && sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION 100 \
 && sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$LLVM_VERSION 100 \
 && sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-$LLVM_VERSION 100 \
 && sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-$LLVM_VERSION 100 \
 && sudo update-alternatives --install /usr/bin/lld lld /usr/bin/lld-$LLVM_VERSION 100 \
 && sudo update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-$LLVM_VERSION 100

# Install common C++ libraries
RUN sudo apt -qy update && sudo apt -qy --no-install-recommends install \
    libstdc++-10-dev \
    libboost-dev \
    libgoogle-glog-dev \
    libgoogle-perftools-dev \
    libgtest-dev \
    liblmdb-dev \
    liblzma-dev \
    libyaml-dev \
    zlib1g-dev \
    libssl-dev

RUN echo "Installing bazel" \
 && wget https://github.com/bazelbuild/bazel/releases/download/4.2.1/bazel-4.2.1-installer-linux-x86_64.sh \
 && chmod +x bazel-4.2.1-installer-linux-x86_64.sh \
 && sudo ./bazel-4.2.1-installer-linux-x86_64.sh \
 && rm ./bazel-4.2.1-installer-linux-x86_64.sh \
 && echo "source /usr/local/lib/bazel/bin/bazel-complete.bash" >> ~/.bashrc \
 && mkdir -p /home/$USER/.config/fish/completions/ \
 && ln -s /usr/local/lib/bazel/bin/bazel.fish /home/$USER/.config/fish/completions/bazel.fish \
 && echo "Installing buildifier" \
 && wget https://github.com/bazelbuild/buildtools/releases/download/4.2.2/buildifier \
 && chmod +x buildifier \
 && sudo mv buildifier /usr/local/bin \
 && echo "Installing vcpkg" \
 && git clone https://github.com/Microsoft/vcpkg.git \
 && ./vcpkg/bootstrap-vcpkg.sh \
 && sudo apt -qy autoremove && sudo apt clean && sudo rm -rf /var/lib/apt/lists/* && sudo rm -rf /tmp/*

ENV CMAKE_TOOLCHAIN_FILE="/home/$USERNAME/vcpkg/scripts/buildsystems/vcpkg.cmake"

COPY --chown=$USER:$USER ./.vscode/settings-cpp.json $XDG_DATA_HOME/code-server/User/settings.json
COPY --chown=$USER:$USER ./extensions/ms-vscode.cpptools-themes-1.0.0.vsix /tmp/
COPY --chown=$USER:$USER ./extensions/jeff-hykin.better-cpp-syntax-1.15.10.vsix /tmp/

RUN echo "Install common extensions" \
 && wget --waitretry 15 --tries 5 --no-verbose https://github.com/microsoft/vscode-cpptools/releases/download/1.7.1/cpptools-linux.vsix \
 && code-server --install-extension ./cpptools-linux.vsix \
 && rm ./cpptools-linux.vsix \
 && code-server --install-extension xaver.clang-format \
 # && code-server --install-extension notskm.clang-tidy \
 && wget --waitretry 15 --tries 5 --no-verbose https://github.com/notskm/vscode-clang-tidy/releases/download/v0.5.1/clang-tidy-0.5.1.vsix \
 && code-server --install-extension clang-tidy-0.5.1.vsix \
 && rm clang-tidy-0.5.1.vsix \
 && code-server --install-extension vadimcn.vscode-lldb \
 && code-server --install-extension twxs.cmake \
 && code-server --install-extension ms-vscode.cmake-tools \
 && code-server --install-extension BazelBuild.vscode-bazel \
 && code-server --install-extension /tmp/ms-vscode.cpptools-themes-1.0.0.vsix \
 && rm /tmp/ms-vscode.cpptools-themes-1.0.0.vsix \
 && code-server --install-extension cschlosser.doxdocgen \
 && code-server --install-extension /tmp/jeff-hykin.better-cpp-syntax-1.15.10.vsix \
 && rm /tmp/jeff-hykin.better-cpp-syntax-1.15.10.vsix \
 && code-server --install-extension TabNine.tabnine-vscode # GitHub.copilot

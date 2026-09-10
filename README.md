# BH_VISION_2027使用说明

1. 支持环境
推荐环境：
• Ubuntu 22.04
• GCC/G++ 11 或更高版本
• CMake 3.16 或更高版本
• x86_64 或 aarch64
• OpenVINO 2024.6
• 海康或迈德威视工业相机
• SocketCAN 或项目支持的串口通信
本项目依赖 Linux SocketCAN 和工业相机 SDK，不支持直接在 Windows 原生环境运行。

3. 安装基础依赖
Bash
启用自动换行
复制
sudo apt update

sudo apt install -y \
  build-essential \
  cmake \
  git \
  can-utils \
  libopencv-dev \
  libfmt-dev \
  libspdlog-dev \
  libeigen3-dev \
  libyaml-cpp-dev \
  libusb-1.0-0-dev \
  nlohmann-json3-dev \
  libceres-dev
此外还需要安装：
• OpenVINO Runtime
• HikRobot MVS SDK 或 MindVision SDK
• ROS 2 Humble，仅哨兵导航通信功能需要
3. fmt 与 spdlog 依赖约束
建议 fmt 和 spdlog 都使用 Ubuntu 系统包：
纯文本
启用自动换行
复制
/usr/lib/.../cmake/fmt
/usr/lib/.../cmake/spdlog
不要混用：
纯文本
启用自动换行
复制
Miniforge/Conda 的 fmt
/usr/local 的 spdlog
如果当前处于 Conda 环境，编译前退出：
Bash
启用自动换行
复制
conda deactivate
检查工具来源：
Bash
启用自动换行
复制
which cmake
which c++
推荐结果：
纯文本
启用自动换行
复制
/usr/bin/cmake
/usr/bin/c++
项目中统一使用 CMake target：
cmake
启用自动换行
复制
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(tools
  PRIVATE
    fmt::fmt
    spdlog::spdlog
)
不要同时使用仓库内置版本和系统版本。
4. 配置构建目录
在项目根目录执行：
Bash
启用自动换行
复制
cmake -S . -B build-system \
  -DCMAKE_BUILD_TYPE=Release
如果 CMake 错误地找到 Conda 或 /usr/local 中的依赖，可以明确指定系统包。
首先获取系统架构目录：
Bash
启用自动换行
复制
dpkg-architecture -qDEB_HOST_MULTIARCH
x86_64 Ubuntu 通常返回：
纯文本
启用自动换行
复制
x86_64-linux-gnu
然后配置：
Bash
启用自动换行
复制
cmake -S . -B build-system \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_IGNORE_PREFIX_PATH=/home/你的用户名/miniforge3 \
  -Dfmt_DIR=/usr/lib/x86_64-linux-gnu/cmake/fmt \
  -Dspdlog_DIR=/usr/lib/x86_64-linux-gnu/cmake/spdlog
aarch64 设备需要将 x86_64-linux-gnu 替换为实际输出，例如：
纯文本
启用自动换行
复制
aarch64-linux-gnu
5. 检查依赖来源
配置完成后检查：
Bash
启用自动换行
复制
grep -E "^(fmt_DIR|spdlog_DIR):" \
  build-system/CMakeCache.txt
正确结果应当全部指向 /usr/lib/...，例如：
纯文本
启用自动换行
复制
fmt_DIR:PATH=/usr/lib/x86_64-linux-gnu/cmake/fmt
spdlog_DIR:PATH=/usr/lib/x86_64-linux-gnu/cmake/spdlog
不应出现：
纯文本
启用自动换行
复制
/home/.../miniforge3
/usr/local/lib/cmake
6. 编译全部程序
Bash
启用自动换行
复制
cmake --build build-system -j$(nproc)
只编译离线自瞄测试：
Bash
启用自动换行
复制
cmake --build build-system \
  --target auto_aim_test \
  -j$(nproc)
只编译标准实车程序：
Bash
启用自动换行
复制
cmake --build build-system \
  --target standard \
  -j$(nproc)
查看所有可构建目标：
Bash
启用自动换行
复制
cmake --build build-system --target help
7. 离线自瞄测试
必须在项目根目录运行，因为配置文件和模型使用相对路径：
Bash
启用自动换行
复制
./build-system/auto_aim_test
运行前确认测试资源存在：
Bash
启用自动换行
复制
ls -lh assets/demo/demo.avi
find assets -maxdepth 2 \
  -type f \
  \( -name "*.xml" -o -name "*.bin" -o -name "*.onnx" \)
如果通过 SSH 运行，需要图形界面支持：
Bash
启用自动换行
复制
echo "$DISPLAY"
推荐使用本地桌面或 NoMachine 运行。程序可能打开 OpenCV 窗口，而不会在终端打印“测试通过”。

## 编译与测试

### 1. 推荐环境

- Ubuntu 22.04
- GCC/G++ 11 或更高版本
- CMake 3.16 或更高版本
- x86_64 或 aarch64
- OpenVINO 2024.6
- HikRobot MVS SDK 或 MindVision SDK
- SocketCAN
- ROS 2 Humble（仅哨兵导航通信功能需要）

> 本项目依赖 Linux SocketCAN、工业相机 SDK 和 Linux 动态库，不支持直接在 Windows 原生环境运行。

### 2. 安装基础依赖

```bash
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
```

此外还需要根据硬件安装：

- [OpenVINO Runtime](https://docs.openvino.ai/)
- [HikRobot MVS SDK](https://www.hikrobotics.com/)
- MindVision SDK

### 3. fmt 与 spdlog 依赖说明

建议 fmt 和 spdlog 都使用 Ubuntu 系统包：

```text
/usr/lib/<architecture>/cmake/fmt
/usr/lib/<architecture>/cmake/spdlog
```

不要混用不同环境中的依赖，例如：

```text
Miniforge/Conda 的 fmt
/usr/local 的 spdlog
```

如果当前处于 Conda 环境，编译前先退出：

```bash
conda deactivate
```

确认正在使用系统编译工具：

```bash
which cmake
which c++
```

推荐输出：

```text
/usr/bin/cmake
/usr/bin/c++
```

项目通过以下 CMake target 使用 fmt 和 spdlog：

```cmake
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(tools
  PRIVATE
    fmt::fmt
    spdlog::spdlog
)
```

不要同时使用仓库内置版本和系统安装版本。

### 4. 配置项目

在项目根目录执行：

```bash
cmake -S . -B build-system \
  -DCMAKE_BUILD_TYPE=Release
```

其中：

- `-S .` 表示源码位于当前目录；
- `-B build-system` 表示编译文件和可执行程序放在 `build-system`；
- `-DCMAKE_BUILD_TYPE=Release` 表示使用 Release 优化编译。

如果 CMake 错误地找到 Conda 或 `/usr/local` 中的 fmt/spdlog，可以明确指定系统版本。

获取当前系统架构：

```bash
dpkg-architecture -qDEB_HOST_MULTIARCH
```

x86_64 Ubuntu 通常输出：

```text
x86_64-linux-gnu
```

可以使用：

```bash
MULTIARCH="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"

cmake -S . -B build-system \
  -DCMAKE_BUILD_TYPE=Release \
  -Dfmt_DIR="/usr/lib/${MULTIARCH}/cmake/fmt" \
  -Dspdlog_DIR="/usr/lib/${MULTIARCH}/cmake/spdlog"
```

如果 Miniforge 仍然干扰依赖查找：

```bash
MULTIARCH="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"

cmake -S . -B build-system \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_IGNORE_PREFIX_PATH="/home/$USER/miniforge3" \
  -Dfmt_DIR="/usr/lib/${MULTIARCH}/cmake/fmt" \
  -Dspdlog_DIR="/usr/lib/${MULTIARCH}/cmake/spdlog"
```

### 5. 检查依赖来源

配置完成后执行：

```bash
grep -E "^(fmt_DIR|spdlog_DIR):" \
  build-system/CMakeCache.txt
```

正确结果应当类似：

```text
fmt_DIR:PATH=/usr/lib/x86_64-linux-gnu/cmake/fmt
spdlog_DIR:PATH=/usr/lib/x86_64-linux-gnu/cmake/spdlog
```

fmt 和 spdlog 应来自同一个系统环境。

不应出现一个指向 Miniforge、另一个指向 `/usr/local` 的情况：

```text
/home/.../miniforge3/lib/cmake/fmt
/usr/local/lib/cmake/spdlog
```

### 6. 编译项目

编译所有目标：

```bash
cmake --build build-system -j$(nproc)
```

只编译离线自瞄测试：

```bash
cmake --build build-system \
  --target auto_aim_test \
  -j$(nproc)
```

只编译标准实车程序：

```bash
cmake --build build-system \
  --target standard \
  -j$(nproc)
```

查看所有可构建目标：

```bash
cmake --build build-system --target help
```

编译完成后，可执行程序位于：

```text
build-system/
```

例如：

```text
build-system/auto_aim_test
build-system/camera_test
build-system/cboard_test
build-system/standard
```

### 7. 离线自瞄测试

请在项目根目录运行测试，因为配置文件、视频和模型使用相对路径：

```bash
./build-system/auto_aim_test
```

运行前确认测试视频存在：

```bash
ls -lh assets/demo/demo.avi
```

确认模型文件存在：

```bash
find assets -maxdepth 2 \
  -type f \
  \( -name "*.xml" -o -name "*.bin" -o -name "*.onnx" \)
```

`auto_aim_test` 是交互式离线视觉测试程序，通常会打开 OpenCV 窗口，不一定在终端中打印“测试通过”。

如果通过 SSH 运行，检查图形环境：

```bash
echo "$DISPLAY"
```

推荐在以下环境中运行：

- 本地 Ubuntu 桌面；
- NoMachine；
- 支持 X11 转发的 SSH。

### 8. 海康相机配置

首先通过 MVS 客户端确认：

- 相机型号；
- 相机序列号；
- 分辨率；
- 像素格式；
- 帧率；
- 曝光时间；
- 增益；
- Gamma；
- 触发模式。

使用以下命令查看 USB VID/PID：

```bash
lsusb
```

例如：

```text
Bus 002 Device 004: ID 2bdf:0001 HIKROBOT
```

对应配置：

```yaml
camera_name: "hikrobot"
exposure_ms: 2.5
gain: 16.0
vid_pid: "2bdf:0001"
```

检查相机是否连接到 USB 3.x：

```bash
lsusb -t
```

常见速度：

```text
5000M 或 10000M：USB 3.x
480M：USB 2.0
```

如果相机错误地连接到 USB 2.0，可能出现帧率不足、丢帧或高分辨率采集失败。

建议关闭自动曝光和自动增益：

```text
ExposureAuto = Off
GainAuto = Off
TriggerMode = Off
```

注意曝光单位：

```text
MVS ExposureTime：通常为微秒（μs）
配置文件 exposure_ms：毫秒（ms）
```

例如：

```text
2500 μs = 2.5 ms
```

### 9. 相机测试

修改 `configs/camera.yaml` 后运行：

```bash
./build-system/camera_test \
  -c configs/camera.yaml \
  -d
```

测试时应检查：

- 图像能否正常显示；
- 图像颜色是否正确；
- 分辨率是否正确；
- 帧率是否稳定；
- 是否存在过曝；
- 快速运动时是否存在严重拖影；
- 相机断开后能否恢复；
- 时间戳是否连续。

### 10. 相机标定

以下参数不能直接从相机中读取，必须在安装到机器人后重新标定：

```yaml
camera_matrix: [...]
distort_coeffs: [...]
R_camera2gimbal: [...]
t_camera2gimbal: [...]
R_gimbal2imubody: [...]
```

参数含义：

- `camera_matrix`：相机内参；
- `distort_coeffs`：镜头畸变；
- `R_camera2gimbal`：相机相对云台的旋转；
- `t_camera2gimbal`：相机相对云台中心的位置；
- `R_gimbal2imubody`：云台坐标系与 IMU 坐标系之间的旋转。

以下情况需要重新标定：

- 更换相机；
- 更换镜头；
- 调整焦距；
- 改变分辨率；
- 改变 ROI；
- 改变相机安装位置；
- 改变 IMU 安装方向。

### 11. 启动 SocketCAN

以下示例使用 1 Mbps CAN 波特率：

```bash
sudo ip link set can0 down 2>/dev/null || true

sudo ip link set can0 up \
  type can \
  bitrate 1000000
```

查看 CAN 接口：

```bash
ip -details link show can0
```

监听 CAN 数据：

```bash
candump can0
```

如果使用不同的波特率，请按照下位机配置修改。

### 12. C板与 IMU 测试

配置文件中需要正确填写：

```yaml
quaternion_canid: 0x100
bullet_speed_canid: 0x101
send_canid: 0xff
can_interface: "can0"
```

运行：

```bash
./build-system/cboard_test \
  configs/standard3.yaml
```

正常情况下应持续输出：

- IMU yaw；
- IMU pitch；
- IMU roll；
- 当前弹速；
- 当前控制模式。

> `CBoard` 初始化时会等待 IMU 四元数。如果下位机没有持续发送四元数，程序可能停在初始化阶段。

### 13. 标准实车程序

实车测试前，务必关闭自动开火：

```yaml
auto_fire: false
```

运行：

```bash
./build-system/standard \
  configs/standard3.yaml
```

建议按照以下顺序验证：

1. 相机图像正常；
2. IMU 数据正常；
3. yaw、pitch、roll 方向正确；
4. 装甲板检测正常；
5. 三维距离解算正确；
6. yaw、pitch 输出方向正确；
7. 云台能够安全跟随；
8. 移动目标预测正常；
9. 最后才测试自动开火。

### 14. 动态库检查

如果程序无法启动：

```bash
ldd ./build-system/auto_aim_test |
  grep -E "not found|fmt|spdlog|openvino"
```

正常情况下不能出现：

```text
not found
```

检查标准程序：

```bash
ldd ./build-system/standard |
  grep -E "not found|fmt|spdlog|openvino"
```

fmt 和 spdlog 应当来自同一个系统环境。

### 15. 常见问题

#### 找不到 fmt

典型报错：

```text
Could not find fmtConfig.cmake
```

解决：

```bash
sudo apt install --reinstall libfmt-dev
```

然后重新配置：

```bash
MULTIARCH="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"

cmake -S . -B build-new \
  -Dfmt_DIR="/usr/lib/${MULTIARCH}/cmake/fmt"
```

#### 找不到 spdlog

典型报错：

```text
Could not find spdlogConfig.cmake
```

解决：

```bash
sudo apt install --reinstall libspdlog-dev
```

重新配置：

```bash
MULTIARCH="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"

cmake -S . -B build-new \
  -Dspdlog_DIR="/usr/lib/${MULTIARCH}/cmake/spdlog"
```

#### fmt 与 spdlog 版本不匹配

典型现象：

```text
undefined reference to fmt::v...
```

检查：

```bash
grep -E "^(fmt_DIR|spdlog_DIR):" \
  build-system/CMakeCache.txt

ldd ./build-system/auto_aim_test |
  grep -E "fmt|spdlog"
```

确保二者都来自 `/usr/lib/...`，不要混用 Conda 和 `/usr/local` 版本。

#### 找不到可执行程序

错误：

```text
./build/auto_aim_test: No such file or directory
```

本项目示例使用的构建目录是 `build-system`，正确命令为：

```bash
./build-system/auto_aim_test
```

也可以查找实际位置：

```bash
find . -type f -name "auto_aim_test" -print
```

### 16. 重新构建

更换以下内容后建议使用新的构建目录：

- 编译器；
- OpenVINO 版本；
- fmt/spdlog 来源；
- 相机 SDK；
- CPU 架构；
- ROS 2 环境。

重新配置：

```bash
cmake -S . -B build-new \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-new -j$(nproc)
```

确认新构建能够正常运行后，再清理旧目录。

### 17. Git 忽略配置

不要把构建产物上传到 GitHub。

在 `.gitignore` 中加入：

```gitignore
build/
build-*/
build-system/
CMakeCache.txt
CMakeFiles/
Makefile
cmake_install.cmake
compile_commands.json
```

### 18. 测试程序说明

当前测试主要以独立可执行程序形式提供，没有全部注册到 CTest。

常用测试命令：

```bash
# 离线自瞄测试
./build-system/auto_aim_test

# 工业相机测试
./build-system/camera_test \
  -c configs/camera.yaml \
  -d

# C板、CAN、IMU和弹速测试
./build-system/cboard_test \
  configs/standard3.yaml

# 完整标准自瞄程序
./build-system/standard \
  configs/standard3.yaml
```

硬件相关测试必须在连接对应设备后进行。初次实车测试时必须关闭自动开火。

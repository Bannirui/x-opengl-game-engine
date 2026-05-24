# x-opengl-game-engine
cxx+OpenGL

mac平台用cmake的pre set不要指定toolchain，会导致find OpenGL的时候没有用apple的默认路径

## 1 HOST REQUIREMENT

### 1.1 OpenGL

```sh
sudo apt update

sudo apt install mesa-utils \
libgl1-mesa-dev \
libglu1-mesa-dev \
libglfw3-dev \
libxinerama-dev \
libxcursor-dev \
libxi-dev

glxinfo | grep "OpenGL"
```

### 1.2 Python

```sh
sudo apt install pip

sudo apt install python3.12-venv
```

## 2 网络代理

代理通过 `CMakePresets.json` 中的 `$env{http_proxy}` / `$env{https_proxy}` 从环境变量读取，不硬编码 IP。

**方式一：CLion 按 profile 设置（推荐，不污染系统环境）**

Settings → Build, Execution, Deployment → CMake → 选择 profile → Environment：

```
http_proxy=http://192.168.31.168:7890
https_proxy=http://192.168.31.168:7890
```

**方式二：命令行临时设置**

```sh
http_proxy=http://192.168.31.168:7890 https_proxy=http://192.168.31.168:7890 cmake --preset linux-gcc-debug
```

**方式三：shell 配置文件（不推荐）**

```sh
export http_proxy=http://192.168.31.168:7890
export https_proxy=http://192.168.31.168:7890
```

## 3 编译

- 理论上，只要上面网络代理配置没有问题，直接用clion的内置cmake工具不会再有问题，下面的内容就不用看了
- 如果有问题就用命令行执行cmake命令，先生成make target，然后clion便能认识这些target继而进行后面的编译

### 3.1 配置

```shell
cmake \
-S . \
--preset linux-gcc-debug
```

### 3.2 构建

```shell
cmake --build build/linux-gcc-debug --target all
```

然后在clion的面板就会出现cmake目标，之后的编译就可以用clion操作了
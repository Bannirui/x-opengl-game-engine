# x-opengl-game-engine
cxx+OpenGL

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

- 1 复制根目录下的CMakeUserPresets.json.template文件，重名名CmakeUserPresets.json
- 2 修改environment下的http_proxy和https_proxy为自己的代理

## 3 编译

- 理论上，只要上面网络代理配置没有问题，直接用clion的内置cmake工具不会再有问题，下面的内容就不用看了
- 如果有问题就用命令行执行cmake命令，先生成make target，然后clion便能认识这些target继而进行后面的编译

### 3.1 配置

```shell
cmake \
-S . \
--preset linux-gcc-debug
```

### 3.2 编译

```shell
cmake --build build/linux-gcc-debug --target all
```

然后在clion的面板就会出现cmake目标，之后的编译就可以用clion操作了
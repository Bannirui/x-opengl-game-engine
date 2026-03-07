# x-opengl-game-engine
cxx+OpenGL

mac平台用cmake的pre set不要指定toolchain，会导致find OpenGL的时候没有用apple的默认路径

## 1 HOST REQUIREMENT

### 1.1 OpenGL

greater or equal 4.6

### 1.2 Vulkan

```shell
sudo apt update
sudo apt upgrade

sudo apt install \
vulkan-tools \
libvulkan1 \
vulkan-validationlayers \
mesa-vulkan-drivers \
libvulkan-dev \
mesa-utils

vkcube
```

### 1.3 Shaderc

```sh
sudo apt install libshaderc-dev \
spirv-tools \
glslang-dev \
libspirv-cross-c-shared-dev
```
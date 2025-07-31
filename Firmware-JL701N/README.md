# Buddie-Firmware-JL701N


本仓库包含 Buddie AI 耳机的固件源码与技术文档，适用于基于杰理 AC701N 芯片的开发板。该固件方案专为**低功耗音频实时传输**设计，内置高效的音频压缩模块。

配合我们开源的手机端 AI 应用，可实现以下核心功能：

- 实时转录说话内容
- 通过耳机和AI助手进行语音交互
- 在线上会议转录我和其他人的说话内容



## 🛠 系统需求

- **操作系统:** Windows 10 及以上版本 (64位系统推荐)
- **硬件:** 
  - 杰理 AC701N 开发板
  - 强制下载工具
- **其他：**USB **数据线**（type-A） 

## 📚 准备工作

本项目，我们建议在 Windows 环境下使用 VSCode 进行编译。环境搭建过程如下：

1. [配置windows系统上的开发环境](#1-配置-Windows-开发环境)  
2. [VSCode上的开发环境](#2-使用-vscode-编译SDK)
3. [使用强制下载工具烧录固件](#3-使用强制下载工具烧录固件)

### 1 配置 Windows 开发环境

该 SDK 项目专为 **Windows 系统** 设计，默认使用 **Code::Blocks** 作为开发环境。

整个配置流程分为三个主要步骤：

1. **下载并安装 [Code::Blocks 的 Windows 版本](https://pkgman.jieliapp.com/s/codeblocks)**

2. **首次打开 Code::Blocks 并立即关闭**
   此操作将生成必要的配置文件，为后续开发做准备。

3. **下载并安装 [最新版本的杰理 Windows 工具链](https://pkgman.jieliapp.com/s/win-toolchain)**
   [点击此处下载]

完成以上步骤后，您就可以打开 Code::Blocks 项目，开始编译和开发工作。（推荐使用VSCode进行编译和开发工作）

如果需要更多的工具链和后处理工具，请参考： **[最新工具版本](https://doc.zh-jieli.com/Tools/zh-cn/other_info/index.html)**。

如需获取更详细的程序开发相关工具说明，请点击下方链接：  
https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html

### 2 使用 VSCode 编译SDK

使用 VSCode 编译是通过调用 `make` 命令实现的。

#### 2.1 在SDK根目录下使用VSCode打开项目
<p align="center">
  <img src="../image/firmware/firmware_open_vscode.jpg" width="400" />
</p>



#### 2.2 安装必要的扩展插件：**Task Explorer** 和 **C/C++**

<p align="center">
  <img src="../image/firmware/firmware_vscode_task.jpg" width="400" />
</p>



<p align="center">
  <img src="../image/firmware/firmware_vscode_c_cpp_ext.jpg" width="400" />
</p>



#### 2.3 选择对应的任务进行编译

点击 **TASK EXPLORER > SDK > vscode**，即可查看可用的任务列表：

- **all**：编译整个项目
- **clean**：清除编译输出文件

<p align="center">
  <img src="../image/firmware/firmware_vscode_build.jpg" width="400" />
</p>



### 3 使用强制下载工具烧录固件

#### 3.1 连接电脑和开发板

将强制下载工具的 USB 母口连接至电脑，USB 公口连接至原型板或开发板。
 （**注意：** 请勿连接反向，具体接法请参考下方图片。）

1. 具体操作步骤如下：
   1. 按照图片中的指引正确连接设备。
   2. 强制下载工具上的绿灯和红灯会开始闪烁。
   3. 按下强制下载工具上的按键——绿灯熄灭，红灯常亮。
   4. 此时即可将程序烧录到开发板上。

<p align="center">
  <img src="../image/firmware/firmware_board_connect.jpg" width="400" />
</p>


**更多详细信息**，请参考：[**升级与下载说明**](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/upgrade_and_download.html)。

#### 3.2 烧录固件

在连接好电脑和开发板之后，按下强制下载工具上的按键。在强制下载工具上**绿灯熄灭，红灯常亮**的状态下，开始烧录。

1. 点击 [**all** task](#23-选择对应的任务进行编译) ，会自动执行编译和烧录任务

2. 当终端显示下载完毕生成UFW文件时，表示烧录完成。
3. 再次按下强制下载工具上的按键，绿灯和红灯会开始闪烁，开发板运行烧录的程序。

<p align="center">
  <img src="../image/firmware/firmware_flashing.png" width="400" />
</p>



---

## 🚀 入门指南

1.  **获取代码:**
    ```bash
    git clone https://github.com/Buddie-AI/Buddie.git
    cd Buddie/Firmware-JL701N
    ```

2.  **编译与烧录固件:**
    
    *   参考[📚-准备工作](#📚-准备工作)

## 📖 文档

本仓库是基于杰理AC701N的固件代码进行开发的。因此文档将分为两部分：杰理官方文档和Buddie新增内容文档。

[技术文档](bud.inc)提供更加详细的说明

*   **杰理官方文档：**
    *   **AC701N 芯片 Datasheet:** 包含芯片的电气特性、引脚定义、功能模块等核心硬件信息。 *(通常包含在 SDK 包中或需从杰理获取)*
    *   **AC701N SDK 开发手册:** 详细说明 SDK 架构、API 接口、外设驱动、BLE 协议栈使用、开发流程等。 *(SDK 包中最重要的文档)*
    <!-- *   **杰理开发工具 (IDE) 用户手册:** 介绍 IDE 的使用方法、工程配置、编译、调试、烧录等功能。 *(通常包含在 IDE 安装包或 SDK 中)* -->
    *   *请查看仓库根目录下的 `doc/html` 文件夹获取详细的项目文档。*
    <!-- *   *请务必查阅您下载的 SDK 包内的 `doc` 或 `docs` 目录获取最准确和最新的官方文档。* -->

*   **Buddie新增内容：**
    *   **获取麦克风数据**
    *   **获取扬声器数据**
    *   **新建的任务进程："pca", "vad_task"**
    *   **数据处理及压缩：**
        *   **快速傅里叶变换：**
    *   **蓝牙数据发送：**
        *   **通过BLE进行发送数据：**
        *   **通过SPP进行发送数据：**
    *   **其他小功能：**
        *   **修改经典蓝牙/BLE广播名称：**
        *   **调整/锁定芯片主频：**
        *   **修改BLE characteristics：**
    

## ❓ 问题与支持

如果您在使用本仓库或开发过程中遇到任何问题，请先查阅 **[技术文档](./docs/FAQ.md)** 和 **杰理官方文档**。

如果问题仍未解决，欢迎在仓库的 **[Issues](https://github.com/你的用户名/你的仓库名/issues)** 页面提交您的问题。请尽量清晰地描述问题现象、复现步骤、您已尝试过的解决方法以及相关的环境信息（如 SDK 版本、IDE 版本、开发板型号等）。

---

## 📜 许可证

本项目采用 **MIT 许可证** - 有关详细信息，请参阅 [LICENSE](LICENSE) 文件。

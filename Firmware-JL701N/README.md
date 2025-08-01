<p align="center">
  <a href="README.md">English</a> | <a href="README.zh_CN.md">简体中文</a>
</p>

# Buddie-Firmware-JL701N

This repository contains the firmware source code and technical documentation for Buddie AI headphones, suitable for development boards based on the JieLi AC701N chip. This firmware solution is specifically designed for **low-power real-time audio transmission** and features a built-in efficient audio compression module.

When used together with our open-source mobile AI application, the following core functions can be achieved:

- Real-time transcription of spoken content
- Voice interaction with the AI assistant via the headphones
- Transcription of both your own and others' speech during online meetings

## 🛠 System Requirements

- **Operating System:** Windows 10 or later (64-bit system recommended)
- **Hardware:** 
  - JieLi AC701N development board
  - Forced download tool
- **Other:** USB **data cable** (type-A)

## 📚 Preparation

For this project, we recommend using VSCode for compilation in a Windows environment. The environment setup process is as follows:

1. [Configure the development environment on Windows](#1-configure-windows-development-environment)  
2. [Development environment in VSCode](#2-build-sdk-in-vscode)
3. [Burn firmware using the forced download tool](#3-burn-firmware-using-the-forced-download-tool)

### 1 Configure Windows Development Environment

This SDK project is designed **specifically for Windows systems** and uses **Code::Blocks** as the default development environment.

The entire configuration process is divided into three main steps:

1. **Download and install [the Windows version of Code::Blocks](https://pkgman.jieliapp.com/s/codeblocks)**

2. **Open Code::Blocks for the first time and close it immediately**  
   This operation will generate the necessary configuration files for subsequent development.

3. **Download and install [the latest JieLi Windows toolchain](https://pkgman.jieliapp.com/s/win-toolchain)**  
   [Click here to download]

After completing the above steps, you can open the Code::Blocks project and start compiling and developing. (It is recommended to use VSCode for compilation and development.)

If you need more toolchains and post-processing tools, please refer to: **[Latest tool versions](https://doc.zh-jieli.com/Tools/zh-cn/other_info/index.html)**.

For more detailed information about development tools, please click the link below:  
https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html

### 2 Build SDK in VSCode

Building in VSCode is done by invoking the `make` command.

#### 2.1 Open the project in VSCode at the SDK root directory
<p align="center">
  <img src="../image/firmware/firmware_open_vscode.jpg" width="400" />
</p>

#### 2.2 Install the necessary extensions: **Task Explorer** and **C/C++**

<p align="center">
  <img src="../image/firmware/firmware_vscode_task.jpg" width="400" />
</p>

<p align="center">
  <img src="../image/firmware/firmware_vscode_c_cpp_ext.jpg" width="400" />
</p>

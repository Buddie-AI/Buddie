<p align="center">
  <a href="README.md">English</a> | <a href="README.zh_CN.md">简体中文</a>
</p>

# Buddie 🌱

**BUDDIE** 是一款专为成为你集中化生活中心而设计的移动应用——无缝倾听你的所有个人信息，成为真正理解你的智能 AI 助手，无论是在工作中还是日常生活中。
为了获得最佳体验，请搭配我们专用的 Buddie AI 耳机使用。

## ✨ 核心功能

#### 🤝 Buddie 作为你的会议个人助手
- 一体化会议转录：Buddie 转录电话通话、在线会议和离线对话——支持 Zoom、Microsoft Teams 等任何支持耳机的会议应用。
- AI 驱动的会议摘要：Buddie 转录并总结你的会议、通话和对话，捕捉关键点和行动项目。这些摘要会被存储，你可以随时轻松检索。
- 会议中的即时提示：通过单次点击，Buddie 为你提供与当前对话相关的信息，帮助你回答问题、检索互联网信息。

#### 🗣️ Buddie 作为你的语音交互日常助手
- 真正的免提体验：享受与尖端 LLM（如 ChatGPT）的无缝免提语音交互，无需拿出手机或按按钮。
- 上下文感知对话：Buddie 始终在倾听（在你的同意下），所以当你寻求帮助时，它已经理解了情况——无需重复背景。
- 个性化助手：Buddie 会随时间学习你的偏好，提供量身定制的建议并回顾你生活中的难忘时刻。

#### 📱 跨平台支持
- Buddie 在 iOS 和 Android 上都能无缝工作。

## 🚀 快速开始

### 前置要求
- Flutter 3.32.4 • channel stable • https://github.com/flutter/flutter.git
  Framework • revision 6fba2447e9 • 2025-06-12 19:03:56 -0700
  Engine • revision 8cd19e509d • 2025-06-12 16:30:12 -0700
  Tools • Dart 3.8.1 • DevTools 2.45.1 （Flutter 3.32.*）
  请参考官方 Flutter 文档：设置你的 Flutter 开发环境(https://docs.flutter.dev/get-started/install)
- Android Studio / Xcode

### 安装
```bash
# 克隆仓库
git clone https://github.com/Buddie-AI/Buddie.git
cd Buddie/APP

# 安装依赖
flutter pub get

# 发布 Android apk
flutter build apk --release
详细文档请参考 Flutter 官方文档构建 Android 应用。(https://docs.flutter.dev/deployment/android)

# 发布 iOS apk
flutter build ipa
构建和发布 iOS 应用需要 Xcode，因此你必须有一台运行 macOS 的电脑。
还需要 Apple Developer 账户。
详细文档请参考 Flutter 官方指南构建 iOS 应用。(https://docs.flutter.dev/deployment/ios)

# 在 Android 或 Android 模拟器上启动
flutter run
```

## 📖 用户指南

1. 首次启动时，应用会引导你完成 30 秒的声纹注册。
2. 要使用 Buddie 耳机的增强语音功能：
  - 在应用中打开设置
  - 选择"连接"
  - 确保耳机已在系统中配对
  - 在应用中确认连接

## 🛠️ 技术栈

- **前端框架**：Flutter
- **状态管理**：Provider
- **数据库**：ObjectBox

## 🧑‍💻 贡献

我们欢迎贡献！请按照以下步骤：
1. Fork 仓库
2. 创建你的功能分支 (git checkout -b feature-AmazingFeature)
3. 提交更改 (git commit -m 'Add some AmazingFeature')
4. 推送到分支 (git push origin feature-AmazingFeature)
5. 开启 Pull Request

## 🧭 下一步

以下是 Buddie 即将到来的里程碑和计划：

## 📄 许可证

基于 MIT 许可证分发。详情请参阅 [LICENSE](LICENSE)。

## �� 致谢

我们要感谢以下个人和项目给予的灵感、支持和贡献：

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/MemX-Workspace/Bud-App/pulls)
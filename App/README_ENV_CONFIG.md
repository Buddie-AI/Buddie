# 环境变量配置指南

本项目集成了 `flutter_dotenv` 库，允许您通过 `env` 文件配置API keys的默认值。

## 设置步骤

### 1. 创建 env 文件

在项目根目录创建 `env` 文件（与 `pubspec.yaml` 同级）：

```bash
# 在项目根目录执行
touch env
```

### 2. 配置环境变量

在 `env` 文件中添加以下配置，将示例值替换为您的实际API keys：

```env
# 自定义LLM配置 (OpenAI兼容)
DEFAULT_LLM_TOKEN=sk-your_openai_api_key_here
DEFAULT_LLM_URL=https://api.openai.com/v1/chat/completions
DEFAULT_LLM_MODEL=gpt-4o

# 阿里云DashScope API Key (用于QwenOmni多模态)
DEFAULT_ALIBABA_API_KEY=sk-your_alibaba_dashscope_api_key_here

# 腾讯云ASR配置 (用于云端语音识别)
DEFAULT_TENCENT_SECRET_ID=your_tencent_secret_id_here
DEFAULT_TENCENT_SECRET_KEY=your_tencent_secret_key_here
DEFAULT_TENCENT_TOKEN=your_tencent_token_here

# OpenAI TTS配置 (用于语音合成)
DEFAULT_OPENAI_TTS_BASE_URL=https://api.openai.com/v1/audio/speech
```
## 工作原理

### 配置优先级

应用会按以下优先级使用配置：

1. **用户设置** - 用户在应用设置中配置的API keys（最高优先级）
2. **环境变量** - `env` 文件中的默认值
3. **无配置** - 如果以上都没有，功能将不可用

### 支持的配置项

| 环境变量名 | 用途 | 示例值 |
|-----------|------|--------|
| `DEFAULT_LLM_TOKEN` | OpenAI兼容API的Token | `sk-proj-xxx...` |
| `DEFAULT_LLM_URL` | LLM API端点URL | `https://api.openai.com/v1/chat/completions` |
| `DEFAULT_LLM_MODEL` | 默认使用的模型 | `gpt-4o` |
| `DEFAULT_ALIBABA_API_KEY` | 阿里云DashScope API Key | `sk-xxx...` |
| `DEFAULT_TENCENT_SECRET_ID` | 腾讯云Secret ID | `AKIDxxx...` |
| `DEFAULT_TENCENT_SECRET_KEY` | 腾讯云Secret Key | `xxx...` |
| `DEFAULT_TENCENT_TOKEN` | 腾讯云Token | `xxx...` |
| `DEFAULT_OPENAI_TTS_BASE_URL` | OpenAI TTS服务URL | `https://api.openai.com/v1/audio/speech` |

## 使用场景

### 开发环境

在开发环境中，您可以：
- 配置测试用的API keys作为默认值
- 避免每次重新安装应用时都要重新配置
- 让团队成员快速上手，无需手动配置

### 企业部署

在企业环境中，您可以：
- 预配置企业的API keys
- 允许用户覆盖某些配置
- 统一管理API配额和访问控制

## 故障排除

### env 文件不生效

1. 确认 `env` 文件位于项目根目录
2. 检查文件编码为 UTF-8
3. 确认变量名拼写正确
4. 重启应用以重新加载配置

### 调试配置状态

在Debug模式下，应用启动时会在控制台输出配置状态：

```
DefaultConfig: env文件加载成功
=== DefaultConfig 状态 ===
初始化状态: true
默认LLM配置: ✓
默认阿里云配置: ✓
默认腾讯云配置: ✗
LLM URL: https://api.openai.com/v1/chat/completions
LLM Model: gpt-4o
OpenAI TTS URL: https://api.openai.com/v1/audio/speech
========================
```

### 常见问题

**Q: 我配置了 env 但应用还是提示缺少API Key？**
A: 检查环境变量名是否正确，确保没有多余的空格或特殊字符。

**Q: 用户设置的API Key和默认值冲突了？**
A: 用户设置的优先级更高，会覆盖默认值。这是预期行为。

**Q: 可以配置部分环境变量吗？**
A: 可以，每个环境变量都是独立的。未配置的环境变量会使用空值。

## 示例配置

以下是一个完整的 `env` 配置示例：

```env
# OpenAI配置
DEFAULT_LLM_TOKEN=sk-proj-example123
DEFAULT_LLM_URL=https://api.openai.com/v1/chat/completions
DEFAULT_LLM_MODEL=gpt-4o

# 阿里云配置
DEFAULT_ALIBABA_API_KEY=sk-alibaba-example456

# 如果不需要腾讯云ASR，可以留空或删除这些行
# DEFAULT_TENCENT_SECRET_ID=
# DEFAULT_TENCENT_SECRET_KEY=
# DEFAULT_TENCENT_TOKEN=

# TTS配置
DEFAULT_OPENAI_TTS_BASE_URL=https://api.openai.com/v1/audio/speech
```

有了这个配置，用户首次使用应用时就能直接使用AI功能，无需手动配置API keys！ 
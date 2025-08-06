<p align="center">
  <a href="README.md">English</a> | <a href="README.zh_CN.md">简体中文</a>
</p>
# PCB Design Documentation

The PCB folder contains design files and documentation related to the Buddie project hardware. Below is a detailed description of the contents in this folder:

## 📋 File Structure

### 🎛️ Schematic Files
- **PCB.pdf**: Schematic file for the main circuit of Buddie's first-generation product, detailing the electrical connections and core circuit design of the mainboard.
- **FPC_L.pdf**: Schematic for the left FPC (Flexible Printed Circuit) of Buddie's first-generation product, describing the connection relationships of the left ear-related circuits.
- **FPC_R.pdf**: Schematic for the right FPC (Flexible Printed Circuit) of Buddie's first-generation product, describing the connection relationships of the right ear-related circuits.

### 🖥️ PCB Layout Files
- **E001-V2.1.PcbDoc**: PCB layout file for the main circuit board, designed using Altium Designer software, containing complete circuit board layout, routing, and component placement.
- **FPC-L-V2.1.PcbDoc**: PCB layout file for the left flexible circuit board, specifically designed for the left ear, containing audio processing, sensor, and connector layouts.
- **FPC-R-V2.1.PcbDoc**: PCB layout file for the right flexible circuit board, specifically designed for the right ear, symmetrical to the left ear layout but optimized for right ear functionality.

### 📦 Production Files
- **YYM001-LR-V2 贴片资料-20250305.rar**: SMT production file package for the main circuit board, containing coordinate files, stencil files, and other files required for SMT assembly.
- **E001-FPC-L-V2贴片资料-20250305.rar**: SMT production file package for the left FPC.
- **E001-FPC-R-V2贴片资料-20250305.rar**: SMT production file package for the right FPC.

### 📁 Archive Files
- **YYM-001-LR-V2-20250304.zip**: Complete production file package for the main circuit board, containing Gerber files, drill files, and more.
- **E001-FPC-L-V2-20250304.rar**: Complete production file package for the left FPC.
- **E001-FPC-R-V2-20250304.rar**: Complete production file package for the right FPC.

## 🔧 Design Features

### Mainboard Design (E001-V2.1)
- **Processor**: Integrated Jieli chip supporting audio processing and AI algorithms
- **Audio System**: High-quality audio codec supporting ANC noise cancellation
- **Connectivity**: Bluetooth 5.0 and WiFi module integration
- **Power Management**: Efficient power management circuit supporting fast charging
- **Sensor Interface**: Support for multiple sensor connections, enabling intelligent sensing

### Flexible Circuit Board Design (FPC-L/R-V2.1)
- **Audio Processing**: Dedicated audio processing circuit supporting high-fidelity audio output
- **Sensor Integration**: Integrated accelerometer, gyroscope, and other motion sensors
- **Connector Design**: Reliable connector design ensuring signal transmission stability
- **Flexible Design**: Adapts to the bending and wearing requirements of earphones

## 📐 Technical Specifications

### Mainboard Specifications
- **Layers**: 4-layer PCB
- **Material**: FR-4 fiberglass board
- **Thickness**: 1.6mm
- **Minimum Trace Width**: 0.1mm
- **Minimum Hole Diameter**: 0.2mm

### FPC Specifications
- **Layers**: 2-layer flexible circuit board
- **Material**: Polyimide substrate
- **Thickness**: 0.1mm
- **Bend Radius**: Minimum 3mm

## 🛠️ Usage Instructions

### Viewing Design Files
1. **Schematic Viewing**: Use PDF reader to open `.pdf` files
2. **PCB Layout Viewing**: Use Altium Designer to open `.PcbDoc` files
3. **Production Files**: Can be sent directly to PCB manufacturers for production

### Production Preparation
1. **Gerber Files**: Extract from archive packages, containing complete production information
2. **Assembly Files**: Contains component coordinates and stencil design
3. **BOM List**: Detailed component list for procurement

## 📞 Technical Support

For technical support or design-related questions, please contact us through:
- **GitHub Issues**: [Submit Issues](https://github.com/Buddie-AI/Buddie/issues)
- **Discord**: [Join Discussion](https://discord.gg/hSDEbnqB)
- **Email**: public@memx.life

## 📄 License

This project is licensed under the MIT License. See [LICENSE](../LICENSE) file for details.

---

> **Note**: All design files are part of the open-source project and welcome community contributions and improvements. Specific file names and content may be adjusted according to actual project progress. Please refer to the actual file list for verification.

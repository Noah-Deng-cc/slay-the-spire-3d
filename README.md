# SS3D — 记忆尖塔
https://github.com/Noah-Deng-cc/slay-the-spire-3d
**2D+3D 卡牌 Roguelike | 记忆尖塔**

> 🎯 大二寒假实习作品集项目
> 🛠 Unreal Engine 5 + C++ + UMG
> 📅 2026年8月重构为 UE5 版本

## 当前主线状态

`C:\Users\Noah\Desktop\Projects\SS3D` 是唯一主工程。`C:\Users\Noah\Documents\Unreal Projects\ss3d` 仅作为外部资源导入实验项目，不能替代主工程。

当前可以不依赖正式美术资源完整运行一局：C++ 会生成白膜战斗场景和玩家/敌人占位体，地图、战斗、奖励、商店、休息、事件、三层 Boss 和通关流程均由规则代码驱动。

模块能力和人工输入边界见 `Docs/ModuleInventory.md`，资源限制和借物表见 `Docs/Attribution.md`，验收命令见 `Docs/Verification.md`。
项目的目录树、依赖方向和逐阶段开发门槛见 `Docs/Architecture.md`。后续功能必须按该框架从规则、用例、表现到资源逐层实现。

---

## 🎮 游戏概述

- **类型**: 单人卡牌 Roguelike
- **核心玩法**: 回合制卡牌战斗 + 随机地图爬塔
- **视觉形式**: 2D UI 操作 + 3D 角色战斗演出
- **玩法参考**: Slay the Spire 2 的卡牌、地图和 Roguelike 节点结构
- **世界观**: 玩家被困在神秘的记忆尖塔，抵达塔顶后才能离开

## 🏗 技术栈（当前实现）

| 层 | 选型 | 用途 |
|----|------|------|
| 引擎 | Unreal Engine 5 | 场景、生命周期、输入和构建 |
| 语言 | C++ | 游戏逻辑、数据模型和运行时系统 |
| UI | UMG / Slate | 地图、战斗 HUD、卡牌和节点交互 |
| 渲染 | Unreal Renderer | 3D 战斗场景、材质、动画和特效 |
| 架构 | Actor / UObject + 委托 + 状态机 | 模块解耦、游戏状态流转、UI 状态同步 |
| 版本控制 | Git + GitHub | 代码与迭代记录 |

> 当前仓库的第一阶段优先实现 UE5 C++ 后端和可验证的 UI 逻辑。3D 模型、贴图、动画和音频资源暂未提交，表现层以可替换接口为边界。

### 当前实现基线

- `Source/SS3D` 是 UE5 C++ 模块。
- 当前已有战斗后端、地图生成、运行时 Slate/UMG HUD 和控制台驱动的完整三层循环。
- 第一角色奥黛塔的初始卡组为：打击 x5、防御 x4、痛击 x1，共 10 张；每回合 3 点能量。
- 主要边界：`SS3DGameMode` / `SS3DGameState`（流程和阶段）、`MapManager`（地图逻辑）、`CombatManager`（战斗逻辑）、UMG Widget（表现层）。

### 运行入口

在 UE5 PIE 的控制台执行：

```text
SS3D character odette
SS3D new 1337
SS3D map
SS3D select <nodeId>
SS3D hand
SS3D play <cardIndex>
SS3D end
SS3D reward <index>
```

当前还提供四条回归命令：`SS3D nodes` 检查三层地图的节点类型覆盖和连接合法性；`SS3D effects` 检查易伤、虚弱、中毒、敌方 Buff、药水击杀和胜利藏品；`SS3D demo` 使用内置自动策略，自动选择路线、处理战斗和奖励，并输出三层通关结果；`SS3D audit` 一次运行规则、阶段、白膜和完整流程验收。

其他命令：`SS3D status`、`SS3D potion <index>`、`SS3D shop`、`SS3D buy card/relic/potion/remove <index>`、`SS3D rest heal/upgrade`、`SS3D event 0/1`、`SS3D window <width> <height>`。当前版本已接入运行时 Slate/UMG HUD，不依赖外部美术资源。

鼠标可点击地图节点、卡牌和所有阶段按钮；键盘支持 `Esc` 切换鼠标模式、`Enter` 确认、`Space` 结束回合、`M` 查看地图、`H` 查看手牌。

### 当前明确缺口

- 3D 战斗场景、玩家模型、敌人模型、卡牌立绘、动画、粒子和音频资源：仓库中不存在，当前以规则和 UI 验证为主。
- 阶段 UI、规则、输入、窗口设置、白膜场景和完整三层流程已经由代码完成；自动验收包含 `UI PASS`、`INPUT PASS`、`WINDOW PASS`、`DEFEAT PASS` 和全阶段覆盖。
- 正式角色、敌人、动画、UI 美术、特效和音频属于人工资源输入；缺失资源不模拟。
- `Local/` 中的模型源文件仅限本地测试，已被 Git 忽略，不会同步到 GitHub。
- `SS3DEditor` 和 Game Target 均已通过 UE5.8 编译。
- 已完成 Windows Cook/Stage/Pak/Archive，归档目录为 `StagedBuild/Windows`。
- 已运行归档程序的 `SS3D demo` 回归，日志输出 `DEMO PASS：三层尖塔完整通关`。

## 📁 项目结构

完整技术框架见 `Docs/Architecture.md`。下面只列出当前仓库的稳定入口；目标目录与迁移边界不在这里重复维护。

```
Source/
├── SS3D.Target.cs       # UE5 Game Target
├── SS3DEditor.Target.cs # UE5 Editor Target
└── SS3D/
    ├── SS3D.Build.cs       # UE5 模块配置
    ├── CombatTypes.h       # 当前基线：卡牌、战斗、资源和敌人数据类型
    ├── CombatManager.*     # 战斗运行时逻辑
    ├── MapTypes.h          # 地图节点、层和路线数据
    ├── MapManager.*        # 地图生成与路径选择
    ├── CardLibrary.*       # 卡牌库和奥黛塔初始卡组
    ├── RelicLibrary.*      # 藏品库
    ├── PotionLibrary.*     # 药水库
    ├── EnemyLibrary.*      # 敌人和 Boss 库
    ├── Core/               # 游戏阶段和检查点
    ├── Presentation/       # 白膜场景和可替换表现
    ├── SS3DGameMode.*      # 纯代码游戏流程
    └── SS3DPlayerController.* # 控制台命令入口

> 说明：`Domain/`、`Application/`、`Platform/` 是目标职责边界。当前代码为保持已通过验收的基线，部分文件暂留在模块根目录，按 `Docs/Architecture.md` 的映射逐步迁移。

Content/
├── Maps/               # UE5 场景
├── Blueprints/         # 可视化表现和资源组装
├── UI/                 # UMG Widget
├── Characters/         # 角色和敌人资源
└── FX/                 # 动画、材质和特效
```

## 一键验收

关闭 UE 编辑器后，在项目根目录执行：

```powershell
$ProjectFile = (Resolve-Path ".\SS3D.uproject").Path
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" SS3DEditor Win64 Development $ProjectFile -WaitMutex -FromMsBuild
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" SS3D Win64 Development $ProjectFile -WaitMutex -FromMsBuild
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $ProjectFile -game -log -nosplash -unattended -ExecCmds="SS3D audit,quit" -stdout -FullStdOutLogOutput
```

验收必须看到 `WHITEBOX PASS`、`UI PASS`、`INPUT PASS`、`WINDOW PASS`、`NODES PASS`、`EFFECTS PASS`、`DEFEAT PASS`、`PHASE PASS`、`PHASE COVERAGE PASS`、`DEMO PASS`、`FLOW PASS` 和 `AUDIT PASS`。完整工作流和人工边界记录在 `Docs/`。

## 🚀 开发计划

- [x] P1: UE5 C++ 工程基线 + 地图数据结构
- [x] P2: 三层尖塔地图生成 + 进入游戏后的路径选择
- [x] P3: 战斗垂直切片 + 节点进入和结算
- [x] P4: 奖励、商店、事件、休息、藏品和药水
- [x] P5: Editor/Cook/打包验收 + 纯代码地图/战斗流程
- [x] P5: 运行时 Slate/UMG 地图、战斗、奖励、商店、事件、休息和结算界面
- [ ] P6: 正式 3D 战斗表现 + 美术资源驱动的 UI 打磨

---

*Built with ❤️ and lots of AI assistance*

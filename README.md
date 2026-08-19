# SS3D — 记忆尖塔
https://github.com/Noah-Deng-cc/slay-the-spire-3d
**2D+3D 卡牌 Roguelike | 记忆尖塔**

> 🎯 大二寒假实习作品集项目
> 🛠 Unreal Engine 5 + C++ + UMG
> 📅 2026年8月重构为 UE5 版本

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
- 当前已有战斗后端、地图生成和控制台驱动的完整三层循环；正式 UMG 仍待实现。
- 第一角色奥黛塔的初始卡组为：打击 x5、防御 x4、痛击 x1，共 10 张；每回合 3 点能量。
- 主要边界：`GameInstance` / 运行状态、`MapManager`（地图逻辑）、`CombatManager`（战斗逻辑）、UMG Widget（表现层）。

### 纯代码运行入口

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

输入 `SS3D demo` 可运行内置自动策略，自动选择路线、处理战斗和奖励，并输出三层通关结果，用于后端回归验收。

其他命令：`SS3D status`、`SS3D potion <index>`、`SS3D shop`、`SS3D buy card/relic/potion/remove <index>`、`SS3D rest heal/upgrade`、`SS3D event 0/1`。当前版本不依赖 3D 模型和 UMG，可通过 UE 日志完整验证游戏循环。

### 当前明确缺口

- 3D 战斗场景、玩家模型、敌人模型、卡牌立绘、动画、粒子和音频资源：仓库中不存在，当前不可用。
- `SS3DEditor` 和 Game Target 均已通过 UE5.8 编译。
- 已完成 Windows Cook/Stage/Pak/Archive，归档目录为 `StagedBuild/Windows`。
- 已运行归档程序的 `SS3D demo` 回归，日志输出 `DEMO PASS：三层尖塔完整通关`。

## 📁 项目结构

```
Source/
├── SS3D.Target.cs       # UE5 Game Target
├── SS3DEditor.Target.cs # UE5 Editor Target
└── SS3D/
    ├── SS3D.Build.cs       # UE5 模块配置
    ├── CombatTypes.h       # 卡牌、战斗、资源和敌人数据类型
    ├── CombatManager.*     # 战斗运行时逻辑
    ├── MapTypes.h          # 地图节点、层和路线数据
    ├── MapManager.*        # 地图生成与路径选择
    ├── CardLibrary.*       # 卡牌库和奥黛塔初始卡组
    ├── RelicLibrary.*      # 藏品库
    ├── PotionLibrary.*     # 药水库
    ├── EnemyLibrary.*      # 敌人和 Boss 库
    ├── SS3DGameMode.*      # 纯代码游戏流程
    └── SS3DPlayerController.* # 控制台命令入口

Content/
├── Maps/               # UE5 场景
├── Blueprints/         # 可视化表现和资源组装
├── UI/                 # UMG Widget
├── Characters/         # 角色和敌人资源
└── FX/                 # 动画、材质和特效
```

## 🚀 开发计划

- [x] P1: UE5 C++ 工程基线 + 地图数据结构
- [x] P2: 三层尖塔地图生成 + 进入游戏后的路径选择
- [x] P3: 战斗垂直切片 + 节点进入和结算
- [x] P4: 奖励、商店、事件、休息、藏品和药水
- [x] P5: Editor/Cook/打包验收 + 纯代码地图/战斗流程
- [ ] P5: 正式 UMG 地图/战斗界面
- [ ] P6: 3D 战斗表现 + UMG 打磨

---

*Built with ❤️ and lots of AI assistance*

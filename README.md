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
- 当前已有战斗后端和运行时 HUD 原型，下一步优先实现进入游戏后的尖塔地图生成与路径选择。
- 第一角色奥黛塔的初始卡组为：打击 x5、防御 x4、痛击 x1，共 10 张；每回合 3 点能量。
- 主要边界：`GameInstance` / 运行状态、`MapManager`（地图逻辑）、`CombatManager`（战斗逻辑）、UMG Widget（表现层）。

### 当前明确缺口

- 3D 战斗场景、玩家模型、敌人模型、卡牌立绘、动画、粒子和音频资源：仓库中不存在，当前不可用。
- UE5 Editor / 编译环境未在当前命令行环境中验证，需在本机 Unreal Editor 中编译和运行。

## 📁 项目结构

```
Source/SS3D/
├── SS3D.Build.cs       # UE5 模块配置
├── CombatTypes.h       # 卡牌和战斗数据类型
├── CombatManager.*     # 战斗运行时逻辑
├── MapTypes.*          # 地图节点、层和路线数据
├── MapManager.*        # 地图生成与路径选择
└── UI/                 # 后续 UMG 表现层

Content/
├── Maps/               # UE5 场景
├── Blueprints/         # 可视化表现和资源组装
├── UI/                 # UMG Widget
├── Characters/         # 角色和敌人资源
└── FX/                 # 动画、材质和特效
```

## 🚀 开发计划

- [ ] P1: UE5 C++ 工程基线 + 地图数据结构
- [ ] P2: 三层尖塔地图生成 + 进入游戏后的路径选择
- [ ] P3: 战斗垂直切片 + 节点进入和结算
- [ ] P4: 奖励、商店、事件、休息、藏品和药水
- [ ] P5: 3D 战斗表现 + UMG 打磨

---

*Built with ❤️ and lots of AI assistance*

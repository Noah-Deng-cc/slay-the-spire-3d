# SS3D 技术框架

版本：v0.1
平台优先级：Windows  64-bit
引擎：Unreal Engine 5.8
语言：C++
表现：UMG/Slate + 3D Actor
输入：Slate 鼠标交互 + PlayerController 键盘快捷键；角色移动阶段再接入 Enhanced Input

## 1. 目标

SS3D 采用 UE 友好的分层架构。规则可以在没有正式模型、动画和特效的情况下运行和验收；表现资源只能替换表现层，不能改变规则结果。

第一阶段只做初始地图和一条可操作的普通跑动链路。地图、战斗、奖励、商店和其他节点虽然已经有规则基线，但未进入对应阶段的表现开发前，不继续扩展内容数量。

## 2. 总体树

```text
SS3D/
├── Config/                         # 输入、窗口、默认地图、平台配置
├── Content/
│   ├── Maps/                       # UE 场景与关卡实例
│   ├── Data/                       # PrimaryDataAsset、数据表、曲线
│   ├── Characters/                 # 角色/敌人 SkeletalMesh、Skeleton、Anim*
│   ├── UI/                         # Widget Blueprint、字体、UI 材质
│   ├── FX/                         # Niagara、材质、受击/卡牌反馈
│   └── Audio/                      # 音效与音乐
├── Source/SS3D/
│   ├── Core/                       # 通用类型、阶段、事件、运行上下文
│   ├── Domain/                     # 纯玩法规则：地图、战斗、卡牌、敌人
│   ├── Application/                # 用例编排：开局、节点进入、战斗结算
│   ├── Platform/                   # UE 输入、窗口、PlayerController、调试命令
│   ├── Presentation/               # HUD、Widget 适配、3D 表现 Actor
│   └── Debug/                      # 回归、审计、白膜测试工具
├── Docs/
│   ├── Architecture.md             # 本文：架构和依赖规则
│   ├── ModuleInventory.md           # 模块能力与人工输入边界
│   ├── ProjectPlan.md               # 阶段目标和完成标准
│   ├── Verification.md              # 编译、运行、打包验收
│   └── Attribution.md               # 外部资源借物和限制
└── Local/                           # 本地受限资源，禁止进入 Git
```

### 当前代码到目标目录的映射

当前第一版为了保持已通过的 UE 编译和回归，仍有一部分文件在 `Source/SS3D/` 根目录。它们按下面的目标归属理解，后续只在修改相关模块时逐步迁移：

```text
现有文件                         目标归属
----------------------------------------------------------------
Core/SS3DTypes.*                 Core
Core/SS3DGameState.*             Core / UE 状态适配
CombatTypes.*                    Domain/Combat
CombatManager.*                 Domain/Combat
CardLibrary.*                    Domain/ContentRules
EnemyLibrary.*                  Domain/ContentRules
MapTypes.*                       Domain/Map
MapManager.*                     Domain/Map
SS3DGameMode.*                   Application/GameFlow（当前临时兼任入口）
SS3DPlayerController.*           Platform/Input
BattleHUD.*                      Presentation/UI（当前临时保留旧名）
Presentation/SS3DWhiteboxStage.* Presentation/World
```

这不是立即搬家的任务。搬迁必须以“编译通过、自动审计通过、行为不变”为门槛；没有这个门槛就不改目录。

## 3. 依赖方向

```text
Core
  ↑
Domain  ───────→ Core
  ↑
Application ───→ Domain + Core
  ↑          ↘
Platform      Presentation
                 ↓
              Content/UE Assets
```

约束：

1. `Core` 不依赖 `Presentation`、具体角色模型、Widget 或音频。
2. `Domain` 只负责规则和可序列化状态，不直接 Spawn Actor、不读取 Widget、不播放动画。
3. `Application` 负责用例顺序和阶段切换，不实现卡牌伤害、地图连接等具体规则。
4. `Platform` 只把 UE 生命周期、输入和控制台命令转成 Application 调用。
5. `Presentation` 只能读取快照或订阅事件，再调用表现资源；不能绕过 Application 修改规则状态。
6. `Content` 是数据和表现输入。资源缺失时显示缺失状态或白膜，不伪造动画、贴图或授权。
7. `Debug` 可以调用公开的 Application/Domain 接口，但回归代码不能成为正式玩法入口。

## 4. 运行时职责

### Core

保存跨模块稳定的概念：

- `ESS3DGamePhase`：Boot、CharacterSelect、Map、Combat、Reward、Shop、Event、Rest、Victory、Defeat。
- 检查点、运行种子、节点位置和最小事件类型。
- 不保存具体 UI 状态，不保存 SkeletalMesh 指针。

### Domain

保存可验证的玩法状态和规则：

- `MapManager`：生成地图、判断可选节点、完成节点。
- `CombatManager`：抽牌、出牌、回合、伤害、护盾、状态效果、敌人意图。
- `CardLibrary`、`EnemyLibrary`、`RelicLibrary`、`PotionLibrary`：第一阶段的规则数据源。
- 所有公开结果通过快照、返回值或事件发出。

目标状态：后续可以把 Domain 的回归测试搬到独立测试入口，而不需要启动完整 UI。

### Application

当前由 `SS3DGameMode` 临时承担：

- 启动、角色选择、开局、节点进入和节点结算。
- 把 Domain 的结果映射为阶段变化。
- 向 UI 发布一个统一的“当前阶段 + 当前快照”。

后续若 `GameMode` 继续变大，拆出 `USS3DRunSession` 或等价的 `UObject` 用例服务；拆分条件是职责可以独立测试，而不是为了增加目录数量。

### Platform

- `PlayerController`：只处理输入绑定和调试命令转发。
- 当前卡牌 UI：鼠标由 Slate/UMG 处理，Enter、Space、Esc、M、H 由 `PlayerController` 统一绑定。
- 角色移动阶段：使用 Enhanced Input 统一 Move、Look、Jump/Interact 动作，不在 Widget 中判断原始按键。
- Windows 窗口：支持窗口化、全屏、分辨率调整，输入与分辨率配置不写死在玩法代码中。

### Presentation

- 地图 UI：显示节点、连接、当前节点和可选节点。
- 战斗 UI：显示手牌、能量、生命、敌人意图和战斗结果。
- 3D 表现：角色、敌人、相机、动画、特效和音频触发。
- 第一版允许白膜，但白膜只表示空间和交互位置；它不能被当作正式模型或缺失动画的替代品。

## 5. 一个功能的标准实现顺序

任何新功能都按这一条链路完成，不跨层直接写：

```text
需求与规则表
  → Domain 状态/规则
  → Domain 回归用例
  → Application 阶段和事件
  → Presentation 输入与状态显示
  → Content 资源绑定
  → Windows PIE / Cook / 自动审计
```

以“普通跑动”为例：

1. 规则需求：角色能在初始地图场景中用 WASD 移动，鼠标控制视角，不能穿过地面或离开测试区域。
2. Domain：不新增卡牌或战斗规则；移动属于角色控制，不应塞进 `CombatManager`。
3. Application：只负责当前阶段允许进入地图场景，并提供角色实体的创建参数。
4. Platform：配置 Enhanced Input 的 Move、Look、Jump/Interact 动作。
5. Presentation：创建 `Character`、`Camera`、碰撞体和动画实例；没有真实 Walk 动画时必须显示“动画缺失”，不能假装播放。
6. 验收：鼠标键盘、可调分辨率、窗口切换、碰撞、重新进入 PIE 后状态清理均通过。

## 6. 阶段树和门槛

```text
P0 工程基线
├── Windows 编译、PIE、Cook
├── 默认地图与 GameMode
├── 日志/审计命令
└── Git 资源边界
    ↓
P1 初始地图壳
├── 初始地图场景
├── 地图节点最小 UI
├── 鼠标键盘输入
└── 窗口/分辨率可调
    ↓
P2 角色普通跑动
├── 角色 Actor 与 Capsule 碰撞
├── WASD 移动、鼠标视角
├── 相机跟随
└── Idle/Walk 资源状态检查
    ↓
P3 一场战斗表现
├── 进入战斗区域
├── 角色/敌人站位
├── 卡牌 UI 与规则快照绑定
└── 攻击/受击/死亡资源逐项接入
    ↓
P4 初始地图完整循环
├── 节点选择
├── 普通战斗
├── 奖励与返回地图
└── 失败/重开
    ↓
P5 内容扩展
├── 精英、商店、事件、休息
├── 多敌人和 Boss
└── 三层完整流程
    ↓
P6 资产、平衡、Windows 发布包
```

每个阶段必须同时满足：

- 代码边界清楚；
- 缺失资源有明确状态；
- 鼠标和键盘至少有一条可操作路径；
- 新增回归用例通过；
- Windows Development 构建通过；
- 不把本地受限资源提交或打包。

## 7. 当前完成状态与下一步

以下无需人工资源的基线已经完成并通过自动验收：

- 初始地图、节点选择和地图 UI；
- 地图、战斗、奖励、商店、事件、休息和结算页面；
- 鼠标按钮和键盘快捷键；
- Windows 窗口分辨率配置与运行时 `window <width> <height>` 命令；
- 白膜场景、玩家/敌人占位体、完整三层流程；
- Boot 到 Victory 以及 Defeat/重开路径的检查点历史。

下一阶段才是需要人工资源配合的角色表现：正式 SkeletalMesh、Skeleton、Idle/Walk/Attack/Hit/Death、材质和动画蓝图。没有这些资源时不继续声称角色表现完成。这样每次只处理一棵树上的一个节点，不再为每个资源临时发明一套做法。

## 8. 资源和授权硬规则

- 奥黛塔模型只用于本地技术测试。
- 禁止商业使用，禁止二次配布，允许修改模型，但必须保留完整借物表。
- `Local/` 不进入 GitHub，不进入公开 Cook/Pak。
- 没有 FBX、Skeleton、Idle、Walk、Attack、Hit、Death 等真实资源时，只报告缺失，不生成“模拟动画”冒充完成。

# 记忆尖塔：当前开发要求

## 项目基线

- 引擎：Unreal Engine 5.8
- 语言：C++
- UI：UMG / Slate
- 目标：PC 单人卡牌 Roguelike
- 当前角色：奥黛塔（战士）
- 当前表现：后端和日志驱动；正式版本采用 2D UI + 3D 战斗表现

## 已确定规则

- 世界观：记忆尖塔是一个神秘空间，登上塔顶才能离开。
- 一局包含三层，每层都有普通节点、精英节点、商店、事件、休息、奖励和层末 Boss。
- 初始卡组为打击 x5、防御 x4、痛击 x1，共 10 张。
- 每回合 3 点能量，默认抽 5 张牌。
- 卡牌、藏品、金币和药水是局内主要资源。
- 第一版优先复用成熟卡牌爬塔规则，之后再加入独特的记忆机制。

## 代码边界

`Source/SS3D` 中的后端模块职责如下：

- `MapManager`：固定种子地图生成、节点连接、可选路线和三层推进。
- `CombatManager`：抽牌、出牌、能量、伤害、护盾、状态和敌人意图。
- `CardLibrary`：初始卡组和奖励卡牌。
- `EnemyLibrary`：普通敌人、精英敌人和三层 Boss。
- `RelicLibrary` / `PotionLibrary`：藏品和药水库。
- `SS3DGameMode`：角色选择、开局、节点结算、商店、事件、休息和通关。
- `SS3DPlayerController`：通过 `SS3D <command>` 提供调试和回归入口。

## 当前验收

在 PIE 控制台输入：

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

输入 `SS3D demo` 会使用固定种子自动选择路线、处理战斗和奖励，并尝试完成三层。成功标志为：

```text
DEMO PASS：三层尖塔完整通关。
```

## 当前待办

1. 安装 Visual Studio 的 `.NET Framework SDK 4.6+`，解除 UE5.8 `SwarmInterface` 的 Editor Target 编译阻塞。
2. 完成 Editor Target 编译、Cook、Stage、Pak 和打包运行验收。
3. 实现正式 UMG 地图和战斗界面。
4. 接入奥黛塔、敌人、卡牌和战斗特效资源。
5. 在资源接入后进行平衡测试和正式试玩。

当前不要求用户额外编写后端系统代码。需要用户提供或制作的具体内容是模型、UI Widget、特效和音频资源；资源到位后再进行表现层绑定。

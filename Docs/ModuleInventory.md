# SS3D 模块能力清单

更新时间：2026-08-21

## 可直接使用的现有模块

| 模块 | 代码位置 | 状态 | 验收入口 |
|---|---|---|---|
| 地图生成与路线 | `Source/SS3D/MapManager.*` | 可用，固定种子可复现 | `SS3D nodes` |
| 战斗回合 | `Source/SS3D/CombatManager.*` | 可用，含抽牌、能量、伤害、护盾和意图 | `SS3D effects` |
| 卡牌库 | `Source/SS3D/CardLibrary.*` | 可用，含初始卡组和奖励卡 | `SS3D demo` |
| 敌人库 | `Source/SS3D/EnemyLibrary.*` | 可用，含普通、精英和三层 Boss | `SS3D demo` |
| 藏品与药水 | `Source/SS3D/RelicLibrary.*`、`PotionLibrary.*` | 可用 | `SS3D effects` |
| 节点流程 | `Source/SS3D/SS3DGameMode.*` | 可用，覆盖地图、战斗、奖励、商店、事件、休息和通关 | `SS3D audit` |
| 调试入口 | `Source/SS3D/SS3DPlayerController.*` | 可用，支持控制台命令 | PIE / `-ExecCmds` |
| 阶段检查点 | `Source/SS3D/Core/SS3DGameState.*` | 已接入，日志输出 `CHECKPOINT` | `SS3D audit` |
| 白膜表现 | `Source/SS3D/Presentation/SS3DWhiteboxStage.*` | 已接入，运行时生成场景、玩家和敌人占位体 | `WHITEBOX PASS` |
| 阶段 UI | `Source/SS3D/BattleHUD.*` | 已覆盖开局、地图、战斗、奖励、商店、事件、休息、胜利和失败 | `UI PASS` |
| 鼠标/键盘输入 | `Source/SS3D/SS3DPlayerController.*`、Slate HUD | 已支持鼠标按钮、Esc、Enter、Space、M、H | `INPUT PASS` |
| Windows 窗口设置 | `Config/DefaultGameUserSettings.ini`、`SS3DGameMode.*` | 已支持默认窗口和运行时分辨率命令 | `WINDOW PASS` |

## 已由代码完成的模块

| 模块 | 当前处理方式 |
|---|---|
| 规则回归和完整流程 | 已由 C++ 完成，包含地图、效果、失败、重开、三层通关和阶段覆盖 |
| 白膜交互演示 | 已由运行时 Slate/UMG、占位场景和控制台/快捷键完成 |
| Windows 自动化验收 | 已由 `Build.bat`、`UnrealEditor-Cmd.exe` 和固定种子日志完成 |
| 分辨率与输入配置 | 已由 GameUserSettings、Slate 鼠标和 PlayerController 键盘入口完成 |
| `UPrimaryDataAsset` 迁移 | 可选后续工程整理，不是当前完整游戏的阻断项 |

## 必须人工提供或确认的内容

| 内容 | 原因 | 当前状态 |
|---|---|---|
| 奥黛塔正式模型 | 外部模型授权和来源不能由代码推断 | 仅本地测试，未提交 |
| 骨骼、Idle、Walk、Attack、Hit、Death | 缺失资源不能模拟，需真实资产和骨骼兼容性确认 | 未纳入主工程运行链 |
| 敌人正式模型和动画 | 项目中没有可公开使用的对应资产 | 待提供 |
| 卡牌立绘、UI 美术、特效和音频 | 属于美术制作或授权资源 | 待提供 |
| 借物表和最终授权确认 | 属于人工法律/授权信息 | 待填写 |
| 正式视觉风格和数值平衡 | 需要人工审美和试玩判断 | 待试玩 |

## 资源处理规则

`Local/` 只存本地测试资源，已加入 `.gitignore`。受限模型不会进入 GitHub、Cook、Pak 或公开打包版本。缺少资源时只显示缺失状态或使用白膜占位，不伪造动画或授权。

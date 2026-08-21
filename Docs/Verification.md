# SS3D 验收工作流

## 编译

```powershell
$ProjectFile = (Resolve-Path ".\SS3D.uproject").Path
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" SS3DEditor Win64 Development $ProjectFile -WaitMutex -FromMsBuild
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" SS3D Win64 Development $ProjectFile -WaitMutex -FromMsBuild
```

## 运行时自动验收

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $ProjectFile -game -log -nosplash -unattended -ExecCmds="SS3D audit,quit" -stdout -FullStdOutLogOutput
```

## Windows 归档验收

```powershell
$ArchiveExe = (Resolve-Path ".\StagedBuild\Windows-Audit\SS3D.exe").Path
& $ArchiveExe -game -log -nosplash -unattended '-ExecCmds="SS3D audit,quit"'
if ($LASTEXITCODE -ne 0) { throw "Archive audit failed: $LASTEXITCODE" }
```

本次归档验收退出码为 `0`，精简日志见 `Docs/Verification/archive-audit.log`。

## 必须出现的结果

```text
WHITEBOX PASS
UI PASS
INPUT PASS
WINDOW PASS
NODES PASS
EFFECTS PASS
DEFEAT PASS
PHASE PASS
PHASE COVERAGE PASS
DEMO PASS：三层尖塔完整通关。
FLOW PASS：固定种子完整运行到 Victory。
AUDIT PASS：规则、地图、战斗、奖励和三层流程全部通过。
```

## 当前验证范围

- `Boot -> CharacterSelect -> Map -> Combat -> Reward -> Victory` 已由运行时检查点历史记录。
- `Defeat -> new -> Map` 失败/重开路径由 `DEFEAT PASS` 和阶段覆盖回归记录。
- `Shop`、`Event`、`Rest` 有明确阶段映射和可执行命令，节点合法性由 `SS3D nodes` 检查。
- 白膜场景和玩家/敌人占位体由 C++ 在运行时生成，不依赖人工摆放资产。
- 阶段 UI、鼠标/键盘入口和窗口设置均纳入 `SS3D audit`；真实模型、动画、特效和音频仍不属于自动验收范围，缺失时必须报告。

## 日志位置

UE 运行日志位于 `Saved/Logs/SS3D.log`。提交前从该文件提取本次 `AUDIT`、`CHECKPOINT` 和 `PASS` 行，保存为验证附件；完整引擎日志不提交到仓库。

本次已额外直接启动归档产物 `StagedBuild/Windows-Audit/SS3D.exe` 验证，归档日志中的 `AUDIT PASS` 和 `LogExit: Exiting` 均已保存到 `Docs/Verification/last-audit.log`。

## 运行操作

- 鼠标：点击地图节点、卡牌、奖励、商店和节点操作按钮。
- `Esc`：显示/隐藏鼠标交互模式。
- `Enter`：开始游戏、结束战斗回合或确认当前默认操作。
- `Space`：结束战斗回合。
- `M`：输出当前地图和路线。
- `H`：输出当前手牌。
- 控制台：`SS3D window 1600 900` 修改窗口分辨率。

## 日志提取

```powershell
Select-String -Path "Saved\Logs\SS3D.log" -Pattern "PASS|CHECKPOINT|LogExit" |
    Set-Content "Docs\Verification\last-audit.log"
```

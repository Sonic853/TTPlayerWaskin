# WAL 扩展、HeadAMP 冷启动及预览修复

日期：2026-09-24。此文记录本轮已经完成并验证的实现；先前的全控件分析是修改前的静态快照。

后续 57 个样本检查与新增修复见 [新增皮肤检查报告](WAL_NEW_SKINS_AUDIT_2026_09_24.md)，下文 17 个样本数据保留为本次扩展前的记录。

## 1. HeadAMP 重启失败的实际原因

`LoadStartupSkin` 在主窗口 `WM_CREATE` 加载播放列表之前创建皮肤。HeadAMP 的 `System.onScriptLoaded` 会调用 `System.getStatus`，进而触发主程序的 `QuerySkinPluginState`。旧实现通过 `PlaybackTrackForUi`、`VisiblePlaylistTrackCount` 等函数访问尚为空的 `PlaylistStore::Active()`，产生 `0xC0000005`。

这解释了“运行中切换正常，保存 HeadAMP 后再次打开崩溃”：原来的测试先创建主窗口、再切换 WAL，未覆盖这个初始化顺序。

修复位置：`rebuild/src/ui/player_window_skin_plugin.cpp`。

- 播放列表尚未初始化时返回曲目数 0、当前行 -1 和正常的停止状态。
- 曲目信息、选中行查询同样保护空列表。
- 保留原有初始化顺序；启动阶段不提前创建窗口、不执行播放命令。
- 普通版和 XP/Win7 构建分别通过“保存 HeadAMP → 新建播放器对象 → 窗口创建前加载皮肤 → 创建窗口 → 保存退出”，连续运行两次。

## 2. 为什么其他 WAL 不在列表里

旧的 `probe` 使用完整皮肤实例，遇到任何未支持控件、MAKI 导入或布局就返回失败，主程序将该文件完全过滤。它混淆了“这是 WAL 文件”和“目前能够完整创建其活动布局”。

现在分为三步：

1. **识别**：检查归档、`skin.xml` 根标签、元数据和可用 VM；不执行皮肤脚本。
2. **兼容性检查**：解析活动布局、资源、MAKI 导入和初始化，返回包含 XML 文件、控件类型/ID 或脚本方法的错误。
3. **应用**：只有实例创建成功才替换当前皮肤；选项页对不支持的皮肤显示原因并禁用“应用”，右键菜单直接选择失败也会说明原因。冷启动失败时沿用主程序的原生皮肤回退。

VM 不存在或不兼容时仍只提供 `.wsz`，不列出 WAL。

## 3. 本轮基础兼容实现

没有接入 Wasabi 服务。XML/资源/绘制/交互留在 `ttp_waskin.dll`，字节码执行留在 `ttp_maki.dll`。

### XML 和资源

- 支持皮肤归档内唯一子目录的 `skin.xml`；拒绝多个根文件造成的歧义。
- `include` 按出现位置展开；允许多次引用同一文件，拒绝递归引用。
- 资源相对当前 XML 目录解析，然后尝试包根目录；规范化路径且禁止逃出包根目录。
- Winamp 的多冒号 XUI 标签按原始名字保留；不把它们当 XML 命名空间处理。属性值中的 URL 等不受修改。
- Group 定义、同 ID 祖先、显式继承、XUI 模板和资源别名基础解析。
- 静态布局使用在其之前可见的同名 Group 定义。Objection 的两个布局各有同名按钮，分别切换到对方；不会再被最后一个定义全部覆盖。

### 控件、绘制和布局

- 基础 `relatx/y/w/h` 的 0/1/2 模式、`fitparent`、尺寸上下限、父级 alpha、ghost/disabled 命中处理。
- Button/ToggleButton 的默认、按下、悬停、激活和禁用图像；音量、平衡、进度、EQ 滑块。
- BitmapFont 使用 Winamp 字母/数字图集排布；TrueTypeFont 在内存中加载，支持基础对齐、颜色、阴影和换行。
- 主容器多个布局及 `SWITCH`，保存与恢复当前布局；旧版 HeadAMP 布局状态仍可读取。
- 播放、暂停、停止、切歌、打开文件、最小化、关闭、原生菜单等命令交给千千静听。
- Shuffle、Repeat、Crossfade 和 EQ 状态通过主程序反馈；Crossfade 对应千千静听已有的曲目切换淡入淡出设置。

### MAKI

- 方法按 GUID 对应的类与继承链校验，避免把所有 GUI 方法放进一个通用白名单。
- 增加乘、除、取模、NEW/DELETE；对象只能由宿主工厂创建并按所有权释放。
- 目前可由脚本创建 Timer，支持延时、启动、停止和 onTimer；不把其他类假装成 Timer。
- ABI 尾部增加可选工厂、释放与诊断回调，保留旧前缀大小协商。
- 回收脚本对象时移除活动节点；保留墓碑直到脚本销毁，避免定时器回调快照使用悬空指针。

这仍是 WAL/MAKI 的部分实现，不代表 66 类控件已经完成。复杂 region/缩放、通用自动尺寸、Ticker、动画图层、通用配置绑定、动态标准窗口、完整系统 XML/XUI 模板和全部 MAKI 指令/方法仍有差异。

## 4. 17 个本地 WAL 的验证结果

“通过”表示创建、预览及本轮列出的基本操作通过，不代表与 Winamp 每个像素/每项交互完全一致。

| 皮肤 | 识别 | 当前结果 / 首个阻塞原因 |
|---|---|---|
| HeadAMP | 是 | 通过；播放/暂停/停止、抽屉、EQ、拖放、重启恢复 |
| DashAmp | 是 | 通过；播放/暂停、Timer、音量 |
| MerokoAndMitsuki | 是 | 通过；播放/暂停、音量 |
| Objection | 是 | 通过；播放/暂停、音量、两个布局互切和重建恢复 |
| xxxHolic | 是 | 通过；播放/暂停、音量 |
| CastleCrashers | 是 | AnimatedLayer |
| Diablo II Resurrected | 是 | EQ_AUTO action |
| Diablo IV Skills / Diablo IV Skills V2 | 是 | AnimatedLayer |
| Meroko-Proto | 是 | @DEFAULTSKINPATH@ 外部系统资源 |
| Players | 是 | AnimatedLayer |
| PokemonDS | 是 | AnimatedLayer |
| SingItKitty | 是 | EQ_AUTO action |
| TwilightDuo | 是 | EQ_AUTO action |
| Ujola Cat | 是 | 未支持的按钮 action |
| Wiimote | 是 | EQ_AUTO action |
| Wriothesley - Beyond the Rules | 是 | SnapPoint |

阻塞原因按首次遇到的问题记录。补齐这一项后仍可能出现后续缺失项，不能据此承诺只差一个控件。

建议下一阶段先补 `AnimatedLayer`、EQ_AUTO 的主程序能力映射、`SnapPoint`，再处理系统资源模板和更完整的 MAKI 接口；继续以真实皮肤操作验证，不能只以“能生成预览”判定兼容。

## 5. 预览灰底和测试中的“损坏的映像”弹窗

WAL 预览原来调用 `GetHBITMAP(Color(255,32,32,32))`，把透明区域固定合成到深灰色。现在先合成到 `COLOR_WINDOW`，与选项页和原生皮肤预览一致，半透明边缘也使用相同底色。不会修改实际皮肤窗口的 alpha/区域。

测试目录的 `wal-expanded/invalid/ttp_maki.dll` 是故意写入无效内容的负面测试样本。此前 Windows 的加载错误弹窗未被抑制，阻塞了测试。这不是发布 DLL 损坏。

可选 VM 加载现在使用局部错误模式：支持时动态调用 `SetThreadErrorMode`，XP 使用短暂的 `SetErrorMode`，加载后恢复原状态。缺失/损坏 VM 安静回退；不会添加 Win7 专有函数的静态导入。

## 6. 验证和交付

全部测试为 `rebuild/tests/waskin` 中的本地 C++ 测试，没有添加到 Actions 或发布 ZIP。

- `wal_document_tests`：嵌套根目录、重复 include、多冒号 XUI、资源目录、同名定义顺序、递归/越界/DTD 拒绝。
- `wal_catalog_tests`：17/17 识别，5 个创建与预览通过，其他皮肤返回非空诊断。
- `modern_dll_tests`：缺失/损坏 VM 回退、宿主错误模式恢复、HeadAMP 抽屉/命令/拖放/布局、预览透明区域像素、其他四个皮肤的实际按钮操作，以及 Objection 切换/恢复。
- `waskin_host_tests --modern-only`：两种构建真实主程序界面的选项诊断、支持/不支持条目并列显示、拖入文件位置、保存后连续两次冷启动。
- `maki_vm_tests`：新增指令、对象创建销毁、宿主对象不可删除、预算和损坏字节码等边界。
- 原有 WSZ atlas、按键、可选 ABI 尾部、30 次生命周期、内置默认皮肤和缺失窗口回退回归通过。
- XP/Win7 静态导入检查通过；这些运行测试在当前 Windows 上完成，未代替 XP/Win7 实机测试。

发布保持原来的格式：两个版本号 ZIP 只包含对应 `TTPlayerRebuild.exe` 和校验文件，共用 DLL 放入 `rebuild/build/Release/AddIn`。VM 的损坏样本和其他测试文件不分发。

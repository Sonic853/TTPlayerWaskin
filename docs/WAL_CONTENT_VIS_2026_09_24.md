# WAL 内嵌内容与 Winamp 风格 Vis

日期：2026-09-24。以 HeadAMP 包内 `xml/player.xml`、`scripts/playerMain.m` 和 Winamp `Src/Wasabi/api/skin/widgets/sa.cpp` 为依据。

## 两个区域的职责

| 控件 | 新行为 |
|---|---|
| `component param="guid:avs"`（HeadAMP 为 `InlineAVS`） | 与 WSZ 视频内容使用同一套千千功能：默认歌词、视觉效果、歌词与视觉同屏；原生菜单、歌词拖动、编辑和全屏 |
| `<vis>` | 跟随主窗口的视觉类型。频谱和波形由插件按 WAL 的 Winamp 样式绘制；梦幻、专辑封面等继续使用主窗口已有绘制 |

HeadAMP 顶部 `Toggle AVS` 仍执行包内 MAKI，切换两个区域的可见性。`InlineAVS` 的显示内容通过右键菜单选择，单击和双击不改变内容模式。内容模式及其视觉类型随皮肤布局保存；它们与主窗口 `vis` 的视觉选择相互独立。

## 原版 Vis 样式

Winamp `SAWnd` 在固定栅格上绘制，向布局暴露左侧 **72×16** 像素，再缩放到 XML 控件尺寸。HeadAMP 的 `vis` 为 191×138，所有频谱、峰值和示波器颜色均设为白色。

本实现读取 `colorallbands`、`colorband1…16`、`colorbandpeak`、`colorallosc`、`colorosc1…5`，以及 `bandwidth`、`coloring`、`peaks`、`falloff`、`peakfalloff`、`oscstyle`、`fliph`、`flipv`、`fps`、`channel`。颜色支持 RGB 三元组、十六进制及已解析的皮肤颜色名称。

- 宽柱使用三像素柱身和一像素间隔；细柱连续。
- 频谱保留普通渐变、fire 和 line 配色，峰值及原版五档衰减参数。
- 默认衰减间隔为 25 ms（`QuickPaintWnd::speed`）；皮肤可通过 `fps` 调整。
- 示波器保留 dots、solid、lines 三种 XML 样式；名称对应原版实现（solid 连接相邻样本，lines 从中线填充）。
- 模式、分析代次或样式变化时清除旧绘制状态；停止播放后清除旧图形。
- 音频 FFT 和双声道 PCM 仍来自千千主程序。频谱幅值沿用千千的分析结果，不声称与 Winamp 音频核心逐帧相同。

`vis` 单击及右键沿用千千主窗口的视觉切换和菜单。按照本次需求，模式由主程序设置决定；没有引入 Winamp 的全局可视化配置或 Wasabi 服务。

## 通用宿主接口

WAL 解析、样式和内容区域仍在 `ttp_waskin.dll`。宿主只扩展通用内容能力：允许主窗口内的内容矩形接入原生歌词输入、菜单及编辑器；编辑器可在嵌入区域与独立歌词窗口之间切换父窗口。切换到 `vis` 时隐藏编辑器，返回 `InlineAVS` 时恢复；绘制排除可见编辑控件，透明工具栏擦背景时正确重画。

分析快照结构追加有长度标记的双声道 PCM 数据。旧版结构前缀保持不变，宿主按调用方长度复制；旧 DLL 不会被越界写入。使用本次全部功能应同步更新 EXE 和 DLL，两版 EXE 共用同一个 x86 `ttp_waskin.dll`，`ttp_maki.dll` 无需更换。

## 本地验证

测试仅在 `rebuild/tests/waskin`，不上传，不加入 Actions 或发布包。结果保存在 `rebuild/out/test-artifacts/wal-content`。

- 固定数据绘制：柱宽/间隔、白色配色、翻转、示波器样式、模式切换清屏。
- 真实 HeadAMP DLL：XML 白色配色和频谱/波形路径、内容选择及保存恢复。
- 真实主程序：嵌入区域几何、歌词拖动/取消、三类原生菜单、编辑器布局及切换可见性、旧快照 ABI 边界。
- 编辑器在内嵌/独立歌词窗口间移动；从内嵌内容进入同屏全屏，退出后恢复主窗口视觉类型。
- 真实音频工作线程：隐藏的原生视觉控件仍提供 FFT 和双声道 PCM，连续切换视觉不会短暂显示旧原生子窗口。
- 普通版及 XP／Win7 版的上述主程序集成通过；WSZ 视频窗口的原有歌词、拖动、编辑工具栏、字体/颜色、同屏、全屏及布局保存回归通过。

最终构建、旧系统运行及交付信息记录在同目录的测试日志和 `deployment.json`。

## 交付位置

- 普通版：`rebuild/build/Release/TTPlayerRebuild-2026.09.24.zip`
- 旧系统版：`rebuild/build/Release/TTPlayerRebuild-XP-Win7-2026.09.24.zip`
- 共享插件：`rebuild/build/Release/AddIn/ttp_waskin.dll`，并同步至 `rebuild/out/legacy-xp/Release/AddIn`。

ZIP 继续只含 `TTPlayerRebuild.exe` 和 `SHA256SUMS.txt`。共享 DLL 为 717312 字节，SHA-256：`a2375fc850ac3d17d124b9f73052ab28e53010999d8527a2de6d3bda802ee8db`。原文件已备份到 `rebuild/out/test-artifacts/before-wal-content-20260924-180228`。

VirtualBox XP 与 Win7 SP1 完整主程序集成日志为 `xp-host-full.txt`、`win7-host-full.txt`。XP 的 MCP 等待在 300 秒超时，但来宾测试继续完成，随后取回的日志包含全部通过记录；最终 DLL 另做针对性回归。

最终共享 DLL 的 XP、Win7 针对性回归均正常完成，日志为 `xp-content-final.txt`、`win7-content-final.txt`；两台虚拟机测试后均恢复为保存状态。

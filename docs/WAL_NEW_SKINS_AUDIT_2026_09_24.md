# 新增 WAL 皮肤检查与修复

日期：2026-09-24。样本来自 `rebuild/build/Release/Skin/waskin`，相对上一轮 17 个样本增加 40 个，总计 57 个。

> 后续更新：本文保留严格加载阶段的检查结果。最新实现已允许可选功能降级，见 [WAL 部分加载与 EQ_AUTO 禁用](WAL_PARTIAL_LOADING_2026_09_24.md)，以该文的可加载数量和限制为准。

## 结论

57 个均可识别并列入皮肤列表。基本创建、预览和交互验证通过的从 5 个增加至 8 个：HeadAMP、DashAmp、MerokoAndMitsuki、Objection、xxxHolic、CocaCola_Musica_Winamp5_Skin、PokemonDS、Players。后三个是本轮新恢复的皮肤；其中 PokemonDS、Players 是上一轮已有但不能应用的样本。

**这不是 57 个皮肤完整兼容的结论。** 剩余 49 个仍会在应用前给出具体错误，保留当前皮肤。即使通过基本检查，也不表示皮肤内所有独立窗口和 Winamp 专属功能都已恢复。本轮未修改用户的 WAL 文件，未引入 Wasabi 服务。

## 已修复的问题及原版依据

| 问题 | 修复 | Winamp 源码依据 |
|---|---|---|
| 空 XML include 导致整个皮肤被拒绝 | 接受零字节的包含文件作为空片段；保留根文件、循环引用和大小检查 | `Src/Wasabi/api/xml/XMLAutoInclude.cpp`；K-Tech、StarTrek 含实际空占位文件 |
| 布局只读 w/h，忽略 default_w/default_h | 尺寸依次取显式值、默认值、背景尺寸、最小值，并应用上下限 | 布局的 GuiObject/Group 尺寸规则；Quinto/WMP11 声明使用默认尺寸 |
| AnimatedLayer 整张图集被当作普通图片 | 支持横向、纵向帧条、elementframes；按当前帧绘制；实现帧范围、播放/暂停/恢复/停止、倒放和事件 | `Src/Wasabi/api/skin/widgets/animlayer.cpp` |
| 动画音量条空白部分不能点击 | Layer/AnimatedLayer 默认矩形命中，显式 rectrgn=0 才使用图像 alpha | `Layer::Layer()` 调用 `setRectRgn(1)` |
| 图层拖动音量/进度时脚本没有收到事件 | 传递按下、移动、松开事件，坐标为父 Group 客户区坐标；保留鼠标捕获和 complete 语义 | GuiObjectWnd 事件及皮肤 `volume.m`、`seek.m` |
| 脚本主动调用 onMouseMove 导致运行时失败 | 支持已声明事件方法的再次派发，仍受递归/指令预算限制 | CocaCola 的音量脚本在按下处理内调用移动事件 |
| 新事件路径重复 SetCapture 使点击目标丢失 | 每次按下只获取一次鼠标捕获；恢复 HeadAMP 播放及拖动 | Win32 捕获改变消息，实际 DLL 回归复现 |
| Map 和 setRegionFromMap 缺失 | 实现 Map 的加载、尺寸、通道/灰度/透明区域读取；按阈值生成独立绘制区域 | `api/script/objects/smap.cpp`、`Layer::setRegionFromMap`、`tataki/region/win/win32_region.cpp` |
| 隐藏 Slider 的宿主反馈没有通知动画 | 音量/进度变化触发 Slider 位置事件及 System.onVolumeChanged | PokemonDS、Players 用隐藏滑块驱动动画帧 |
| 脚本信息接口缺口 | 补充 getParam、getPosition、getPlayItemLength、getPlayItemString、seekTo、integerToTime、getToken、strSearch、strLeft、getSongInfoText 和 Button 激活事件 | `api/script/objects/systemobj.cpp`、`widgets/button.cpp` |
| 时间单位可能扩大 1000 倍 | integerToTime 直接使用毫秒，getPlayItemString 返回媒体路径/URL | `SystemObject::vcpu_integerToTime`、`vcpu_getPlayItemString` |
| 缺少字体导致整个皮肤加载失败 | 缺失或无效 TrueType 字体使用系统 Arial 回退并缓存失败结果 | `api/font/font.cpp` 的 `Font::requestSkinFont` 回退链；Players 引用了不存在的 `pokeFRLG.ttf.ttf` |
| TOGGLE 点击后才发现目标不支持 | 加载时检查目标；guid:ml 映射原生媒体库，eq 映射原生均衡器；CocaCola 内置浏览器按钮禁用 | 主程序使用现有媒体库/均衡器，无额外浏览器容器实现 |

`getSongInfoText` 使用千千静听提供的音频参数，文字格式不保证和 Winamp 的本地化字符串逐字相同。动画实时模式保存标志，仍由宿主 20 ms 定时器驱动，不承诺复刻 Winamp 的即时重绘调度。

## 功能边界与后续实现项

- 目前实例化主容器及其多个布局。其他独立容器、全局服务脚本、动态窗口创建、Config/PopupMenu、ComponentBucket 等尚未完整实现。缺少专用播放列表、EQ、歌词窗口时仍由默认 WSZ 窗口与千千静听原生功能承接。
- `EQ_AUTO` 是 Winamp 按曲目自动加载 EQ 的功能，不是 EQ 开关；没有查到千千静听现成功能可以等价替代，所以没有错误映射到 EQ_ENABLE。
- CocaCola 的内置网页容器无原生对应功能，浏览器按钮保持禁用。它的媒体库、播放列表和 EQ 按钮使用千千静听窗口；独立 EQ 容器的 XML 与 EQ_AUTO 尚未执行。
- `NStatesButton`、`SnapPoint`、`Rect`、`EqVis`、`Status`、`AlbumArt`，以及 Layer FX 网格变形、Region 对象、多窗口缩放/区域组合仍需逐项实现和验证。
- `@DEFAULTSKINPATH@` 指向 Winamp 自带系统模板，不能把缺失模板当作空节点。下一阶段需要随插件提供可独立使用的对应模板/控件实现，而不是安装 Wasabi 服务。
- 包内找不到引用的 XML/PNG/位图 ID，并不一定说明整个原包已损坏：有些是可选 include，有些依赖外部系统模板，另一些确实拼错或遗漏。当前检查器保守拒绝未解析的必经引用。需按引用用途补齐或采用原版允许的回退，不能一律忽略，否则会出现空窗口。

建议后续顺序：基础绘制控件与 Region → 通用容器及原生功能映射 → 多状态按钮/菜单/配置 → 系统模板 → Layer FX 和复杂皮肤。每一步使用实际 WAL 交互回归，不能仅靠预览成功宣布支持。

## 已执行验证

- 本地 C++ `modern_dll_tests`：8 个真实皮肤的播放/暂停/音量，CocaCola 紧凑布局/Map 音量/原生媒体库，PokemonDS 的进度拖动及六个布局往返，Players 媒体路径查询和进度按钮。
- C++ 像素夹具：横向/纵向/独立图片帧、倒放、停止后不重播、默认布局尺寸、空 include 和缺字体回退。
- 普通版、XP/Win7 版真实主程序集成：HeadAMP 保存后冷启动各两次，空播放列表阶段查询、媒体库命令、选项页错误提示、实际 OLE 插入、WSZ/原生切换。
- MAKI VM 边界、WSZ 控件和内置默认皮肤回归；可选 VM 缺失/损坏仍能静默返回，不弹系统“损坏的映像”窗口。
- XP/Win7 静态导入检查。运行测试在当前 Windows 上执行，本轮未在 XP/Win7 实机重新执行。

测试源码仅保留在 `rebuild/tests/waskin`，未加入 Actions 或发布包。扫描和运行日志在 `rebuild/out/test-artifacts/wal-new`。

## 57 个皮肤逐项结果

以下“受限”列出当前检查器遇到的第一个阻塞点；修复这一项后仍可能发现其他依赖。`新增` 表示相对上一轮 17 个样本新增。

| 文件 | 批次 | 当前结果 / 首个阻塞点 |
|---|---|---|
| Airtel__Song_Catcher.wal | 新增 | 受限：`xml/player-groups.xml: <togglebutton id=''> unsupported action: eq_auto` |
| AS-FxS.wal | 新增 | 受限：`skin.xml: <eqvis id='EQ.vis'> unsupported XML control` |
| AS_Fx21_EVO.wal | 新增 | 受限：`skin.xml: <button id='Credits'> unsupported toggle component: credits` |
| CastleCrashers.wal | 既有 | 受限：`xml/player-green.xml: <togglebutton id='Shuffle'> missing bitmap: eb.shuf.press` |
| Chevy_Number_29_Car.wal | 新增 | 受限：`xml/player-groups.xml: <button id=''> unsupported action: eq_auto` |
| Chevy_Number_8_Car.wal | 新增 | 受限：`xml/player-groups.xml: <button id=''> unsupported action: eq_auto` |
| CLIENT.wal | 新增 | 受限：`xml/player-normal.xml: <button id='on'> unsupported action: eq_auto` |
| CocaCola_Musica_Winamp5_Skin.wal | 新增 | 基本验证通过（范围见上文） |
| Coca_Cola__My_Coke_Music.wal | 新增 | 受限：`skin.xml: standardframe/standardframe.xml: standardframe/standardframe.xml: missing resource window_menus.xml` |
| DashAmp.wal | 既有 | 基本验证通过（范围见上文） |
| Diablo II Resurrected.wal | 既有 | 受限：`xml/player.xml: <button id='eqauto'> unsupported action: eq_auto` |
| Diablo IV Skills V2.wal | 既有 | 受限：`xml/component-seeker.xml: <layer id='paragon1'> missing bitmap: paragon.seeker.point` |
| Diablo IV Skills.wal | 既有 | 受限：`xml/component-controls-horse.xml: <rect id='visBg'> unsupported XML control` |
| Doctor_Who__The_Eleven_Doctors.wal | 新增 | 受限：`xml/player-normal.xml: <button id='on'> unsupported action: eq_auto` |
| Ebonite_2.0.wal | 新增 | 受限：`skin.xml: wasabi/wasabi.xml: wasabi/xml/pledit.xml: wasabi/xml/pledit-normal.xml: wasabi/xml/pledit-normal.xml: missing resource standardframe.xml` |
| EPS_High-End_System_v1_test.wal | 新增 | 受限：`skin.xml: missing resource PNG/shade/base.png` |
| Firefox.wal | 新增 | 受限：`skin.xml: skin.xml: missing resource xml/xuiobjects.xml` |
| HeadAMP.wal | 既有 | 基本验证通过（范围见上文） |
| Heroes.wal | 新增 | 受限：`xml/player-normal.xml: <snappoint id='eqdock'> unsupported XML control` |
| IMAX_Nascar_3D.wal | 新增 | 受限：`xml/player-normal.xml: <layer id='Volume pointer'> unsupported attribute: region` |
| jvc.tape.v0.5.wal | 新增 | 受限：`skin.xml: skin.xml: missing resource xml/video.xml` |
| K-Tech_SFX_-_201.wal | 新增 | 受限：`xml/player-def.xml: <button id='pl'> unsupported toggle component: pledit` |
| KameleonDUI.wal | 新增 | 受限：`skin.xml: external/system XML resource required: path variable` |
| Komodo_Vanguard_v0.8_by_Victhor.wal | 新增 | 受限：`skin.xml: engine/load.xml: engine/xml/player.xml: external/system XML resource required: path variable` |
| lexicon.wal | 新增 | 受限：`skin.xml: <button id='pledit'> unsupported toggle component: guid:{45f3f7c1-a6f3-4ee6-a15e-125e92fc3f8d}` |
| Meroko-Proto.wal | 既有 | 受限：`skin.xml: external/system XML resource required: path variable` |
| MerokoAndMitsuki.wal | 既有 | 基本验证通过（范围见上文） |
| MerryChristmas.wal | 新增 | 受限：`xml/player-groups.xml: <togglebutton id=''> unsupported action: eq_auto` |
| MMD3-4-5.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eqauto'> unsupported action: eq_auto` |
| MMD3.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eqauto'> unsupported action: eq_auto` |
| nullsoft_media_player_10_forked_by_hb860-d7h03zd.wal | 新增 | 受限：`skin.xml: xml/player.xml: xml/player.xml: missing resource player-fullscreen.xml` |
| Objection.wal | 既有 | 基本验证通过（范围见上文） |
| Pimeer_Modern_v2.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eq'> unsupported toggle component: egaliseur` |
| Pimeer_v2-2_Ultime.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eq'> unsupported toggle component: egaliseur` |
| Pimeer_v2-3_Ultime.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eq'> unsupported toggle component: egaliseur` |
| Pimeer_v2.4.1.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eq'> unsupported toggle component: egaliseur` |
| Pimeer_v2.4.3.wal | 新增 | 受限：`xml/player-normal.xml: <button id='eq'> unsupported toggle component: egaliseur` |
| Players.wal | 既有 | 基本验证通过（范围见上文） |
| PokemonDS.wal | 既有 | 基本验证通过（范围见上文） |
| Quinto Black CT v1.6.wal | 新增 | 受限：`xml/player.xml: <togglebutton id='eq.button.auto.id'> unsupported action: eq_auto` |
| Quinto_Black.wal | 新增 | 受限：`xml/player-groups.xml: <albumart id='image.not.found.id'> unsupported XML control` |
| Retrotech.wal | 新增 | 受限：`skin.xml: <button id='pledit'> unsupported toggle component: guid:{45f3f7c1-a6f3-4ee6-a15e-125e92fc3f8d}` |
| SingItKitty.wal | 既有 | 受限：`xml/player.xml: <button id='eqauto'> unsupported action: eq_auto` |
| Space_Invaders_by_flatmatt_1.wal | 新增 | 受限：`scripts/invaderanim.maki: unsupported static MAKI class` |
| StarTrek_LCARS_AMP_PADD_II.wal | 新增 | 受限：`skin.xml: standardframe/standardframe.xml: standardframe/standardframe.xml: missing resource system-elements.xml` |
| The_Unauthorized_Matrix_Skin.wal | 新增 | 受限：`skin.xml: skin.xml: missing resource xml/standard-colors.xml` |
| TRON___Legacy.wal | 新增 | 受限：`xml/player.xml: missing group player.normal` |
| TwilightDuo.wal | 既有 | 受限：`xml/player-main-eq.xml: <button id='eqauto'> unsupported action: eq_auto` |
| Ujola Cat.wal | 既有 | 受限：`xml/player-console-left.xml: <button id=''> unsupported action: eq_auto` |
| Wiimote.wal | 既有 | 受限：`xml/player-mode1.xml: <togglebutton id='op'> unsupported toggle component: spacecatsamba` |
| Winamp 3.0 Default.wal | 新增 | 受限：`skin.xml: skin.xml: missing resource xml/gamma-presets.xml` |
| Winamp Modern Holiday Skin 2013.wal | 新增 | 受限：`standardframe/standardframe.xml: <layer id='window.top.left'> unsupported attribute: resize` |
| WMP11-BlueVU.wal | 新增 | 受限：`xml/standardframe.xml: <layer id='regiontopright'> unsupported sysregion operation` |
| Wriothesley - Beyond the Rules.wal | 既有 | 受限：`xml/player.xml: <snappoint id='playerbase'> unsupported XML control` |
| xxxHolic.wal | 既有 | 基本验证通过（范围见上文） |
| ZDL-AUDIOPHILE_Stereo-System_5-Plus.wal | 新增 | 受限：`xml/player-normal.xml: <status id='status'> unsupported XML control` |
| ZDL_Reel-To-Reel_Analog_Tape_Machine.wal | 新增 | 受限：`xml/player-normal.xml: <status id='status'> unsupported XML control` |

## 本地构建产物

已更新 `rebuild/build/Release/AddIn/ttp_waskin.dll`（693760 字节），普通版与 XP/Win7 版共用。`ttp_maki.dll` 本轮未改动。

已更新普通版及 XP/Win7 版 EXE，并重建 `TTPlayerRebuild-2026.09.24.zip`、`TTPlayerRebuild-XP-Win7-2026.09.24.zip`。沿用旧包结构：ZIP 内仅含 `TTPlayerRebuild.exe` 和 `SHA256SUMS.txt`，共用 DLL 单独放 AddIn。压缩包内 EXE 哈希已核对。

替换前文件保存在 `rebuild/out/test-artifacts/before-wal-new-20260924-155128`。本轮构建哈希清单见 `rebuild/out/test-artifacts/wal-new/release-manifest.json`。

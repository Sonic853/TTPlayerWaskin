# WSZ 首期实现记录

日期：2026-09-19。

本次按最新要求实现 `AddIn/ttp_waskin.dll`。此前可行性文档中“专用目录”和“DLL 只输出布局、EXE 绘制”的方案已调整：实际插件与其他 AddIn 共目录，由 DLL 自行解析、绘制和处理皮肤输入。

## 源码结构

| 文件 | 职责 |
| --- | --- |
| [archive.cpp](../src/archive.cpp) | ZIP 目录解析、内存解压、CRC32、包大小/展开大小/条目数边界 |
| [skin.cpp](../src/skin.cpp) | BMP 与 RLE 解码、经典图集、三个窗口绘制、区域、鼠标输入、预览和解绑 |
| [plugin.cpp](../src/plugin.cpp) | ABI 入口、版本验证、错误返回及异常边界 |
| [tooltips.cpp](../src/tooltips.cpp) | 实际命中区域的控件提示、宿主文字／快捷键回调与旧系统兼容 |
| [layout.cpp](../src/layout.cpp) | 窗口布局快照、折叠／缩放状态解释、旧矩形恢复及状态校验 |
| [video.cpp](../src/video.cpp) | 视频窗口外框、五按钮、内容模式与菜单；见 [视频内容窗口](VIDEO_CONTENT.md) |
| [spectrum.cpp](../src/spectrum.cpp)、[visual.cpp](../src/visual.cpp) | Winamp 默认经典频谱的独立点阵、逐行颜色、峰值动画及边界绘制；复用宿主分析快照 |
| [ttp_skin_plugin.h](../include/ttp_skin_plugin.h) | 版本化 C ABI 和宿主状态/命令协议 |
| [skin_plugin.cpp](../../rebuild/src/skin/skin_plugin.cpp) | EXE 的通用模块和实例生命周期 |
| [player_window_skin_plugin.cpp](../../rebuild/src/ui/player_window_skin_plugin.cpp) | EXE 的播放器状态查询与命令桥接 |

以上链接相对于 `docs`：插件源码位于 `../src`，宿主位于 `../../rebuild`。

## 生命周期和线程

插件在创建实例时完成完整加载和校验，成功后才绑定窗口。切换失败保留上一个有效皮肤；切回 TTPlayer 皮肤时先解绑 DLL，再走既有重绑定流程。

原生与 Winamp 选择分别保存，原生包先加载并为歌词等窗口提供后备资源；选择 Winamp 不覆盖原生名称，选择原生则清空 Winamp。规则见 [SKIN_SELECTION.md](SKIN_SELECTION.md)。

`probe` 可由目录扫描线程调用，使用独立解析对象，不访问活动皮肤。实例绑定、绘制、预览、状态查询和销毁在 UI 线程进行。
宿主命令回调只投递消息；打开菜单、关闭窗口和切换皮肤等操作在回到宿主消息循环后执行，避免 DLL 仍在回调栈中就被销毁。

接口使用固定宽度数值、UTF-16 缓冲区、Windows 句柄及不透明实例指针。没有跨模块传递 STL 对象；实例由创建它的 DLL 销毁，预览 HBITMAP 由调用者 `DeleteObject`。
模块使用共享所有权，皮肤实例及后台扫描完成前不会卸载。

窗口移动及缩放使用 ABI v1 的可选 `drag` 回调，同步复用宿主原生拖动状态机。该回调只更新窗口几何和捕获，不执行播放命令或换肤。旧长度接口按大小读取，避免新增字段越界；详见 [WINDOW_DOCKING.md](WINDOW_DOCKING.md)。

## 校验范围

- Stored、固定 Huffman、动态 Huffman 及 DEFLATE 非压缩块。
- 包文件名大小写、单层嵌套根目录、可选图片缺失。
- 调色板位图、RLE4、RLE8，以及本地 Winamp 经典资源包。
- CRC 错误、截断包、截断位图、重复资源、多个候选根目录、路径穿越、异常声明大小和现代 XML 包拒绝。
- 主窗口、列表、均衡器像素及参数命令映射。
- 30 次创建、绑定、绘制、解绑和销毁后，GDI 对象数量稳定；首次系统绘制缓存单独预热。
- 真实 `PlayerWindow`：AddIn 识别、保存的 WSZ 启动恢复、目录菜单、独立预览、错误包保持现状、10 次 WSZ/原生皮肤切换及 HWND 身份保留。
- 构建只输出一份使用 XP 工具链的 x86 DLL，两版宿主共享此二进制；通过 C ABI 隔离宿主与插件运行库。
- Winamp 包仅从 `Skin/waskin` 扫描，配置保留 `waskin` 命名空间；外部打开和拖入先安装至此目录。
- 选项页按已加载插件接口显示原生／Winamp 标签，默认按活动皮肤选中；覆盖两类预览、反复切换、筛选后应用、空列表，以及无可用 DLL 时的原生布局。
- 本轮普通版及 XP／Win7 版宿主均在当前 Windows 主机加载同一份 DLL 通过集成测试；两处安装文件 SHA-256 一致。DLL 缺失及同名文件无皮肤接口两种情况也均通过验证。
- XP / Win7 构建对 DLL 的静态导入执行独立审计；这不等于已在 XP、Win7 来宾系统完成界面运行验证。

实际兼容边界见 [README.md](../README.md)；本轮不是 Modern / Bento 支持，也不是所有 WSZ 皮肤及 Winamp 行为的全面兼容认证。

2026-09-20 补充：修复有效短图集的错误后备绘制、标题背景、列表标题栏取图顺序及底部固定数字槽／时长统计，详见 [Pink hearts 绘制修复](PINK_HEARTS_RENDERING.md)。

同日补充：新增 DLL 内的默认经典频谱样式，修复效果切换时原生可视化子窗口短暂露出的尺寸错误，详见 [频谱与切换修复](CLASSIC_SPECTRUM.md)。

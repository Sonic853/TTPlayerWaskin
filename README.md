# ttp_waskin

TTPlayerRebuild 的可选 Winamp 经典皮肤插件，输出名称为 **`ttp_waskin.dll`**。

## 使用

1. 使用已接入皮肤插件接口的新版 `TTPlayerRebuild.exe`。
2. 将 `ttp_waskin.dll` 放入 **该 EXE 同目录下的 `AddIn`**。普通版与 XP／Win7 版共用完全相同的 DLL。
3. 将经典 `.wsz` 放入该运行目录的 **`Skin/waskin`**，启动播放器后从皮肤菜单或选项中的皮肤页选择。此前放在 `Skin` 或 `Skin/new` 的 Winamp 包需要移入此目录。
4. 也可以将 `.wsz` 拖入主窗口安装，或通过命令行打开 `.wsz`，程序会先复制至 `Skin/waskin` 再加载。

选项 → 皮肤：检测到有效 Plugin Skin 接口后，顶部显示“原生”和各 DLL 定义的标签，打开时默认选中当前使用的插件。右键菜单的皮肤分组也使用同一名称。本 DLL 声明名称 **“Winamp”**、子目录 **`waskin`**、后缀 **`.wsz;.wal`**，宿主因此读取 `Skin/waskin` 直属目录。空目录显示空列表，禁用应用和删除。没有可用插件时不显示标签。

插件皮肤的配置名称保存为相对于 `Skin` 的完整路径，例如 `waskin/文件名.wsz`。目录与类型由 DLL 声明，宿主不再硬编码 `waskin` 或 `.wsz/.wal`。

程序在启动时发现插件。运行期间新放入 DLL，需要重启后再选择 WSZ。插件缺失或 Winamp 皮肤损坏时，启动后使用已选原生皮肤；原生包也不可用时才回退到内置默认皮肤。

原生选择保存为 `Skin/PackageName`，插件选择单独保存为 `Skin/CustomPackageName`。未接管的窗口使用已选原生皮肤；应用原生皮肤会清空插件选择。**不读取或迁移旧的 `WinampPackageName`，也不从旧的合并字段恢复插件选择。** 配置示例见 [皮肤选择配置](docs/SKIN_SELECTION.md)。

当前版本实现的是经典 WSZ。**Modern / Bento 的 XML、Wasabi、MAKI 运行时尚未实现，包含现代 `skin.xml` 的包会被拒绝。** 不会将现代皮肤中的后备 `main.bmp` 当作完整现代皮肤加载。

## 实现边界

完整的资源、控件切图、输入状态及当前缺项对照见 [WSZ 控件解析分析](docs/WSZ_CONTROL_PARSING.md)。下面列出的是首期基础能力，不表示已完整还原全部经典控件。

DLL 内实现：

- ZIP 容器识别，Stored / DEFLATE 解压，CRC32 校验和资源大小限制；只在内存中读取，不解压到磁盘。
- 根目录或单个子目录中的经典资源识别，文件名大小写兼容。
- BMP RGB / Bitfields，以及 RLE4 / RLE8 解码。
- 主窗口、播放列表、均衡器的位图绘制及各自输入处理。
- 标准按钮图集、音量/平衡/定位、时间与标题、位图文字、播放列表颜色、窗口区域和折叠模式。
- 独立预览图生成，窗口消息子类化，原生子控件隐藏/恢复，换肤时的资源清理。
- 鼠标悬停在插件窗口时的滚轮转发，不要求先获得焦点。

EXE 中的通用接入层负责：

- 从 `AddIn/ttp_*.dll` 发现 `ttpGetSkinPlugin`，校验版本并保活模块。
- 将外部皮肤加入已有菜单、选项预览、拖入安装和启动恢复流程。
- 向 DLL 提供播放状态、可见歌曲列表，并执行 DLL 提交的播放/设置命令。
- 通过同步拖动回调复用 TTPlayer 原生的吸附、分离及相连窗口批量移动，统一使用常规选项中的吸附设置。

主程序没有 WSZ 解压器、Winamp 图集坐标表或经典绘制代码。DLL 直接绘制现有三个 HWND，不生成 TTPlayer `Skin.xml`，不替换音频引擎。歌词和选项界面继续使用 TTPlayer 的原生实现。

### 当前支持范围

已按控件分析修复主窗口／EQ／播放列表的主要绘制、三种折叠模式、滑块预览、列表多选与内部拖动、光标、资源回退和双倍尺寸。

功能优先复用千千静听：五组列表按钮打开原生添加／删除／编辑／排序／列表菜单；EQ 使用原生参数与预设；可视化使用千千已有渲染器。完整映射、验证与未支持项见 **[修复与原生功能替代表](docs/NATIVE_FUNCTION_MAPPING.md)**。

Auto 按歌曲自动装载 EQ、Winamp 视频／TV、Winamp 插件和现代 WAL 运行时没有等价原生替代，未伪装为其他功能。通用对话框与歌词继续使用所选千千原生皮肤。

随机按钮映射到 TTPlayer 随机模式（再次点击恢复顺序模式）；重复按钮在列表循环与顺序模式之间切换，已有单曲循环也显示为选中。皮肤使用宿主已有的循环与随机索引策略。

两个 EXE 与 DLL 需要配套更新才能使用新增功能。两版共用同一份 DLL；旧 v1 宿主仍可加载。窗口吸附依据见 [窗口吸附与分离](docs/WINDOW_DOCKING.md)。尚未声称所有 WSZ 变体、全部手势和像素完全一致，剩余边界已列入替代表。

## 构建

统一 x86 版本（兼容普通版和 XP／Win7 版宿主）：

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A Win32
cmake --build build --config Release --parallel 4
```

唯一分发产物：`build/Release/ttp_waskin.dll`。构建始终使用同级 `rebuild` 的 VC-LTL／YY-Thunks 旧系统工具链，并在链接后执行 XP、Win7 导入审计，不再提供普通／旧系统 DLL 的构建开关。接口不跨模块传递 CRT 内存或 STL 对象，因此同一份 DLL 可供两种运行库的宿主使用。安装时同时附带第三方许可说明。

可以安装至指定播放器目录：

```powershell
cmake --install build --config Release --prefix "D:/path/to/TTPlayer"
```

宿主接口定义见 [include/ttp_skin_plugin.h](include/ttp_skin_plugin.h)。目前宿主仓库保留相同内容的 SDK 头文件；变更 ABI 时需同步两端并升级版本。

## 本地验证

测试代码位于 `../rebuild/tests/waskin`，不进入此插件的构建目标、分发包或 Actions。

已验证的内容和限制见 [docs/WSZ_IMPLEMENTATION.md](docs/WSZ_IMPLEMENTATION.md)。本工程独立实现兼容逻辑；没有链接 `gen_ff`、Wasabi 或把 Winamp 默认皮肤素材嵌入 DLL。本地对照使用的 Winamp 资源仅保存在忽略的测试输出目录。

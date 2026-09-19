# ttp_waskin

TTPlayerRebuild 的可选 Winamp 经典皮肤插件，输出名称为 **`ttp_waskin.dll`**。

## 使用

1. 使用已接入皮肤插件接口的新版 `TTPlayerRebuild.exe`。
2. 将 `ttp_waskin.dll` 放入 **该 EXE 同目录下的 `AddIn`**。普通版与 XP／Win7 版共用完全相同的 DLL。
3. 将经典 `.wsz` 放入该运行目录的 **`Skin/waskin`**，启动播放器后从皮肤菜单或选项中的皮肤页选择。此前放在 `Skin` 或 `Skin/new` 的 Winamp 包需要移入此目录。
4. 也可以将 `.wsz` 拖入主窗口安装，或通过命令行打开 `.wsz`，程序会先复制至 `Skin/waskin` 再加载。

选项 → 皮肤：检测到有效皮肤插件接口后，顶部显示 **“原生”／“Winamp”** 标签，打开时默认选中当前正在使用的皮肤类型。“原生”保留默认皮肤及 `Skin`、`Skin/new` 中的原生包；“Winamp”只列出 `Skin/waskin` 直属目录中插件能够识别的 `.wsz`、`.wal`。空目录显示空列表，禁用应用和删除。插件缺失、位数错误或接口不兼容时不显示这两个标签。

Winamp 包的配置名称保存为 `waskin/文件名`；旧的纯文件名 `.wsz`／`.wal` 配置也只从 `Skin/waskin` 恢复，不回头扫描根目录或 `new`。

程序在启动时发现插件。运行期间新放入 DLL，需要重启后再选择 WSZ。插件缺失或 Winamp 皮肤损坏时，启动后使用已选原生皮肤；原生包也不可用时才回退到内置默认皮肤。

原生选择保存为 `Skin/PackageName`，Winamp 选择单独保存为 `Skin/WinampPackageName`。Winamp 未接管的窗口使用已选原生皮肤；应用原生皮肤会清空 Winamp 选择。界面不变，配置示例和旧配置迁移规则见 [皮肤选择配置](docs/SKIN_SELECTION.md)。

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

### 首期支持范围

窗口吸附修复需要同时更新 EXE 和 DLL；普通版与 XP／Win7 版继续共用同一份 DLL。原版依据、调用边界与验证见 [窗口吸附与分离](docs/WINDOW_DOCKING.md)。

主窗口的播放、暂停、停止、上一首/下一首、打开文件、窗口开关、音量、平衡、定位、随机/重复和时间模式已连接宿主。
播放列表支持歌曲显示、单行选择、双击播放、滚轮、滚动条、窗口调整尺寸，以及添加/移除入口；均衡器支持开关、预设菜单、前级和十段参数。

随机按钮映射到 TTPlayer 随机模式（再次点击恢复顺序模式）；重复按钮在列表循环与顺序模式之间切换，已有单曲循环也显示为选中。皮肤使用宿主已有的循环与随机索引策略。

这是 WSZ 读取及显示的第一阶段，尚未宣称与 Winamp 全部行为一致。以下仍不属于完整实现：

- Winamp 可视化频谱、完整 EQ 曲线显示、Auto 自动预设、`gen.bmp` 通用窗口和自定义光标。
- Winamp 播放列表的全部弹出子菜单、多选/重排手势、经典双倍尺寸模式。
- 所有经典皮肤变体的像素一致性；可选图片缺失时部分控件采用文字或简单绘制后备。
- 各窗口折叠状态及插件内部滚动位置的持久化；附属窗口在绑定时按主窗口位置排列。

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

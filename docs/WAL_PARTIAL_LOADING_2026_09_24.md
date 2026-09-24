# WAL 部分加载与不支持控件的禁用

日期：2026-09-24。接续 [新增皮肤检查报告](WAL_NEW_SKINS_AUDIT_2026_09_24.md)。

## 行为

插件自动对可选功能降级，无须修改 WAL 或在选项窗口增加开关。

| 情况 | 运行时处理 |
|---|---|
| `EQ_AUTO` 等尚未映射的按钮动作 | 保留普通状态图片，禁用鼠标命中及点击；脚本 `leftClick` 也不能绕过 |
| 普通 `EQ_TOGGLE`、播放等已支持动作 | 继续调用千千静听原有功能 |
| 按下、悬停或激活图片缺失 | 回退到普通状态图片 |
| 主图片缺失、未知控件或未实现的嵌入组件 | 跳过该控件，保留可解析的子控件 |
| 缺少 include、继承组、系统模板或脚本文件 | 记录诊断，继续处理包内已有内容 |
| 不支持的 MAKI 类、导入或字节码 | 跳过对应脚本，保留其他脚本和原生动作 |
| 脚本初始化中途失败 | 禁用该脚本，还原初始化前的节点属性、可见性、裁剪区域及内部整数设置，避免先隐藏窗口再报错 |
| 单个脚本运行时失败 | 禁用该脚本、停止其自有计时器及动画；保留其他脚本和原生命令 |
| 脚本数量超限 | 最多处理 128 个候选、保留 32 个实例，跳过额外脚本 |

部分兼容的诊断接口返回成功状态 `S_FALSE`，附带被跳过的功能。宿主仍可预览、应用皮肤，不将降级诊断视为致命错误。

`EQ_AUTO` 控制 Winamp 自动装载均衡器设置，不是均衡器启用开关。依据本地 `Src/Wasabi/api/core/coreactions.cpp`、`api/skin/widgets/button.cpp` 和 `Src/Winamp/Eq.cpp` 的 `eq_autoload`，未将它错误映射成 `EQ_ENABLE`。

## 仍然保留的检查

ZIP/CRC 错误、非法 XML、路径越界、循环 include/继承、图片或对象容量超限、无效主布局仍拒绝加载。初始化后的主画面必须存在可见像素，避免得到无法操作的透明窗口。

57 个本地样本均能识别，其中 54 个允许完整或部分加载。已检查创建、挂接真实 Win32 窗口、定时刷新、原生右键、预览、保存布局及销毁重建恢复。**这不表示 54 个皮肤的所有控件、脚本和外观均已恢复。**

| 仍拒绝加载的皮肤 | 原因 |
|---|---|
| `Coca_Cola__My_Coke_Music.wal` | 相对布局模式超出当前支持的 0/1/2 |
| `Komodo_Vanguard_v0.8_by_Victhor.wal` | `colorthemes.xml` 无法按当前 UTF-8 XML 路径解析 |
| `Winamp 3.0 Default.wal` | 缺失资源后主布局没有可见内容 |

多容器、系统模板、配置、PopupMenu、Layer FX 等未实现功能仍会影响部分皮肤的外观或交互。跳过整份不兼容脚本，也会使该脚本中原本可能支持的行为失效。

## XP 动态加载修复

VirtualBox XP 中复现了另一处旧问题：动态加载 DLL 后探测内置默认 WSZ，发生 `0xc0000005`。调试器定位到访问未初始化资源表；顶层异常处理将其转换成加载失败。

MSVC 局部静态初始化使用隐式 TLS，XP 对动态加载 DLL 的静态 TLS 存在限制。依据微软文档：[局部静态初始化](https://learn.microsoft.com/en-us/cpp/build/reference/zc-threadsafeinit-thread-safe-local-static-initialization?view=msvc-170)、[DLL 数据与 TLS](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-data)。

共享 DLL 禁用这项编译器初始化机制。内置资源和 MAKI 类表改用 `LazyCache`：Interlocked 操作负责跨线程发布，构造失败允许重试，等待线程使用 `Sleep(1)`，缓存随 DLL 卸载释放。按钮动作和控件类型使用常量表。后台皮肤扫描仍然线程安全。

## 验证与交付

本地 C++ 测试源仅在 `rebuild/tests/waskin`，不上传、不加入 Actions 或发布包：

- `modern_partial_tests.inc`：失败初始化回滚、缺失状态图、EQ_AUTO 禁用、普通 EQ/播放继续工作；实际 Diablo II 皮肤按钮验证。
- `modern_dll_tests.cpp`：并发首次初始化默认皮肤，原有 8 个样本主要交互、抽屉、保存恢复和资源处理。
- `wal_catalog_tests.cpp`：57 个样本的批量创建、挂接及恢复。
- `modern_host_tests.inc`：部分兼容皮肤可预览/应用，致命错误仍显示诊断；HeadAMP 冷启动两次、OLE 拖入和皮肤切换。

结果日志与交付哈希位于 `rebuild/out/test-artifacts/wal-partial`。

已部署到 `rebuild/build/Release/AddIn/ttp_waskin.dll` 和 `rebuild/out/legacy-xp/Release/AddIn/ttp_waskin.dll`，两份一致：702464 字节，SHA-256 为 `cae0a5a851c54b93dacc3083dc1d6c1a9e035c7081b84d7040a34e5d5a9627c5`。替换前 DLL 保存在 `rebuild/out/test-artifacts/before-wal-partial-20260924-164753`。

VirtualBox 的 Windows 7 SP1 和 Windows XP（5.1.2600）均通过真实 DLL 回归；XP 另验证了并发首次初始化。两台来宾的最终主程序集成均通过，包括两次 HeadAMP 冷启动、降级皮肤预览/应用、播放设置、OLE 拖入及切回原生/WSZ。来宾使用独立 `C:\TTPlayerWalPartial20260924` 测试目录，结束后均恢复到原来的保存状态。

| 检查 | 最终结果 / 日志 |
|---|---|
| 本机 57 个 WAL 批量创建、挂接、预览及恢复 | 54 个可完整或部分加载；`catalog-final.txt` |
| 本机 WSZ 与内置默认皮肤 | 通过；`waskin_tests.txt`、`waskin_builtin_tests.txt` |
| Win7 SP1 DLL 回归 | 通过；`win7-runtime.txt` |
| XP DLL 回归及并发首次缓存初始化 | 通过；`xp-runtime.txt`；调试器未再出现 `0xc0000005` |
| Win7 SP1 主程序集成 | 通过；`win7-host-final.txt` |
| XP 主程序集成 | 通过；`xp-host-final.txt` |

虚拟机复测覆盖上述具体场景，不代表所有 WAL 的全部功能均已恢复。测试辅助程序允许较慢的 XP 来宾最多运行 300 秒；最初 90 秒的调试运行在后续样本处超时，最终完整运行已通过。

本次功能集中在 `ttp_waskin.dll`，沿用独立 `ttp_maki.dll`，未引入 Wasabi 服务。两版播放器共用同一份插件；本次无需更换 EXE 或原有 ZIP。

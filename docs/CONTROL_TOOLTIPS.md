# Plugin Skin 控件悬停提示修复

## 原因

插件接管主窗口、播放列表和均衡器后，宿主仍在公共 Tooltip 中注册原生后备皮肤的控件矩形，并在 `PreTranslateMessage` 中转发鼠标消息。DLL 绘制、命中的位置已经变为 WSZ 坐标，提示却仍按后备皮肤的位置查询，所以会出现错位提示。此前 DLL 没有自己的提示实现。

## 修复

- 宿主在插件绑定成功和失败回滚时移除被接管窗口的原生提示；后续布局刷新也不再注册这些提示。鼠标消息不再转给原生 Tooltip。
- DLL 的 `Hit` 同时返回控件编号和实际命中矩形，`tooltips.cpp` 使用同一结果注册提示，避免维护第二套坐标。
- 每个窗口按需创建一个 Tooltip，只登记当前悬停控件。主窗口、列表工具栏／滚动条、EQ 按钮／滑块、折叠状态和双倍尺寸共用此机制；列表右侧按钮跟随窗口宽度移动。
- 标题拖动区、空白区域和歌曲行不冒用原生控件提示。原生歌词窗口继续使用宿主提示。
- 按下、拖动、离开窗口、隐藏、缩放、禁用及失活时收起提示；换肤和窗口销毁时释放 Tooltip。切回原生皮肤会恢复宿主的提示区域。

## 文案与接口

`TtpSkinHost` 尾部新增可选 `tip(context, command, value, text, count)`，仍使用 ABI v1 和 `size` 协商。返回 `FALSE` 时由 DLL 提供文案；输出缓冲区由调用方持有，`count` 是包含结尾空字符的宽字符容量，回调在 UI 线程执行。

- 播放、暂停、上一首／下一首等使用宿主资源文字和用户配置的快捷键。
- `TTP_SKIN_LIST_TOOLBAR` 的 `value` 仍为原生工具栏菜单位置。
- EQ 滑块以 `TTP_SKIN_EQ_VALUE` 查询，`value=0` 表示前置增益，`1..10` 表示频段；宿主提供数值，DLL 补充控件名称。
- 音量、平衡、随机／循环、折叠、双倍尺寸等由 DLL 按实际状态描述；没有对应功能的 EQ Auto 明确提示暂不支持。
- 旧长度宿主保留已有 `visual` 回调，不读取不完整的 `tip` 字段，并使用 DLL 的后备文案。

Tooltip 使用 Unicode 通知及 `TTTOOLINFOW_V2_SIZE`。普通控件 v5 会拒绝包含 v6 保留尾字段的完整结构体，而本功能不需要该字段；结构体版本说明见 [Microsoft Common Control Versions](https://learn.microsoft.com/en-us/windows/win32/controls/common-control-versions)。

完整修复需同时更新 EXE 和 `AddIn/ttp_waskin.dll`。普通版和 XP／Win7 版继续共用一份 DLL。

## 本地验证

测试源码位于 `rebuild/tests/waskin`，不上传、不进入发布包，Actions 不增加测试步骤。

- 独立 DLL：控件文字／矩形、空白区域、折叠、双倍尺寸、列表缩放、离开清理、销毁清理及旧 ABI 保护页。
- 两版宿主：禁止原生提示重新注册、保留歌词提示、读取快捷键设置、原生／插件切换清理与恢复，以及实际鼠标悬停弹出的窗口和文字。
- 保留既有资源解析、播放命令、拖动吸附、进度定位、选项标签、皮肤配置及原生换肤／均衡器回归。

运行验证使用当前 Windows 环境；XP／Win7 兼容性另做静态导入审计，未执行旧系统真机或虚拟机测试。

### 验证结果（2026-09-20）

- 两版宿主和统一 DLL 的 Release 构建、独立 DLL 测试、两版完整宿主测试均通过，包含实际鼠标悬停弹窗检查。
- `skin_rebind_tests`、`equalizer_recovery_tests` 通过（2/2）。
- XP／Win7 导入审计通过：DLL 为 5 个依赖 DLL、162 项导入；旧系统 EXE 为 19 个依赖 DLL、648 项导入。
- DLL 已安装到两版 `Release/AddIn`；两处安装文件与 `waskin/build/Release/ttp_waskin.dll` 的 SHA-256 相同：`DC67C78E3E6894FC530F994BF14DE5FA7CC5A064F163802D9CA118F26CDDD899`。

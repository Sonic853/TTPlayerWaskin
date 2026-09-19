# 原生与 Winamp 皮肤选择分别保存

配置文件 `TTPlayerRebuild.xml` 的 `Skin` 节点使用两个属性：

```xml
<Skin PackageName="TT2012.skn" WinampPackageName="waskin\classic.wsz"/>
```

- `PackageName`：原生皮肤，保留原有字段。Winamp 未接管的窗口（例如歌词窗口、桌面歌词工具栏）继续使用此皮肤的资源。
- `WinampPackageName`：Winamp 皮肤选择，路径相对于 `Skin`，仅从 `Skin/waskin` 加载。

选择 Winamp 时只修改第二个属性，保留原生选择及已经加载的原生皮肤资源。启动时先加载原生皮肤，再让 DLL 接管主窗、播放列表和均衡器。原生皮肤没有均衡器布局时，也不会为了显示 Winamp 均衡器而替换整个原生皮肤。

应用原生皮肤后清空 Winamp 选择，包括重新选择当前记住的同一原生皮肤或默认皮肤：

```xml
<Skin PackageName="TT2012.skn" WinampPackageName=""/>
```

写入空属性会覆盖旧配置中的 Winamp 值，不会在下次启动时恢复已经取消的选择。无效皮肤切换不改变当前成功加载的选择。选项窗口的标签、布局、预览和操作入口保持原有界面。

## 兼容与失败恢复

- 旧版本把 WSZ／WAL 写进 `PackageName`（或旧别名 `SkinFile`）时，读取阶段会移入 `WinampPackageName`，原生选择使用默认皮肤。旧文件没有保留此前原生皮肤名称，无法据此恢复那一项。
- 明确写出的空 `WinampPackageName` 优先，不从旧字段重新启用 Winamp。
- DLL 暂时缺失或 Winamp 包读取失败时，显示已选原生皮肤，并保留独立的 Winamp 请求；用户应用原生皮肤则明确清空该请求。
- 原生包本身缺失时才回退到内置默认原生皮肤。
- 运行 Winamp 时，窗口布局保存到该 Winamp 包的侧边配置（例如 `Skin/waskin/classic.wsz.xml`），不把 Winamp 的窗口尺寸写入原生包的 `.skn.xml`。

本地测试覆盖 XML 迁移、两个字段的保存与清空、重启恢复、歌词资源保持、无 DLL 后备、损坏包、重新选择同一原生皮肤，以及原生缺少均衡器布局的启动场景。测试位于 `rebuild/tests/waskin`，不分发、不在 Actions 执行。

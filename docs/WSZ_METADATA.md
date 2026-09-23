# 外部 WSZ 的名称与联系方式

依据本地 Winamp 源码 `Src/Winamp/setup/skininfo.cpp`：经典包从 `skininfo.xml` 读取 `SkinInfo` 信息，`SkinXMLCallback::xmlReaderOnCharacterDataCallback` 按字段名识别内容。

## 字段映射

| XML 元素 | 插件返回值 | 宿主显示 |
| --- | --- | --- |
| `name` | `TtpSkinInfo.name` | 皮肤列表及右键皮肤菜单 |
| `author` | `TtpSkinInfo.author` | 作者 |
| `email` | `TtpSkinInfo.email` | 邮箱 |
| `homepage` | `TtpSkinInfo.website` | 主页 |

```xml
<?xml version="1.0" encoding="UTF-8"?>
<SkinInfo>
  <name>Example Skin</name>
  <author>Example Author</author>
  <email>author@example.com</email>
  <homepage>https://example.com/</homepage>
</SkinInfo>
```

支持根目录或子目录中的 `skininfo.xml`，文件名与字段名不区分大小写。根元素可以是 `SkinInfo`，也可以是 `WinampAbstractionLayer`／`WasabiXML` 内的直接 `SkinInfo` 子元素。不从其他元素下的同名字段取值。

采用与宿主现有 XML 功能相同的 MSXML 6，直接解析 ZIP 中的原始字节，支持声明的编码、UTF-8／UTF-16 BOM、XML 转义及 CDATA。去掉字段首尾的空格、制表和换行，保留正文。各字段按接口容量截断，截断时不拆开 UTF-16 代理对。

缺失、空元素或只有空白的字段均返回空字符串。没有名称时，宿主以包含扩展名的包文件名显示列表／菜单条目，**不会把文件名写回名称元数据**。原始路径仍是配置与换肤使用的身份标识，名称相同的包可以独立选择。

不解析 README 中的自由文本，不猜测工具作者、邮箱或主页。格式错误、超过 256 KiB、含 DTD 或不支持根结构的 XML 不提供元数据，但有效的经典皮肤仍可预览和加载。禁止 DTD 与外部实体解析，不读取元数据中引用的本地文件或网络内容。ZIP／CRC 错误及歧义的重复资源仍按原有压缩包错误处理。

## 接口和界面

`TtpSkinInfo` 在原有 `size/name/author` 后追加可选 `email/website` 数组，ABI 版本保持 v1。新版 DLL 只写调用方提供的完整字段，旧结构和不完整尾部不会被越界写入。新版宿主先清空结构，旧 DLL 没有提供的字段保持空白。

皮肤页沿用现有“作者／主页／邮箱”控件。每次选择时更新全部字段，避免残留前一款皮肤的信息；作者和 URL 中的 `&` 按字面显示，不作为快捷键标记。预览、应用和配置仍使用包路径。完整功能需要配套更新两版宿主和共用 DLL。

## 验证

本地测试位于 `rebuild/tests/waskin`，不进入分发 ZIP 或 Actions。覆盖四字段、缺失／空白／格式错误、嵌套包路径、UTF-8／UTF-16／GB2312／Windows-1252、实体与 CDATA、Unicode 截断、旧接口保护页、STA 调用及并发扫描。两版宿主验证目录、真实选项控件、预览、字段清空、菜单显示和配置身份。保留默认皮肤、窗口像素、旧接口与资源释放回归，并执行 XP／Win7 静态导入审计。

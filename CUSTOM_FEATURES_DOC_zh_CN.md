# Stellarium 自定义功能开发文档

## 概述

本文档记录了在 Stellarium 开源天文望远镜基础上开发的一系列自定义功能，主要围绕中国古代天文观测（入宿度、去极度）和星表加载功能进行扩展。

---

## 一、中国古代天文信息显示

### 1.1 赤道入宿度与去极度

#### 功能说明
- 计算恒星的**入宿度**（该恒星到所在星宿距星的赤经差）
- 计算恒星的**去极度**（90° - 赤纬）
- 将现代天文度数转换为中国古度（一周365.25°，而非360°）
- 支持“度以下十二分法”形式显示（如 `3+6/12` 表示 3.5°）

#### 计算方式
```
入宿度 = (恒星赤经 - 距星赤经) × 365.25 / 360
去极度 = (90° - 恒星赤纬) × 365.25 / 360
```

#### 修改文件
- [ConstellationMgr.cpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/modules/ConstellationMgr.cpp)
  - 新增 `getChineseLunarMansionCoordinate()` 方法实现核心计算逻辑
  - 新增 `isChineseLunarSystem()` 方法检测是否为中国/朝鲜/日本星空文化

- [ConstellationMgr.hpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/modules/ConstellationMgr.hpp)
  - 添加相关方法声明

- [StelObject.cpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/StelObject.cpp)
  - 修改信息显示逻辑，添加中国古代天文信息输出

#### 显示格式
```
星宿名 入宿度分数, 去极度分数, (星宿名 十进制入宿度, 十进制去极度)
```

**示例**：`心 3+6/12, 65+3/12, (心 3.50°, 65.25°)`

---

### 1.2 黄道入宿度与黄纬

#### 功能说明
- 计算恒星的**黄道入宿度**（该恒星黄道经度与距星黄道经度之差）
- 计算恒星的**黄纬**（恒星在黄道坐标系中的纬度）
- 独立使用黄道坐标判断恒星所在星宿
- 距星特判：若恒星恰好是距星，显示到上一宿的入宿度

#### 计算方式
```
黄道入宿度 = 恒星黄道经度 - 距星黄道经度
黄纬 = 恒星在黄道坐标系中的纬度值
```

#### 关键逻辑
1. 将所有距星从赤道坐标转换为黄道坐标
2. 用黄道经度独立判断恒星应落入哪个星宿
3. 用该星宿的黄道距星计算黄道入宿度
4. 若入宿度 < 0.01°（距星），回退到上一宿

#### 修改文件
- [ConstellationMgr.cpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/modules/ConstellationMgr.cpp)
  - 添加黄道坐标的独立星宿判断逻辑
  - 添加黄道入宿度计算
  - 添加距星特判处理
  - 新增 `SolarSystem.hpp` 头文件引用

#### 显示格式
```
黄道入宿度 XX.XX°(黄道星宿名), 黄纬 XX.XX°
```

**示例**：`黄道入宿度 15.23°(尾), 黄纬 5.67°`

#### 完整输出格式
```
星宿名 入宿度分数, 去极度分数, (星宿名 十进制入宿度, 十进制去极度), 黄道入宿度 XX.XX°(黄道星宿名), 黄纬 XX.XX°
```

---

## 二、星表选取框功能增强

### 2.1 功能说明

对星表加载对话框进行扩展，支持从配置文件读取待可视化的星表列表。

#### 主要特性
- **配置文件驱动**：从 `starcatalog/_catalog_list.json` 读取星表列表
- **自定义顺序**：按配置文件中的顺序列出星表
- **多级路径支持**：支持子目录路径（如 `English titles/`）
- **双击打开**：双击列表项用系统默认程序打开 JSON 文件
- **更新列表**：提供"更新列表"按钮，重新读取配置文件
- **加载选中**：支持选中多个星表批量加载

### 2.2 配置文件格式

**文件**：[starcatalog/_catalog_list.json](file:///f:/temp/stellarium_4A2.19/stellarium/starcatalog/_catalog_list.json)

```json
{
    "description": "Custom star catalog list configuration. Files are loaded in order.",
    "files": [
        "景祐星表数据1034.json",
        "皇祐星表数据1052.json",
        "陈卓星表(石氏)数据.json",
        "English titles/Tianwen_Huichao_(14th_century).json"
    ]
}
```

### 2.3 修改文件

#### 核心代码修改
- [SearchDialog.cpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/gui/SearchDialog.cpp)
  - `loadStarCatalogFiles()`：从配置文件读取星表列表
  - `loadSelectedFiles()`：加载选中的星表文件
  - `on_starCatalogListView_doubleClicked()`：双击打开 JSON 文件
  - 添加 `QDesktopServices` 和 `QUrl` 头文件引用

- [SearchDialog.hpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/gui/SearchDialog.hpp)
  - 添加新方法声明

#### UI 修改
- [searchDialogGui.ui](file:///f:/temp/stellarium_4A2.19/stellarium/src/gui/searchDialogGui.ui)
  - 在"取消全选"和"清屏并加载选中文件"之间添加"更新列表"按钮

### 2.4 操作说明

1. **更新列表**：点击"更新列表"按钮，重新读取配置文件并刷新列表
2. **双击打开**：双击列表中的 JSON 文件项，使用系统默认程序打开
3. **选择加载**：勾选多个星表，点击"加载选中文件"批量加载
4. **清屏加载**：点击"清屏并加载选中文件"先清除再加载

---

## 三、星空文化区域归类修复

### 3.1 问题描述
在东亚语言（简体中文、繁体中文、日本语、韩国语）环境下，印度星空文化被错误地列入"东南亚"区域，而非"南亚"。

**根因**：使用 `Qt::MatchContains` 匹配"南亚"时，会错误匹配包含这两个字的"东南亚"。

### 3.2 修复方案
将匹配方式从 `Qt::MatchContains` 改为 `Qt::MatchExactly` 精确匹配，并考虑分隔符装饰格式。

### 3.3 修改文件
- [ViewDialog.cpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/gui/ViewDialog.cpp)

```cpp
// 修改前
QList<QListWidgetItem*> foundItems = l->findItems(translatedRegion, Qt::MatchContains);

// 修改后
QString separatorText = "⸻ " + translatedRegion + " ⸻";
QList<QListWidgetItem*> foundItems = l->findItems(separatorText, Qt::MatchExactly);
```

---

## 四、星空分区配置扩展

### 4.1 新增配置参数

为星空文化的 `zodiac` 和 `lunar_system` 配置添加了两个新参数。

#### 参数说明
| 参数名 | 类型 | 说明 |
|--------|------|------|
| `start_offset` | double | 自定义起始经度偏移（直接经度值，单位：度） |
| `label_latitude` | double | 自定义标签显示纬度（单位：度，若为 NaN 则使用默认计算） |

### 4.2 修改文件
- [StelSkyCultureSkyPartition.cpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/StelSkyCultureSkyPartition.cpp)
  - 添加自定义偏移和标签纬度的读取与应用
  - 修复赤道坐标系下 `partitions` 参数处理导致的崩溃问题

- [StelSkyCultureSkyPartition.hpp](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/StelSkyCultureSkyPartition.hpp)
  - 添加 `labelLatitude` 和 `customOffset` 成员变量

### 4.3 配置示例
```json
"zodiac": {
    "name": {"native": "十二星次", "english": "Chinese Jupiter Station"},
    "partitions": [12, 2],
    "context": "Chinese Earthly Branches",
    "coordsys": "equatorial",
    "extent": 90,
    "start_offset": 0,
    "label_latitude": -30,
    "names": [...]
}
```

---

## 五、构建系统调整

### 5.1 插件禁用

为解决编译兼容性问题，临时禁用了以下插件：

| 插件 | 原因 |
|------|------|
| Scenery3d | 使用已弃用的 OpenGL 1.x 即时模式函数（glBegin, glEnd 等） |
| BoundaryMaker | 使用在 Qt6 中已移除的 `QOpenGLFunctions_1_0` 头文件 |

### 5.2 CMake 配置
```bash
cmake .. -DUSE_PLUGIN_SCENERY3D=0 -DUSE_PLUGIN_BOUNDARYMAKER=0
```

### 5.3 注意事项
- 第三方库 `QXlsxQt6` 存在编译错误，与本次修改无关
- 若需使用 Excel 读写功能，需要修复该第三方库

---

## 六、完整文件修改清单

| 文件路径 | 修改类型 | 说明 |
|----------|----------|------|
| `src/core/modules/ConstellationMgr.cpp` | 新增功能 | 实现入宿度、去极度、黄道入宿度计算 |
| `src/core/modules/ConstellationMgr.hpp` | 接口声明 | 添加新方法声明 |
| `src/core/StelObject.cpp` | 功能扩展 | 添加中国古代天文信息显示 |
| `src/gui/SearchDialog.cpp` | 功能扩展 | 实现配置文件读取、双击打开、更新列表 |
| `src/gui/SearchDialog.hpp` | 接口声明 | 添加新方法声明 |
| `src/gui/searchDialogGui.ui` | UI修改 | 添加"更新列表"按钮 |
| `src/gui/ViewDialog.cpp` | Bug修复 | 修复印度星空文化区域归类错误 |
| `src/core/StelSkyCultureSkyPartition.cpp` | 功能扩展 | 添加自定义偏移和标签纬度支持 |
| `src/core/StelSkyCultureSkyPartition.hpp` | 接口声明 | 添加新成员变量 |
| `starcatalog/_catalog_list.json` | 新建文件 | 星表列表配置文件 |

---

## 七、使用说明

### 7.1 查看天文信息
1. 启动 Stellarium
2. 切换到中文星空文化（设置 → 星空文化 → 中国）
3. 点击任意恒星
4. 信息栏将显示：
   - 所在星宿名称
   - 赤道入宿度（分数形式 + 十进制）
   - 去极度（分数形式 + 十进制）
   - 黄道入宿度（十进制 + 黄道星宿名）
   - 黄纬

### 7.2 加载星表
1. 打开搜索对话框（F3）
2. 切换到"星表"标签页
3. 点击"更新列表"读取最新配置
4. 勾选要加载的星表
5. 点击"加载选中文件"

### 7.3 编辑星表配置
1. 打开 `starcatalog/_catalog_list.json`
2. 在 `files` 数组中添加或调整 JSON 文件路径
3. 保存后在 Stellarium 中点击"更新列表"

---

## 八、技术细节

### 8.1 坐标转换
- 使用 `StelUtils::equToEcl()` 进行赤道坐标到黄道坐标的转换
- 黄赤交角通过 `SolarSystem::getEarth()->getRotObliquity()` 获取实时值
- 恒星位置使用 J2000 平春分点坐标系

### 8.2 中国古度换算
```
中国古度 = 现代度 × 365.25 / 360.0
```
一周为 365.25°，与回归年长度（365.25 天）相对应。

### 8.3 分数显示
- 小数部分转换为十二分之几
- 特殊处理：11/12 进位显示为 `X+1-1/12`
- 示例：3.5° → `3+6/12`，3.9167° → `4-1/12`

---

## 九、星座边界绘制功能设计方案

### 9.1 需求分析

#### 功能目标
在 Sky Culture Maker 中新增"星座边界"一栏，用于绘制和导出星座边界数据。

#### 输入需求
1. **星空文化文件**：用户选择已有的 `index.json` 文件，程序读取星座列表（缩写 + native 名称）
2. **坐标精度**：用户指定最低分度值（如 10'、1°），输出坐标按精度四舍五入
3. **坐标选取**：用户在界面上点选坐标点，支持连续选点绘制
4. **星座关联**：为每条边界线指定所属的两个星座（从已读取的星座列表中选择）
5. **关于选取的星座**：考虑到为每条线选星座是在太繁琐，而且相邻连线是同样的星座，我希望能像excel下拉一样自动按照之前连线的星座填充。

#### 输出格式
```json
{
  "edges_type": "own",
  "edges_epoch": "J2000.0",
  "edges": [
    "001:002 M+ 22:52:00 +34:30:00 22:52:00 +52:30:00 AND LAC",
    "002:003 P+ 22:52:00 +52:30:00 23:20:00 +52:30:00 AND CAS"
  ]
}
```

**每行格式**：`编号1:编号2 类型 赤经1 赤纬1 赤经2 赤纬2 AND 星座1 星座2`
- `M+`/`M-`：沿经度方向延伸（纬度不变）
- `P+`/`P-`：沿纬度方向延伸（经度不变）
- 编号格式：三位数字，带下划线的为未定编号（`___`）

### 9.2 现有功能分析

#### 暗星座绘制实现
- **核心类**：`ScmDraw`（[ScmDraw.hpp](file:///f:/temp/stellarium_4A2.19/stellarium/plugins/SkyCultureMaker/src/ScmDraw.hpp)）
- **坐标获取**（暗星座使用 J2000）：
  ```cpp
  // 屏幕坐标 → J2000 赤道坐标
  StelProjectorP prj = core->getProjection(drawFrame);  // FrameJ2000
  Vec3d point;
  prj->unProject(x, y, point);
  ```
- **边界绘制将使用**（当前历元）：
  ```cpp
  // 屏幕坐标 → 当前历元赤道坐标
  StelProjectorP prj = core->getProjection(StelCore::FrameEquinoxEqu);
  Vec3d point;
  prj->unProject(x, y, point);
  ```
- **数据结构**：
  ```cpp
  struct SkyPoint { Vec3d coordinate; QString star; };
  struct ConstellationLine { SkyPoint start; SkyPoint end; };
  ```
- **坐标转换导出**：
  ```cpp
  // 笛卡尔坐标 → 球面坐标（赤经/赤纬）
  StelUtils::rectToSphe(&longitude, &latitude, vec);
  RA = longitude * M_180_PI / 15.0;  // 弧度 → 小时
  DE = latitude * M_180_PI;          // 弧度 → 度
  ```

#### 星空文化 JSON 结构
```json
{
  "id": "chinese_mdn",
  "constellations": [
    {
      "id": "CON chinese_mdn P01",
      "lines": [[75097, 72607]],
      "common_name": {
        "english": "Northern Pole",
        "native": "北极",
        "pronounce": "Běi Jí"
      }
    }
  ]
}
```
- 星座 ID 格式：`CON {文化ID} {缩写}`
- 缩写示例：`P01`、`23G`
- 仅读取 `constellations`，不读取 `asterisms`

### 9.3 系统设计

#### 核心设计原则
- **不修改现有类**：创建独立的边界绘制类，确保不影响暗星座绘制功能
- **独立坐标框架**：边界绘制使用 `FrameEquinoxEqu`，暗星座使用 `FrameJ2000`
- **数据结构分离**：边界数据与星空文化数据完全分离

#### 新增文件
| 文件 | 说明 |
|------|------|
| `ScmBoundary.hpp` | 边界数据结构定义（边界点、边界线） |
| `ScmBoundary.cpp` | 边界数据处理实现 |
| `ScmBoundaryDraw.hpp` | 边界绘制类（独立于 ScmDraw） |
| `ScmBoundaryDraw.cpp` | 边界绘制实现 |
| `ScmBoundaryDialog.hpp` | 边界绘制对话框头文件 |
| `ScmBoundaryDialog.cpp` | 边界绘制对话框实现 |
| `ScmBoundaryDialog.ui` | 边界绘制对话框 UI |

#### 修改文件
| 文件 | 修改内容 |
|------|----------|
| `types/DialogID.hpp` | 新增 `BoundaryDialog` 枚举值 |
| `SkyCultureMaker.hpp` | 添加边界对话框相关成员 |
| `SkyCultureMaker.cpp` | 初始化边界对话框 |

#### 设计优势
1. **完全独立**：边界绘制和暗星座绘制使用完全独立的类，互不干扰
2. **坐标系统分离**：
   - `ScmDraw`（暗星座）：使用 `FrameJ2000`
   - `ScmBoundaryDraw`（边界）：使用 `FrameEquinoxEqu`
3. **易于维护**：修改边界功能不会影响暗星座功能
4. **易于扩展**：未来可以独立为边界绘制添加新功能

### 9.4 数据结构设计

#### ScmBoundaryPoint（边界点）
```cpp
struct ScmBoundaryPoint {
    Vec3d coordinate;      // 赤道坐标（当前历元）
    QString label;         // 点编号（如 "001"）
    double ra;             // 赤经（度）
    double dec;            // 赤纬（度）
};
```

#### ScmBoundaryEdge（边界线）
```cpp
struct ScmBoundaryEdge {
    int id;                // 边编号
    QString point1Label;   // 端点1编号
    QString point2Label;   // 端点2编号
    QString direction;     // "M+" 或 "P+"
    QString constellation1; // 星座1缩写
    QString constellation2; // 星座2缩写
    double ra1, dec1;      // 端点1赤经(度), 赤纬(度)
    double ra2, dec2;      // 端点2赤经(度), 赤纬(度)
};
```

#### ScmBoundary（边界数据集合）
```cpp
class ScmBoundary {
public:
    void loadFromJson(const QString &filePath);  // 从 JSON 读取星座列表
    void addPoint(const Vec3d &coord, double ra, double dec);  // 添加边界点
    void addEdge(int p1Idx, int p2Idx);           // 添加边界线
    void setEdgeConstellations(int edgeIdx, const QString &c1, const QString &c2);  // 设置边界关联的星座
    QString toExportString() const;               // 导出为文本格式
    void clear();                                  // 清空所有数据
    
    int getPointCount() const { return points.size(); }
    int getEdgeCount() const { return edges.size(); }
    const QList<ScmBoundaryEdge>& getEdges() const { return edges; }
    const QList<ScmBoundaryPoint>& getPoints() const { return points; }
    
private:
    QList<ScmBoundaryPoint> points;
    QList<ScmBoundaryEdge> edges;
    QMap<QString, QString> constellationMap;  // 缩写 → native 名称
    int nextPointId = 0;
    int nextEdgeId = 0;
};
```

#### ScmBoundaryDraw（边界绘制类 - 独立）
```cpp
class ScmBoundaryDraw : public QObject
{
    Q_OBJECT
public:
    ScmBoundaryDraw(ScmBoundary *boundary);
    ~ScmBoundaryDraw() override;
    
    // 坐标获取（使用 FrameEquinoxEqu）
    void handleMouseClicks(QMouseEvent *event);
    bool handleMouseMoves(int x, int y, Qt::MouseButtons b);
    void handleKeys(QKeyEvent *event);
    
    // 绘制
    void drawBoundary(StelCore *core) const;
    
    // 工具控制
    void setTool(DrawTools tool);
    void undoLastPoint();
    void resetDrawing();
    
    // 状态
    DrawTools getCurrentTool() const { return activeTool; }
    bool isDrawing() const { return isDrawingPoint; }

private:
    ScmBoundary *boundary = nullptr;
    DrawTools activeTool = DrawTools::None;
    bool isDrawingPoint = false;
    int currentPointIndex = -1;  // 当前正在绘制的点索引
    Vec3d currentFloatingPos;    // 当前浮动点位置
    
    // 独立的绘制状态
    enum class BoundaryDrawState {
        Idle,
        DrawingPoint,
        DrawingEdge
    };
    BoundaryDrawState drawState = BoundaryDrawState::Idle;
};
```

**与 ScmDraw 的区别**：
| 特性 | ScmDraw | ScmBoundaryDraw |
|------|---------|----------------|
| 坐标系 | `FrameJ2000` | `FrameEquinoxEqu` |
| 用途 | 暗星座绘制 | 边界绘制 |
| 存储 | 独立存储 | 使用 `ScmBoundary` 存储 |
| 功能 | 绘制恒星连线 | 绘制坐标点、边界线 |

### 9.5 功能流程设计

#### 5.1 初始化流程
```
1. 用户选择星空文化文件 (.json)
2. 解析 constellations 数组，提取缩写和 native 名称
3. 显示在星座选择下拉框中，信息栏中每行（每条连线）都有独立的两个星座下拉框！！不能共用一个
4. 用户设置坐标精度（度/分/秒）
```

#### 5.2 绘制流程
```
1. 用户在星图上点击选取坐标点
2. 读取座标点在当前程序设定时间的赤道坐标，注意不一定是 J2000 ！如你所见，星座边界可以指定历元，如"edges_epoch": "B1875",因此后面的座标都是B1875的座标，而不是2000
3. 显示选取点的赤经/赤纬信息
4. 支持连续选点（点1→点2→点3...）
5. 每两个相邻点组成一条边界线
6. 用户为每条线选择关联的两个星座
```

#### 5.3 导出流程
```
1. 获取所有边界点坐标
2. 按精度对坐标进行四舍五入
3. 根据坐标差判断边界方向（M+/P+）
4. 格式化坐标为 HH:MM:SS ±DD:MM:SS
5. 生成编号和边编号
6. 输出为文本格式
```

### 9.6 坐标精度处理

#### 精度选项
| 精度 | 分母 | 显示格式 | 示例 |
|------|------|----------|------|
| 10' | 60 | HH:MM:SS.s | 22:52:00.0 |
| 1° | 60 | HH:MM:SS | 22:52:00 |
| 1' | 60 | HH:MM:SS | 22:52:00 |
| 0.1° | 3600 | HH:MM:SS.ss | 22:52:00.30 |

#### 四舍五入算法
```cpp
// 将角度按指定精度四舍五入
double roundToPrecision(double value, int precisionDenominator) {
    double step = 360.0 / precisionDenominator;
    return std::round(value / step) * step;
}

// 对坐标进行精度化简
void simplifyCoordinates(ScmBoundaryEdge &edge, int precision) {
    edge.ra1 = roundToPrecision(edge.ra1, precision);
    edge.dec1 = roundToPrecision(edge.dec1, precision);
    edge.ra2 = roundToPrecision(edge.ra2, precision);
    edge.dec2 = roundToPrecision(edge.dec2, precision);
}
```

#### 方向判断
```cpp
// 判断边界是沿经度还是纬度方向
QString determineDirection(const ScmBoundaryEdge &edge) {
    double raDiff = std::abs(edge.ra1 - edge.ra2);
    double decDiff = std::abs(edge.dec1 - edge.dec2);
    
    // 化简后判断：经度差为0则沿纬度(P)，纬度差为0则沿经度(M)
    if (raDiff < epsilon) {
        return "P+";  // 经度相同，沿纬度方向
    } else if (decDiff < epsilon) {
        return "M+";  // 纬度相同，沿经度方向
    }
    // 否则需要用户手动指定或报错
    return "";
}
```

### 9.7 UI 界面设计

#### 7.1 对话框布局

```
┌─────────────────────────────────────────────────────────────────┐
│ 星座边界绘制                                                     │
├─────────────────────────────────────────────────────────────────┤
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ 文件设置                                                     │ │
│ │ [选择星空文化文件] 当前: chinese_mdn/index.json               │ │
│ │ 坐标精度: [1° ▼]                                             │ │  // 历元不用指定，按照当前程序设置的时间。
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ 绘制工具                                                     │ │
│ │ [画笔] [橡皮擦] [撤销] [清空]                                │ │
│ │ 模式: [坐标模式 ▼]                                           │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ 边界线列表                                                   │ │
│ │ ┌────┬──────┬─────────┬─────────┬──────────┬──────────┬───┐ │ │
│ │ │ #  │ 方向 │ 端点1    │ 端点2    │ 星座1    │ 星座2    │操作│ │ │
│ │ ├────┼──────┼─────────┼─────────┼──────────┼──────────┼───┤ │ │
│ │ │ 01 │ M+   │ 22:52:00│ 22:52:00│ [P01 ▼]  │ [P07 ▼]  │✏️🗑│ │ │
│ │ │    │      │ +34:30:0│ +52:30:0│          │          │   │ │ │
│ │ ├────┼──────┼─────────┼─────────┼──────────┼──────────┼───┤ │ │
│ │ │ 02 │ P+   │ 22:52:00│ 23:20:00│ [P01 ▼]  │ [P07 ▼]  │✏️🗑│ │ │
│ │ │    │      │ +52:30:0│ +52:30:0│          │          │   │ │ │
│ │ └────┴──────┴─────────┴─────────┴──────────┴──────────┴───┘ │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ 选中点信息                                                   │ │
│ │ 点 #001: 赤经 22:52:00 赤纬 +34:30:00                       │ │
│ │ 坐标: (0.8526, -0.5123, 0.1234)                             │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ 导出预览                                                     │ │
│ │ ┌─────────────────────────────────────────────────────────┐ │ │
│ │ │ "___:___ M+ 22:52:00 +34:30:00 22:52:00 +52:30:00 P01 P07"│ │ │
│ │ │ "___:___ P+ 22:52:00 +52:30:00 23:20:00 +52:30:00 P01 P07"│ │ │
│ │ └─────────────────────────────────────────────────────────┘ │ │
│ │ [复制到剪贴板] [保存为文件]                                   │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
│                    [保存]  [取消]  [导出]                        │
└─────────────────────────────────────────────────────────────────┘
```

#### 7.2 控件说明

| 控件 | 类型 | 说明 |
|------|------|------|
| 文件选择 | `QPushButton` | 选择星空文化 JSON 文件 |
| 精度选择 | `QComboBox` | 10'、1°、1'、0.1° 等选项 |
| 画笔工具 | `QToolButton` | 选取坐标点 |
| 橡皮擦 | `QToolButton` | 删除已选点 |
| 撤销 | `QToolButton` | 撤销上一步操作 |
| 边界线列表 | `QTableWidget` | 显示所有边界线及关联星座 |
| 星座选择 | `QComboBox` (在表格内) | 为每条边界线选择星座 |
| 导出预览 | `QTextEdit` | 预览输出格式 |
| 复制按钮 | `QPushButton` | 复制导出文本到剪贴板 |

### 9.8 关键实现细节

#### 8.1 坐标系统

**重要**：星座边界的坐标不一定是 J2000.0，而是应使用当前设定时间（历元）下的赤道坐标。

| 坐标系 | 枚举值 | 说明 |
|--------|--------|------|
| `FrameJ2000` | J2000.0 | 固定的 J2000 赤道坐标系 |
| `FrameEquinoxEqu` | 当前历元 | 随时间变化的赤道坐标系，包含岁差和章动 |

**示例**：现代星座边界使用 B1875 历元，因此在 B1875 时间下获取的坐标才是正确的。

**获取当前历元坐标**：
```cpp
// 使用 FrameEquinoxEqu 获取当前设定时间下的赤道坐标
StelProjectorP prj = core->getProjection(StelCore::FrameEquinoxEqu);
Vec3d point;
prj->unProject(x, y, point);
// point 即为当前时间历元下的赤道坐标
```

**参考代码**：[StelMovementMgr.cpp:1665](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/StelMovementMgr.cpp#L1665)

**星体质点位置获取**：
```cpp
// 获取星体质点在不同坐标系下的位置
Vec3d posJ2000 = star->getJ2000EquatorialPos(core);      // J2000 坐标
Vec3d posEquinox = star->getEquinoxEquatorialPos(core);  // 当前历元坐标
```

**参考代码**：[StelObject.hpp:421](file:///f:/temp/stellarium_4A2.19/stellarium/src/core/StelObject.hpp#L421)

#### 8.2 历元处理

用户需要指定目标历元，程序需支持以下功能：

1. **历元选择**：
   - 从星空文化 `index.json` 中读取 `edges_epoch` 字段
   - 或由用户手动指定

2. **时间设置**：
   - 在绘制前将 Stellarium 时间设置为目标历元
   - 这样 `FrameEquinoxEqu` 就会返回目标历元下的坐标

3. **历元格式**：
   - `B1875` → Besselian 历元 1875.0
   - `J2000` → Julian 历元 2000.0
   - 或自定义年份

**设置历元的方法**：
```cpp
// 将 Stellarium 时间设置为目标历元年份
// 例如：B1875 对应 1875 年
double jd1875 = StelUtils::getJDFromDate(QDate(1875, 1, 1));
core->setJD(jd1875);
// 之后 FrameEquinoxEqu 就会返回 1875 年的赤道坐标
```

#### 8.3 坐标格式化
```cpp
// 弧度 → 度分秒字符串
QString formatRA(double degrees, int precision) {
    int h = floor(degrees / 15.0);
    double remain = degrees - h * 15.0;
    int m = floor(remain * 60.0);
    double s = (remain * 60.0 - m) * 60.0;
    return QString("%1:%2:%3").arg(h, 2, '0').arg(m, 2, '0')
                              .arg(s, 2, '0', 'f');
}

QString formatDec(double degrees, int precision) {
    QString sign = degrees >= 0 ? "+" : "-";
    degrees = std::abs(degrees);
    int d = floor(degrees);
    double remain = degrees - d;
    int m = floor(remain * 60.0);
    double s = (remain * 60.0 - m) * 60.0;
    return QString("%1%2:%3:%4").arg(sign).arg(d, 2, '0').arg(m, 2, '0')
                               .arg(s, 2, '0', 'f');
}
```

#### 8.3 边界编号生成
```cpp
// 自动生成边界点编号
QString generatePointLabel(int index) {
    if (index < 10) return QString("00%1").arg(index);
    if (index < 100) return QString("0%1").arg(index);
    return QString::number(index);
}

// 生成边编号（基于端点编号）
QString generateEdgeLabel(const QString &p1, const QString &p2) {
    return QString("%1:%2").arg(p1, p2);
}
```

#### 8.4 数据校验
- 检查两端点是否相同（无效边界）
- 检查精度化简后是否满足 M+ 或 P+ 条件
- 检查星座缩写是否存在于已加载的列表中
- 检查边界是否自相交

### 9.9 与现有功能的关系

#### 9.9.1 核心设计原则
- **不修改任何现有类**：边界绘制功能通过全新的类实现
- **完全独立**：`ScmBoundaryDraw` 与 `ScmDraw` 完全独立，互不影响
- **接口兼容**：新类使用与现有类相同的接口签名，便于集成

#### 9.9.2 独立的绘制类对比
| 特性 | ScmDraw（现有） | ScmBoundaryDraw（新增） |
|------|----------------|------------------------|
| 坐标系 | `FrameJ2000` | `FrameEquinoxEqu` |
| 用途 | 暗星座绘制 | 边界绘制 |
| 数据存储 | 内部存储 | 使用 `ScmBoundary` |
| 功能 | 恒星连线 | 坐标边界线 |
| 生命周期 | 由 `SkyCultureMaker` 管理 | 由 `ScmBoundaryDialog` 管理 |

#### 9.9.3 复用的组件
| 组件 | 来源 | 用途 |
|------|------|------|
| `StelUtils::rectToSphe` | 坐标转换 | 笛卡尔坐标转球面坐标 |
| `StelCore::FrameEquinoxEqu` | 坐标框架 | 当前历元赤道坐标系 |
| `StelProjector` | 投影器 | 屏幕坐标与赤道坐标转换 |
| `DrawTools` 枚举 | 工具类型 | 画笔、橡皮擦等工具标识 |

#### 9.9.4 需要修改的组件
| 组件 | 修改内容 | 影响范围 |
|------|----------|----------|
| `types/DialogID.hpp` | 添加 `BoundaryDialog` 枚举 | 仅新增枚举值 |
| `SkyCultureMaker.hpp` | 添加边界对话框成员 | 仅新增成员 |
| `SkyCultureMaker.cpp` | 初始化边界对话框 | 仅新增初始化代码 |

**重要**：以上修改均为**纯新增**，不修改现有代码逻辑，确保不影响暗星座绘制功能。

#### 9.9.5 新增组件
| 组件 | 说明 |
|------|------|
| `ScmBoundary` | 边界数据模型（点、线、星座关联） |
| `ScmBoundaryDraw` | 边界绘制类（独立于 ScmDraw） |
| `ScmBoundaryDialog` | 边界绘制对话框 |
| 坐标精度处理 | 新增精度化简功能 |

### 9.10 实现计划

#### 第一阶段：基础框架
1. 创建 `ScmBoundary.hpp/cpp` 数据结构
2. 创建 `ScmBoundaryDialog.hpp/cpp/ui` 对话框框架
3. 在 `DialogID` 中添加 `BoundaryDialog` 枚举
4. 在 `SkyCultureMaker` 中集成新对话框

#### 第二阶段：核心功能
5. 实现星空文化 JSON 解析（读取星座列表）
6. 实现坐标精度处理
7. 实现边界绘制（复用 ScmDraw 逻辑）
8. 实现边界线与星座关联

#### 第三阶段：导出功能
9. 实现坐标格式化（度分秒）
10. 实现方向判断（M+/P+）
11. 实现导出文本生成
12. 添加预览和复制功能

#### 第四阶段：完善
13. 添加数据校验
14. 优化 UI 交互
15. 添加快捷键支持
16. 测试和调试

### 9.11 注意事项

1. **坐标框架**：使用 `FrameEquinoxEqu` 获取当前设定时间下的赤道坐标，而非固定的 J2000.0
2. **历元设置**：绘制前需将 Stellarium 时间设置为目标历元（如 B1875 设为 1875 年）
3. **精度处理**：必须先按精度化简，再判断方向，因为原始高精度坐标几乎不会完美对齐
4. **边界方向**：M+ 表示沿经度（经度值相同），P+ 表示沿纬度（纬度值相同）
5. **星座缩写**：使用 `index.json` 中 `id` 字段的最后一部分（如 `P01`、`23G`）
6. **历元标注**：导出时需标注 `edges_epoch`，从输入文件读取或由用户指定
7. **多段曲线**：用户可以连续选点绘制多条边界线组成曲线
8. **暗星座差异**：暗星座使用 J2000 坐标，而边界使用当前历元坐标，两者不同

---

**文档创建日期**：2026年7月28日  
**适用版本**：Stellarium 4A2.19 (Qt6 适配版)

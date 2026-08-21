/*
 * Sky Culture Maker plug-in for Stellarium
 *
 * Copyright (C) 2025 Vincent Gerlach
 * Copyright (C) 2025 Luca-Philipp Grumbach
 * Copyright (C) 2025 Fabian Hofer
 * Copyright (C) 2025 Mher Mnatsakanyan
 * Copyright (C) 2025 Richard Hofmann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScmBoundaryDraw.hpp"
#include "StelProjector.hpp"
#include "StelPainter.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelGui.hpp"
#include "StelMainView.hpp"

#include <QDebug>
#include <cmath>

namespace scm
{

ScmBoundaryDraw::ScmBoundaryDraw(ScmBoundary *boundary, QObject *parent)
	: QObject(parent)
	, boundary(boundary)
{
}

ScmBoundaryDraw::~ScmBoundaryDraw()
{
}

Vec3d ScmBoundaryDraw::screenToEquinoxEqu(int x, int y, StelCore *core)
{
	StelProjectorP prj = core->getProjection(drawFrame);
	Vec3d point;
	if (prj->unProject(x, y, point))
	{
		return point;
	}
	return Vec3d(0.0, 0.0, 0.0);
}

void ScmBoundaryDraw::drawBoundary(StelCore *core) const
{
	if (!boundary) return;

	const QList<ScmBoundaryPoint> &points = boundary->getPoints();
	const QList<ScmBoundaryEdge> &edges = boundary->getEdges();

	StelPainter painter(core->getProjection(drawFrame));

	Vec3f pointColor(1.0f, 1.0f, 0.0f);
	Vec3f edgeColor(0.0f, 1.0f, 1.0f);
	float lineAlpha = 0.8f;

	painter.setBlending(true);
	painter.setLineSmooth(true);

	const float scale = painter.getProjector()->getScreenScale();
	float pointSize = 8.0f * scale;

	// 绘制端点（简单方块）
	for (const ScmBoundaryPoint &pt : points)
	{
		Vec3f win;
		if (painter.getProjector()->project(pt.coordinate, win))
		{
			painter.setColor(pointColor.v[0], pointColor.v[1], pointColor.v[2], 1.0f);
			painter.drawRect2d(win.v[0] - pointSize/2, win.v[1] - pointSize/2, pointSize, pointSize, false);
		}
	}

	// 绘制边
	painter.setLineWidth(2.0f * scale);
	for (const ScmBoundaryEdge &edge : edges)
	{
		int p1Idx = -1, p2Idx = -1;
		for (int i = 0; i < points.size(); ++i)
		{
			if (points[i].label == edge.point1Label) p1Idx = i;
			if (points[i].label == edge.point2Label) p2Idx = i;
		}

		if (p1Idx < 0 || p2Idx < 0) continue;

		painter.setColor(edgeColor.v[0], edgeColor.v[1], edgeColor.v[2], lineAlpha);
		painter.drawGreatCircleArc(points[p1Idx].coordinate, points[p2Idx].coordinate);
	}

	// 绘制浮动线
	if (isDrawingActive && pendingPointIndex >= 0)
	{
		int p1Idx = pendingPointIndex;
		if (p1Idx >= 0 && p1Idx < points.size())
		{
			Vec3f floatColor(1.0f, 0.5f, 0.5f);
			painter.setColor(floatColor.v[0], floatColor.v[1], floatColor.v[2], 0.5f);
			painter.setLineWidth(1.0f * scale);
			painter.drawGreatCircleArc(points[p1Idx].coordinate, currentFloatingPos);
		}
	}

	painter.setLineWidth(1);
	painter.setLineSmooth(false);
}

void ScmBoundaryDraw::handleMouseClicks(QMouseEvent *event)
{
	if (!boundary) return;
	if (activeTool != DrawTools::Pen) return;

	// 左键不处理，用于拖动导航
	if (event->button() == Qt::LeftButton)
	{
		return;
	}

	// 右键选点
	if (event->button() == Qt::RightButton && event->type() == QEvent::MouseButtonPress)
	{
		StelCore *core = StelApp::getInstance().getCore();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		qreal x = event->position().x(), y = event->position().y();
#else
		qreal x = event->x(), y = event->y();
#endif

		Vec3d pos = screenToEquinoxEqu(static_cast<int>(x), static_cast<int>(y), core);

		double ra, dec;
		StelUtils::rectToSphe(&ra, &dec, pos);
		ra = ra * 180.0 / M_PI;
		dec = dec * 180.0 / M_PI;

		addPointAndEdge(pos, ra, dec);
		event->accept();
		return;
	}

	// 右键双击停止连线
	if (event->button() == Qt::RightButton && event->type() == QEvent::MouseButtonDblClick)
	{
		isDrawingActive = false;
		pendingPointIndex = -1;
		emit boundaryChanged();
		event->accept();
		return;
	}
}

bool ScmBoundaryDraw::handleMouseMoves(int x, int y, Qt::MouseButtons b)
{
	if (!boundary) return false;
	if (activeTool != DrawTools::Pen) return false;
	if (!isDrawingActive) return false;

	if (b.testFlag(Qt::LeftButton)) return false;

	StelCore *core = StelApp::getInstance().getCore();
	currentFloatingPos = screenToEquinoxEqu(x, y, core);

	double ra, dec;
	StelUtils::rectToSphe(&ra, &dec, currentFloatingPos);
	currentFloatingRA = ra * 180.0 / M_PI;
	currentFloatingDec = dec * 180.0 / M_PI;

	return true;
}

void ScmBoundaryDraw::handleKeys(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		if (isDrawingActive)
		{
			isDrawingActive = false;
			pendingPointIndex = -1;
			emit boundaryChanged();
			event->accept();
		}
	}
	else if (event->key() == Qt::Key_Z && (event->modifiers() & Qt::ControlModifier))
	{
		undoLastPoint();
		event->accept();
	}
}

void ScmBoundaryDraw::setTool(DrawTools tool)
{
	activeTool = tool;
	isDrawingActive = false;
	pendingPointIndex = -1;
}

void ScmBoundaryDraw::undoLastPoint()
{
	if (!boundary) return;
	boundary->removeLastPoint();
	isDrawingActive = false;
	pendingPointIndex = -1;
	emit boundaryChanged();
}

void ScmBoundaryDraw::resetDrawing()
{
	if (!boundary) return;
	boundary->clearAll();
	isDrawingActive = false;
	pendingPointIndex = -1;
	emit boundaryChanged();
}

void ScmBoundaryDraw::addPointAndEdge(const Vec3d &coord, double ra, double dec)
{
	if (!boundary) return;

	int newPointIdx = boundary->getPointCount();
	boundary->addPoint(coord, ra, dec);

	if (pendingPointIndex >= 0)
	{
		boundary->addEdge(pendingPointIndex, newPointIdx);
	}

	if (firstPointMode)
	{
		isDrawingActive = false;
		pendingPointIndex = -1;
		firstPointMode = false;
	}
	else
	{
		isDrawingActive = true;
		pendingPointIndex = newPointIdx;
	}

	emit pointAdded(ra, dec);
	emit boundaryChanged();
}

void ScmBoundaryDraw::removeLastDrawnItem()
{
	if (!boundary) return;
	boundary->removeLastPoint();
}

} // namespace scm

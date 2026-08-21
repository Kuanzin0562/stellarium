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

#ifndef SCMBOUNDARYDRAW_HPP
#define SCMBOUNDARYDRAW_HPP

#include "StelCore.hpp"
#include "StelUtils.hpp"
#include "types/DrawTools.hpp"
#include "ScmBoundary.hpp"

#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QString>

namespace scm
{

class ScmBoundaryDraw : public QObject
{
	Q_OBJECT

public:
	static constexpr const StelCore::FrameType drawFrame = StelCore::FrameEquinoxEqu;

	explicit ScmBoundaryDraw(ScmBoundary *boundary, QObject *parent = nullptr);
	~ScmBoundaryDraw() override;

	void drawBoundary(StelCore *core) const;

	void handleMouseClicks(QMouseEvent *event);
	bool handleMouseMoves(int x, int y, Qt::MouseButtons b);
	void handleKeys(QKeyEvent *event);

	void setTool(DrawTools tool);
	void undoLastPoint();
	void resetDrawing();

	DrawTools getCurrentTool() const { return activeTool; }
	bool isDrawing() const { return isDrawingActive; }

	void setFirstPointMode(bool enable) { firstPointMode = enable; }

signals:
	void pointAdded(double ra, double dec);
	void boundaryChanged();

private:
	ScmBoundary *boundary = nullptr;
	DrawTools activeTool = DrawTools::None;
	bool isDrawingActive = false;
	bool firstPointMode = false;

	int pendingPointIndex = -1;
	Vec3d currentFloatingPos;
	double currentFloatingRA = 0.0;
	double currentFloatingDec = 0.0;

	Vec3d screenToEquinoxEqu(int x, int y, StelCore *core);
	void addPointAndEdge(const Vec3d &coord, double ra, double dec);
	void removeLastDrawnItem();
};

} // namespace scm

#endif

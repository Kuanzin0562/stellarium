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

#ifndef SCM_BOUNDARY_HPP
#define SCM_BOUNDARY_HPP

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QJsonObject>

#include "VecMath.hpp"

namespace scm
{

struct ScmBoundaryPoint
{
	Vec3d coordinate;
	QString label;
	double ra = 0.0;
	double dec = 0.0;
};

struct ScmBoundaryEdge
{
	int id = -1;
	QString point1Label;
	QString point2Label;
	QString direction;
	QString constellation1;
	QString constellation2;
	double ra1 = 0.0, dec1 = 0.0;
	double ra2 = 0.0, dec2 = 0.0;
};

class ScmBoundary : public QObject
{
	Q_OBJECT

public:
	explicit ScmBoundary(QObject *parent = nullptr);
	~ScmBoundary() override;

	void loadConstellationsFromJson(const QString &filePath);

	void addPoint(const Vec3d &coord, double ra, double dec);
	void addEdge(int p1Idx, int p2Idx);
	void setEdgeConstellations(int edgeIdx, const QString &c1, const QString &c2);
	void setDefaultConstellations(const QString &c1, const QString &c2);
	void updateEdgeData(int edgeIdx);

	void removeLastPoint();
	void removeEdge(int edgeIdx);
	void clearAll();

	QString toExportString() const;

	int getPointCount() const { return points.size(); }
	int getEdgeCount() const { return edges.size(); }
	const QList<ScmBoundaryEdge> &getEdges() const { return edges; }
	const QList<ScmBoundaryPoint> &getPoints() const { return points; }
	const QMap<QString, QString> &getConstellationMap() const { return constellationMap; }
	QStringList getConstellationAbbreviations() const { return constellationMap.keys(); }

	void setPrecisionDenominatorRA(int denom) { precisionDenominatorRA = denom; }
	int getPrecisionDenominatorRA() const { return precisionDenominatorRA; }
	void setPrecisionDenominatorDec(int denom) { precisionDenominatorDec = denom; }
	int getPrecisionDenominatorDec() const { return precisionDenominatorDec; }

	void setPrecisionDenominator(int denom) { setPrecisionDenominatorRA(denom); setPrecisionDenominatorDec(denom); }
	int getPrecisionDenominator() const { return precisionDenominatorDec; }

	void setEdgesEpoch(const QString &epoch) { edgesEpoch = epoch; }
	QString getEdgesEpoch() const { return edgesEpoch; }

	QString formatRA(double degrees) const;
	QString formatDec(double degrees) const;

	double roundToPrecision(double value, int denominator);
	double roundRAToPrecision(double raDegrees, int denominator);
	QString determineDirection(double ra1, double dec1, double ra2, double dec2, int raDenominator, int decDenominator);

signals:
	void dataChanged();

private:
	void simplifyCoordinates(ScmBoundaryEdge &edge);
	void generatePointLabels();
	void updateEdgeDirections();

	QList<ScmBoundaryPoint> points;
	QList<ScmBoundaryEdge> edges;
	QMap<QString, QString> constellationMap;
	int precisionDenominatorRA = 144;
	int precisionDenominatorDec = 2160;
	int nextPointId = 0;
	int nextEdgeId = 0;
	QString edgesEpoch = "J2000.0";
	QString defaultConstellation1;
	QString defaultConstellation2;
};

} // namespace scm

#endif

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

#include "ScmBoundary.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>
#include <QDebug>

namespace scm
{

ScmBoundary::ScmBoundary(QObject *parent)
	: QObject(parent)
{
}

ScmBoundary::~ScmBoundary()
{
	clearAll();
}

void ScmBoundary::loadConstellationsFromJson(const QString &filePath)
{
	constellationMap.clear();

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Cannot open file:" << filePath;
		return;
	}

	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	file.close();

	if (!doc.isObject())
	{
		qWarning() << "Invalid JSON format in:" << filePath;
		return;
	}

	QJsonObject obj = doc.object();
	if (obj.contains("edges_epoch"))
	{
		edgesEpoch = obj["edges_epoch"].toString("J2000.0");
	}

	if (!obj.contains("constellations") || !obj["constellations"].isArray())
	{
		return;
	}

	QJsonArray constellations = obj["constellations"].toArray();
	for (const QJsonValue &val : constellations)
	{
		if (!val.isObject()) continue;
		QJsonObject constObj = val.toObject();

		QString id = constObj["id"].toString();
		if (!id.startsWith("CON")) continue;

		QString abbreviation = id.mid(id.lastIndexOf(' ') + 1);

		QString nativeName;
		if (constObj.contains("common_name"))
		{
			QJsonObject commonName = constObj["common_name"].toObject();
			if (commonName.contains("native"))
			{
				nativeName = commonName["native"].toString();
			}
			else if (commonName.contains("english"))
			{
				nativeName = commonName["english"].toString();
			}
		}

		if (!abbreviation.isEmpty())
		{
			constellationMap[abbreviation] = nativeName.isEmpty() ? abbreviation : nativeName;
		}
	}

	emit dataChanged();
}

void ScmBoundary::addPoint(const Vec3d &coord, double ra, double dec)
{
	ScmBoundaryPoint point;
	point.coordinate = coord;
	point.ra = ra;
	point.dec = dec;
	points.append(point);
	nextPointId++;
	generatePointLabels();
	emit dataChanged();
}

void ScmBoundary::addEdge(int p1Idx, int p2Idx)
{
	if (p1Idx < 0 || p2Idx < 0 || p1Idx >= points.size() || p2Idx >= points.size())
	{
		qWarning() << "Invalid point indices for edge:" << p1Idx << p2Idx;
		return;
	}

	ScmBoundaryEdge edge;
	edge.id = nextEdgeId++;
	edge.point1Label = points[p1Idx].label;
	edge.point2Label = points[p2Idx].label;
	edge.ra1 = points[p1Idx].ra;
	edge.dec1 = points[p1Idx].dec;
	edge.ra2 = points[p2Idx].ra;
	edge.dec2 = points[p2Idx].dec;

	if (!defaultConstellation1.isEmpty())
	{
		edge.constellation1 = defaultConstellation1;
	}
	else if (!edges.isEmpty())
	{
		edge.constellation1 = edges.last().constellation1;
	}

	if (!defaultConstellation2.isEmpty())
	{
		edge.constellation2 = defaultConstellation2;
	}
	else if (!edges.isEmpty())
	{
		edge.constellation2 = edges.last().constellation2;
	}

	qDebug() << "addEdge: raw ra1=" << edge.ra1 << "dec1=" << edge.dec1 << "ra2=" << edge.ra2 << "dec2=" << edge.dec2 << "raDenom=" << precisionDenominatorRA << "decDenom=" << precisionDenominatorDec;

	edges.append(edge);
	simplifyCoordinates(edges.last());
	updateEdgeDirections();

	qDebug() << "addEdge: simplified ra1=" << edges.last().ra1 << "dec1=" << edges.last().dec1 << "ra2=" << edges.last().ra2 << "dec2=" << edges.last().dec2;

	emit dataChanged();
}

void ScmBoundary::setDefaultConstellations(const QString &c1, const QString &c2)
{
	defaultConstellation1 = c1;
	defaultConstellation2 = c2;
}

void ScmBoundary::setEdgeConstellations(int edgeIdx, const QString &c1, const QString &c2)
{
	if (edgeIdx < 0 || edgeIdx >= edges.size())
	{
		qWarning() << "Invalid edge index:" << edgeIdx;
		return;
	}

	edges[edgeIdx].constellation1 = c1;
	edges[edgeIdx].constellation2 = c2;
	emit dataChanged();
}

void ScmBoundary::updateEdgeData(int edgeIdx)
{
	if (edgeIdx < 0 || edgeIdx >= edges.size())
	{
		qWarning() << "Invalid edge index:" << edgeIdx;
		return;
	}

	ScmBoundaryEdge &edge = edges[edgeIdx];

	int p1Idx = -1, p2Idx = -1;
	for (int i = 0; i < points.size(); ++i)
	{
		if (points[i].label == edge.point1Label) p1Idx = i;
		if (points[i].label == edge.point2Label) p2Idx = i;
	}

	if (p1Idx >= 0 && p2Idx >= 0)
	{
		edge.ra1 = points[p1Idx].ra;
		edge.dec1 = points[p1Idx].dec;
		edge.ra2 = points[p2Idx].ra;
		edge.dec2 = points[p2Idx].dec;
		simplifyCoordinates(edge);
	}

	emit dataChanged();
}

void ScmBoundary::removeLastPoint()
{
	if (points.isEmpty()) return;

	points.removeLast();
	generatePointLabels();

	for (int i = edges.size() - 1; i >= 0; --i)
	{
		bool p1Exists = false, p2Exists = false;
		for (const ScmBoundaryPoint &p : points)
		{
			if (p.label == edges[i].point1Label) p1Exists = true;
			if (p.label == edges[i].point2Label) p2Exists = true;
		}
		if (!p1Exists || !p2Exists)
		{
			edges.removeAt(i);
		}
	}

	updateEdgeDirections();
	emit dataChanged();
}

void ScmBoundary::removeEdge(int edgeIdx)
{
	if (edgeIdx < 0 || edgeIdx >= edges.size())
	{
		qWarning() << "Invalid edge index:" << edgeIdx;
		return;
	}

	edges.removeAt(edgeIdx);
	emit dataChanged();
}

void ScmBoundary::clearAll()
{
	points.clear();
	edges.clear();
	nextPointId = 0;
	nextEdgeId = 0;
	emit dataChanged();
}

QString ScmBoundary::toExportString() const
{
	QString result;
	result += "{\n";
	result += QString("  \"edges_type\": \"own\",\n");
	result += QString("  \"edges_epoch\": \"%1\",\n").arg(edgesEpoch);
	result += QString("  \"edges\": [\n");

	for (int i = 0; i < edges.size(); ++i)
	{
		const ScmBoundaryEdge &edge = edges[i];
		QString ra1Str = formatRA(edge.ra1);
		QString dec1Str = formatDec(edge.dec1);
		QString ra2Str = formatRA(edge.ra2);
		QString dec2Str = formatDec(edge.dec2);

		QString line;
		line += QString("    \"%1:%2 %3 %4 %5 %6 %7 %8 %9\"")
			.arg(edge.point1Label, edge.point2Label,
			     edge.direction,
			     ra1Str, dec1Str,
			     ra2Str, dec2Str,
			     edge.constellation1, edge.constellation2);

		if (i < edges.size() - 1)
		{
			line += ",";
		}
		line += "\n";
		result += line;
	}

	result += "  ]\n";
	result += "}\n";
	return result;
}

QString ScmBoundary::formatRA(double degrees) const
{
	double ra = fmod(degrees, 360.0);
	if (ra < 0) ra += 360.0;

	double raStepSeconds = 86400.0 / precisionDenominatorRA;
	double raStepMinutes = raStepSeconds / 60.0;

	double raHours = ra / 15.0;
	int h = static_cast<int>(floor(raHours + 1e-9));
	double remainMinutes = (raHours - h) * 60.0;
	int m = static_cast<int>(floor(remainMinutes + 1e-9));
	double s = (remainMinutes - m) * 60.0;

	int precisionLevel;
	if (raStepMinutes >= 60.0) precisionLevel = 2;
	else if (raStepMinutes >= 1.0) precisionLevel = 1;
	else if (raStepMinutes >= 1.0 / 60.0) precisionLevel = 0;
	else precisionLevel = -1;

	switch (precisionLevel)
	{
	case 2:
		return QString("%1:00:00")
			.arg(h, 2, 10, QChar('0'));
	case 1:
		return QString("%1:%2:00")
			.arg(h, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'));
	case 0:
		return QString("%1:%2:%3")
			.arg(h, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(QString::number(s, 'f', 0).rightJustified(2, QChar('0')));
	case -1:
	default:
		return QString("%1:%2:%3")
			.arg(h, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(QString::number(s, 'f', 2).rightJustified(5, QChar('0')));
	}
}

QString ScmBoundary::formatDec(double degrees) const
{
	QString sign = degrees >= 0 ? "+" : "-";
	double dec = std::abs(degrees);

	double step = 360.0 / (precisionDenominatorDec > 0 ? precisionDenominatorDec : 360);

	int d = static_cast<int>(floor(dec + 1e-9));
	double remainMinutes = dec - d;
	int m = static_cast<int>(floor(remainMinutes * 60.0 + 1e-9));
	double s = (remainMinutes * 60.0 - m) * 60.0;

	int precisionLevel;
	if (step >= 1.0) precisionLevel = 2;
	else if (step >= 1.0 / 60.0) precisionLevel = 1;
	else if (step >= 1.0 / 3600.0) precisionLevel = 0;
	else precisionLevel = -1;

	switch (precisionLevel)
	{
	case 2:
		return QString("%1%2:00:00")
			.arg(sign)
			.arg(d, 2, 10, QChar('0'));
	case 1:
		return QString("%1%2:%3:00")
			.arg(sign)
			.arg(d, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'));
	case 0:
		return QString("%1%2:%3:%4")
			.arg(sign)
			.arg(d, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(QString::number(s, 'f', 0).rightJustified(2, QChar('0')));
	case -1:
	default:
		return QString("%1%2:%3:%4")
			.arg(sign)
			.arg(d, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(QString::number(s, 'f', 2).rightJustified(5, QChar('0')));
	}
}

double ScmBoundary::roundToPrecision(double value, int denominator)
{
	if (denominator <= 0) return value;
	double step = 360.0 / denominator;
	double result = std::round(value / step) * step;
	return result;
}

double ScmBoundary::roundRAToPrecision(double raDegrees, int denominator)
{
	if (denominator <= 0) return raDegrees;

	double totalSeconds = raDegrees * 240.0;

	double timeStepSeconds = 86400.0 / denominator;

	double resultSeconds = std::round(totalSeconds / timeStepSeconds) * timeStepSeconds;

	double resultDegrees = resultSeconds / 240.0;
	resultDegrees = fmod(resultDegrees, 360.0);
	if (resultDegrees < 0) resultDegrees += 360.0;

	return resultDegrees;
}

QString ScmBoundary::determineDirection(double ra1, double dec1, double ra2, double dec2, int raDenominator, int decDenominator)
{
	double decStep = 360.0 / decDenominator;

	double raStepSeconds = 86400.0 / raDenominator;
	double raStepDegrees = raStepSeconds / 240.0;

	double raDiff = std::abs(ra1 - ra2);
	double decDiff = std::abs(dec1 - dec2);

	double raEpsilon = raStepDegrees / 2.0;
	double decEpsilon = decStep / 2.0;

	if (raDiff < raEpsilon && decDiff >= decEpsilon)
	{
		if (dec2 > dec1) return "M+";
		else return "M-";
	}
	else if (decDiff < decEpsilon && raDiff >= raEpsilon)
	{
		if (ra2 > ra1) return "P+";
		else return "P-";
	}
	else if (raDiff < raEpsilon && decDiff < decEpsilon)
	{
		return "INVALID";
	}
	else
	{
		return "AMBIGUOUS";
	}
}

void ScmBoundary::simplifyCoordinates(ScmBoundaryEdge &edge)
{
	edge.ra1 = roundRAToPrecision(edge.ra1, precisionDenominatorRA);
	edge.dec1 = roundToPrecision(edge.dec1, precisionDenominatorDec);
	edge.ra2 = roundRAToPrecision(edge.ra2, precisionDenominatorRA);
	edge.dec2 = roundToPrecision(edge.dec2, precisionDenominatorDec);

	edge.direction = determineDirection(edge.ra1, edge.dec1, edge.ra2, edge.dec2, precisionDenominatorRA, precisionDenominatorDec);
}

void ScmBoundary::generatePointLabels()
{
	for (int i = 0; i < points.size(); ++i)
	{
		int num = i + 1;
		if (num < 10)
		{
			points[i].label = QString("00%1").arg(num);
		}
		else if (num < 100)
		{
			points[i].label = QString("0%1").arg(num);
		}
		else
		{
			points[i].label = QString::number(num);
		}
	}
}

void ScmBoundary::updateEdgeDirections()
{
	for (ScmBoundaryEdge &edge : edges)
	{
		int p1Idx = -1, p2Idx = -1;
		for (int i = 0; i < points.size(); ++i)
		{
			if (points[i].label == edge.point1Label) p1Idx = i;
			if (points[i].label == edge.point2Label) p2Idx = i;
		}

		if (p1Idx >= 0 && p2Idx >= 0)
		{
			edge.ra1 = points[p1Idx].ra;
			edge.dec1 = points[p1Idx].dec;
			edge.ra2 = points[p2Idx].ra;
			edge.dec2 = points[p2Idx].dec;
			simplifyCoordinates(edge);
		}
	}
}

} // namespace scm

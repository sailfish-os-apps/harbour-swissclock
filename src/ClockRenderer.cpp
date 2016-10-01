/*
 * Copyright (C) 2014-2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   - Neither the name of Jolla Ltd nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ClockRenderer.h"
#include "ClockDebug.h"

#include <qmath.h>
#include <QSGFlatColorMaterial>

ClockRenderer::ImageNode::ImageNode(
    QQuickWindow* aWindow,
    qreal aX,
    qreal aY,
    QImage aImage)
{
    setRect(aX, aY, aImage.width(), aImage.height());
    setTexture(aWindow->createTextureFromImage(aImage));
}

ClockRenderer::ImageNode::~ImageNode()
{
    delete texture();
}

ClockRenderer::~ClockRenderer()
{
}

QMatrix4x4
ClockRenderer::secNodeMatrix(
    const QSize& aSize,
    const QTime& aTime)
{
    qreal dx = aSize.width()/2;
    qreal dy = aSize.height()/2;
    return QMatrix4x4(QTransform::fromTranslate(dx, dy).
        rotate(6.0 * (aTime.second() + aTime.msec()/1000.0) - 90).
        translate(-dx, -dy));
}

QSGNode*
ClockRenderer::geometryNode(
    QSGGeometry* aGeometry,
    const QColor& color)
{
    QSGFlatColorMaterial* m = new QSGFlatColorMaterial;
    m->setColor(color);
    QSGGeometryNode* node = new QSGGeometryNode;
    node->setGeometry(aGeometry);
    node->setMaterial(m);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

QSGGeometry*
ClockRenderer::circleGeometry(
    const QPointF& aCenter,
    qreal aRadius)
{
    const int n = 8*aRadius;
    const float x0 = aCenter.x();
    const float y0 = aCenter.y();
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),n + 2);
    QSGGeometry::Point2D* v = g->vertexDataAsPoint2D();
    g->setDrawingMode(GL_TRIANGLE_FAN);
    v[0].x = x0;
    v[0].y = y0;
    for (int i=0; i<n; i++) {
        const float theta = i*2*M_PI/n;
        v[i+1].x = x0 + aRadius*cos(theta);
        v[i+1].y = y0 + aRadius*sin(theta);
    }
    v[n+1] = v[1];
    return g;
}

QSGGeometry*
ClockRenderer::ringGeometry(
    const QPointF& aCenter,
    qreal aRadius,
    qreal aThickness)
{
    const int n = 8*aRadius;
    const float x0 = aCenter.x();
    const float y0 = aCenter.y();
    const float innerRadius = aRadius - qMin(aRadius, aThickness);
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),2*(n+1));
    QSGGeometry::Point2D* v = g->vertexDataAsPoint2D();
    g->setDrawingMode(GL_TRIANGLE_STRIP);
    for (int i=0; i<n; i++) {
        const float theta = i*2*M_PI/n;
        v[2*i].x = x0 + innerRadius*cos(theta);
        v[2*i].y = y0 + innerRadius*sin(theta);
        v[2*i+1].x = x0 + aRadius*cos(theta);
        v[2*i+1].y = y0 + aRadius*sin(theta);
    }
    v[2*n] = v[0];
    v[2*n+1] = v[1];
    return g;
}

QSGNode*
ClockRenderer::circleNode(
    const QPointF& aCenter,
    qreal aRadius,
    const QColor& aColor)
{
    return geometryNode(circleGeometry(aCenter, aRadius), aColor);
}

QSGNode*
ClockRenderer::ringNode(
    const QPointF& aCenter,
    qreal aRadius,
    qreal aThickness,
    const QColor& aColor)
{
    return geometryNode(ringGeometry(aCenter, aRadius, aThickness), aColor);
}

QSGNode*
ClockRenderer::centerDiskNode(
    QQuickWindow* aWindow,
    const QPointF& aCenter,
    qreal aRadius,
    int aCenterRadius,
    const QColor& aColor)
{
    QSGNode* disk = circleNode(aCenter, aRadius, aColor);
    disk->appendChildNode(centerNode(aWindow, aCenter, aCenterRadius));
    return disk;
}

QSGNode*
ClockRenderer::centerNode(
    QQuickWindow* aWindow,
    const QPointF& aCenter,
    int aRadius)
{
    const int r = qMax(aRadius & ~1, 2);
    const int r2 = qMax((aRadius/2) & ~1, 1);

    if (r2 == 1) {
        const int d = 2*r;
        QPointF center(0,0);
        QPixmap pixmap(d, d);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.translate(r, r);
        painter.setBrush(QBrush(Qt::white));
        painter.drawEllipse(center, r, r);
        painter.setBrush(QBrush(Qt::black));
        painter.drawEllipse(center, r2, r2);
        return new ImageNode(aWindow, aCenter.x()-r, aCenter.y()-r,
            pixmap.toImage());
    } else {
        QSGNode* node = circleNode(aCenter, r, Qt::white);
        node->appendChildNode(circleNode(aCenter, r2, Qt::black));
        return node;
    }
}

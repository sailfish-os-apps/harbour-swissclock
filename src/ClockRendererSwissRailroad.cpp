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

const QString ClockRenderer::SWISS_RAILROAD("SwissRailroad");

class SwissRailroad : public ClockRenderer
{
public:
    SwissRailroad();
    virtual void paintDialPlate(QPainter* aPainter, const QSize& aSize,
        ClockTheme* aTheme, bool aDrawBackground);
    virtual void paintHourMinHands(QPainter* aPainter, const QSize& aSize,
        const QTime& aTime, ClockTheme* aTheme);
    virtual void paintSecHand(QPainter* aPainter, const QSize& aSize,
        const QTime& aTime, ClockTheme* aTheme);
    virtual void initSecNode(QSGTransformNode* aTxNode, const QSizeF& aSize,
        QQuickWindow* aWindow, ClockTheme* aTheme);

    static void paintHand(QPainter* aPainter, const QRectF& aRect,
        qreal aAngle, const QBrush& aBrush,
        qreal aX = 0.0, qreal aY = 0.0);

    QColor iSecondHandColor;
    QBrush iSecondHandBrush;
    QBrush iBlack;
    QBrush iWhite;
};

ClockRenderer*
ClockRenderer::newSwissRailroad()
{
    return new SwissRailroad;
}

SwissRailroad::SwissRailroad() :
    ClockRenderer(SWISS_RAILROAD),
    iSecondHandColor(255, 0, 0),
    iSecondHandBrush(iSecondHandColor, Qt::SolidPattern),
    iBlack(Qt::black),
    iWhite(Qt::white)
{
}

void
SwissRailroad::paintDialPlate(
    QPainter* aPainter,
    const QSize& aSize,
    ClockTheme* aTheme,
    bool aDrawBackground)
{
    const qreal w = aSize.width();
    const qreal h = aSize.height();
    const qreal d = qMin(w, h);

    aPainter->save();
    aPainter->translate(w/2, h/2);

    // Draw the dial plate
    QPointF center(0,0);
    if (aDrawBackground) {
        aPainter->setPen(Qt::NoPen);
        aPainter->setBrush(QBrush(aTheme->iBackgroundColor));
        aPainter->drawEllipse(center, d/2, d/2);
    }

    const qreal y = d / 50;
    const qreal y1 = qMax(qreal(1), d / 158);
    const qreal x1 = d * 10 / 27;
    const qreal x2 = d * 10 / 21;
    const qreal x = (x1+x2)/2;
    QRectF hourMarkRect(x1, -y, (x2-x1), 2*y);
    QRectF minMarkRect(x, -y1, (x2-x), 2*y1);
    QBrush markBrush(aTheme->iHourMinHandColor);

    aPainter->setPen(aTheme->iHourMinHandColor);
    for (int i=0; i<60; i++) {
        if (i % 5) {
            aPainter->fillRect(minMarkRect, markBrush);
        } else {
            aPainter->fillRect(hourMarkRect, markBrush);
        }
        aPainter->rotate(6.0);
    }
    aPainter->restore();
}

void
SwissRailroad::paintHand(
    QPainter* aPainter,
    const QRectF& aRect,
    qreal aAngle,
    const QBrush& aBrush,
    qreal aX,
    qreal aY)
{
    aPainter->save();
    aPainter->translate(aX,aY);
    aPainter->rotate(aAngle);
    aPainter->fillRect(aRect, aBrush);
    aPainter->restore();
}

void
SwissRailroad::paintHourMinHands(
    QPainter* aPainter,
    const QSize& aSize,
    const QTime& aTime,
    ClockTheme* aTheme)
{
    // Draw hands
    const qreal d = qMin(aSize.width(),aSize.height());
    const qreal y = d / 50;
    const qreal x1 = d * 10 / 27;
    const qreal x2 = d * 10 / 21;
    const qreal xh1 = -(d * 10 / 74);
    const qreal xh2 = (d * 10 / 36);
    const qreal xm1 = xh1;
    const qreal xm2 = (x1+x2)/2;
    QRectF hourHandRect(xh1, -y, (xh2-xh1), 2*y);
    QRectF minHandRect(xm1, -y, (xm2-xm1), 2*y);
    QBrush handBrush(aTheme->iHourMinHandColor);
    QBrush shadowBrush1(aTheme->iHandShadowColor1);
    QBrush shadowBrush2(aTheme->iHandShadowColor2);

    qreal hourAngle = 30.0 * (aTime.hour() +
        (aTime.minute() + aTime.second()/60.0)/60.0) - 90;
    qreal minAngle= 6.0 * (aTime.minute() +
        (aTime.second() + aTime.msec()/1000.0)/60.0) - 90;

    QRectF rect(QPoint(0,0), aSize);
    aPainter->save();
    aPainter->translate(rect.center());
    aPainter->setPen(aTheme->iHourMinHandColor);
    paintHand(aPainter, hourHandRect, hourAngle, handBrush);
    paintHand(aPainter, minHandRect, minAngle, shadowBrush2, -1, -1);
    paintHand(aPainter, minHandRect, minAngle, shadowBrush2, 2, 2);
    paintHand(aPainter, minHandRect, minAngle, shadowBrush1, 1, 1);
    paintHand(aPainter, minHandRect, minAngle, handBrush);
    aPainter->restore();
}

void
SwissRailroad::paintSecHand(
    QPainter* aPainter,
    const QSize& aSize,
    const QTime& aTime,
    ClockTheme* aTheme)
{
    const qreal w = aSize.width();
    const qreal h = aSize.height();
    const qreal d = qMin(w, h);

    QPointF center(0,0);

    // Draw the second hand
    const qreal y = qMax(d / 50, qreal(5));
    const qreal x1 = d * 10 / 27;
    const qreal xh1 = -(d * 10 / 74);

    const qreal rs1 = y-1;
    const qreal rs2 = d/26;
    const qreal ds = rs1-2;
    const qreal xs1 = xh1 + d / 140;
    const qreal xs2 = x1 - d / 50 - rs2;
    QRectF secHandRect(xs1, -ds/2.0, (xs2-xs1), ds);

    aPainter->save();
    aPainter->setPen(Qt::NoPen);
    aPainter->setBrush(iSecondHandBrush);
    aPainter->translate(w/2, h/2);
    aPainter->rotate(6.0 * (aTime.second() + aTime.msec()/1000.0) - 90);
    aPainter->fillRect(secHandRect, iSecondHandBrush);
    aPainter->drawEllipse(center, rs1, rs1);
    aPainter->drawEllipse(QPointF(xs2,0), rs2, rs2);
    aPainter->setBrush(iWhite);
    aPainter->drawEllipse(center, 2, 2);
    aPainter->setBrush(iBlack);
    aPainter->drawEllipse(center, 1, 1);
    aPainter->restore();
}

void
SwissRailroad::initSecNode(
    QSGTransformNode* aTxNode,
    const QSizeF& aSize,
    QQuickWindow* aWindow,
    ClockTheme* aTheme)
{
    const qreal w = aSize.width();
    const qreal h = aSize.height();
    const qreal d = qMin(w, h);
    const qreal x0 = w/2;
    const qreal y0 = h/2;

    // Draw the second hand
    const qreal rs = qMax(d / 50, qreal(5));
    const qreal x1 = x0 + d * 10 / 27;
    const qreal xh1 = x0 - (d * 10 / 74);

    const qreal rc = qMax((int)d/200, 2);
    const qreal rs2 = d/26;
    const qreal ds = rs-3;
    const qreal xs1 = xh1 + d / 140;
    const qreal xs2 = x1 - d / 50 - rs2;

    HDEBUG("initializing" << qPrintable(id()) << "node");
    QSGGeometry* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
    QSGGeometry::Point2D* v = g->vertexDataAsPoint2D();
    g->setDrawingMode(GL_TRIANGLE_FAN);
    v[0].x = xs1;  v[0].y = y0 - ds/2.0;
    v[1].x = xs1;  v[1].y = y0 + ds/2.0;
    v[2].x = xs2;  v[2].y = v[1].y;
    v[3].x = xs2;  v[3].y = v[0].y;
    aTxNode->appendChildNode(geometryNode(g, iSecondHandColor));
    aTxNode->appendChildNode(circleNode(QPointF(xs2, y0), rs2, iSecondHandColor));
    aTxNode->parent()->appendChildNode(centerDiskNode(aWindow,
        QPointF(x0,y0), rs, rc, iSecondHandColor));
}

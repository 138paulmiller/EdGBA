/*
    Elypson/qt-collapsible-CollapsibleLayout
    (c) 2016 Michael A. Voelkel - michael.alexander.voelkel@gmail.com

    This file is part of Elypson/qt-collapsible-section  .

    Elypson/qt-collapsible-section  is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elypson/qt-collapsible-section is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elypson/qt-collapsible-section . If not, see <http://www.gnu.org/licenses/>.
*/

#include <QPropertyAnimation>

#include "collapsiblesection.h"
#include <QDebug>

CollapsibleSection::CollapsibleSection(QWidget* parent)
    : QWidget(parent)
    , animationDuration(100)
    , isExpanded(false)
{
    toggleButton = new QToolButton(this);
    headerLine = new QFrame(this);
    toggleAnimation = new QParallelAnimationGroup(this);
    contentArea = new QScrollArea(this);
    mainLayout = new QGridLayout(this);

    toggleButton->setStyleSheet("QToolButton {border: none;}");
    toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton->setArrowType(Qt::ArrowType::RightArrow);
    toggleButton->setText("");
    toggleButton->setCheckable(true);
    toggleButton->setChecked(false);

    headerLine->setFrameShape(QFrame::HLine);
    headerLine->setFrameShadow(QFrame::Sunken);
    headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // start out collapsed
    contentArea->setMaximumHeight(0);
    contentArea->setMinimumHeight(0);

    // let the entire widget grow and shrink with its content
    toggleAnimation->addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    toggleAnimation->addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    toggleAnimation->addAnimation(new QPropertyAnimation(contentArea, "maximumHeight"));

    mainLayout->setVerticalSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    int row = 0;
    mainLayout->addWidget(toggleButton, row, 0, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(headerLine, row++, 2, 1, 1);
    mainLayout->addWidget(contentArea, row, 0, 1, 3);
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    connect(toggleButton, &QToolButton::toggled, this, &CollapsibleSection::toggle);
}

void CollapsibleSection::toggle(bool expanded)
{
    toggleButton->setArrowType(expanded ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    toggleAnimation->setDirection(expanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggleAnimation->start();

    this->isExpanded = expanded;
}

void CollapsibleSection::setContentLayout(QLayout* contentLayout)
{
    delete contentArea->layout();
    contentArea->setLayout(contentLayout);

    collapsedHeight = sizeHint().height() - contentArea->maximumHeight();
    int contentHeight = contentLayout->sizeHint().height();
    for (int i = 0; i < toggleAnimation->animationCount() - 1; ++i)
    {
        QPropertyAnimation* CollapsibleLayoutAnimation = static_cast<QPropertyAnimation *>(toggleAnimation->animationAt(i));
        CollapsibleLayoutAnimation->setDuration(animationDuration);
        CollapsibleLayoutAnimation->setStartValue(collapsedHeight);
        CollapsibleLayoutAnimation->setEndValue(collapsedHeight + contentHeight);
    }

    QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation *>(toggleAnimation->animationAt(toggleAnimation->animationCount() - 1));
    contentAnimation->setDuration(animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);

    toggleAnimation->setDirection(isExpanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggleAnimation->start();
}

void CollapsibleSection::setTitle(QString title)
{
    toggleButton->setText(std::move(title));
}

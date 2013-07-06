/* Copyright (C) 2013  Marco Heisig

This file is part of Feldrand.

Feldrand is free software: you can redistribute it and/or modify it under the
terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <QtCore/QVector>
#include <QtGui/QMouseEvent>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPainterPath>
#include <QtGui/QStyleOption>
#include "AboutWindow.hpp"
#include "config.hpp"

namespace Feldrand {

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About Feldrand"));
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    QHBoxLayout* icons = new QHBoxLayout();
    QPushButton* button = new QPushButton(tr("OK"), this);

    QLabel* logo_label = new QLabel(this);
    QLabel* agpl_label = new QLabel(this);
    QLabel* lss_label = new QLabel(this);
    QLabel* text_label = new QLabel(this);
    QPixmap feldrand_logo = QPixmap(":/images/feldrand.jpeg", "JPEG");
    QPixmap agpl_logo = QPixmap(":/images/agplv3-155x51.png", "PNG");
    QPixmap lss_logo = QPixmap(":/images/lss_logo.png", "PNG");

    logo_label->setPixmap(feldrand_logo.scaled(150, 500,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));
    agpl_label->setPixmap(agpl_logo.scaled(100, 100,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
    lss_label->setPixmap(lss_logo.scaled(100, 100,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    text_label->setWordWrap(true);
    text_label->setOpenExternalLinks(true);

    text_label->setText(tr("The interactive fluid simulator."
                           "<br/><br/>For more information check the "
                           "<a href=\"http://www10.cs.fau.de/\">Chair for system simulation homepage</a>.") +
                           tr("<br/><br/>Application Version: %1"
                           "<br/>Core Library Version: %2"
                           "<br/>Visualisation Library Version: %3")
                        .arg(Feldrand::version.c_str())
                        .arg(Feldrand::core_so_version.c_str())
                        .arg(Feldrand::visualisation_so_version.c_str()) +
                        tr("<br/><br/>Feldrand is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br/>"));
    //button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setDefault(true);
    connect(button, SIGNAL(clicked()),
            this, SLOT(reject()));

    icons->addWidget(logo_label);
    icons->addSpacing(50);
    icons->addWidget(lss_label);
    icons->addWidget(agpl_label);
    vlayout->addLayout(icons);
    vlayout->addWidget(text_label);
    vlayout->addWidget(button);
}

AboutWindow::~AboutWindow()
{
}
}

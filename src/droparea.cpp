// Copyright (C) 2014 Sacha Refshauge

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 3.0 for more details.

// A copy of the GPL 3.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official GIT repository and contact information can be found at
// http://github.com/xsacha/Sachesi

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QDebug>
#include "droparea.h"

DropArea::DropArea(QDeclarativeItem *parent)
        : QDeclarativeItem(parent),
    m_accepting(true)
{
    setAcceptDrops(m_accepting);
}

void DropArea::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();
    setCursor(Qt::DragMoveCursor);
}

void DropArea::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    unsetCursor();
}

void DropArea::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    QStringList files;
    foreach (QUrl file, event->mimeData()->urls())
    {
        files += file.toString();
    }

    emit fileDrop(files);
    unsetCursor();
}

void DropArea::setAcceptingDrops(bool accepting)
{
    if (accepting == m_accepting)
                return;

    m_accepting = accepting;
    setAcceptDrops(m_accepting);
    emit acceptingDropsChanged();
}

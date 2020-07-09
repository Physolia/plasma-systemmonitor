/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright 2020 David Redondo <kde@david-redondo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PageSortModel.h"
#include "PagesModel.h"

PageSortModel::PageSortModel(QObject* parent)
    : QAbstractProxyModel(parent)
{
}

QVariant PageSortModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == PagesModel::HiddenRole) {
        return m_hiddenProxy[mapToSource(index).row()];
    }
    return QAbstractProxyModel::data(index, role);
}

bool PageSortModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid() || role != PagesModel::HiddenRole) {
        return false;
    }
    m_hiddenProxy[mapToSource(index).row()] = value.toBool();
    Q_EMIT dataChanged(index, index, {PagesModel::HiddenRole});
    return true;
}

void PageSortModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    beginResetModel();
    m_rowMapping.clear();
    m_hiddenProxy.clear();
    if (newSourceModel) {
        for (int i = 0; i < newSourceModel->rowCount(); ++i) {
            m_rowMapping.append(i);
            m_hiddenProxy.append(newSourceModel->index(i, 0).data(PagesModel::HiddenRole).toBool());
        }
    }
    QAbstractProxyModel::setSourceModel(newSourceModel);
    endResetModel();
}


int PageSortModel::rowCount (const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rowMapping.size();
}

int PageSortModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 1;
}

QModelIndex PageSortModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    } else if (m_rowMapping.size() <= row || column != 0) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex PageSortModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

QModelIndex PageSortModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }
    const int row = m_rowMapping.indexOf(sourceIndex.row());
    return row != -1 ? createIndex(row, sourceIndex.column()) : QModelIndex();
}

QModelIndex PageSortModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }
    return sourceModel()->index(m_rowMapping.at(proxyIndex.row()), proxyIndex.column());
}

void PageSortModel::move(int fromRow, int toRow)
{
    beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), fromRow < toRow ? toRow + 1 : toRow);
    m_rowMapping.move(fromRow, toRow);
    endMoveRows();
}

void PageSortModel::applyChangesToSourceModel() const
{
    auto *pagesModel = static_cast<PagesModel*>(sourceModel());
    QStringList newOrder, hiddenPages;
    for (int i = 0; i < m_rowMapping.size(); ++i) {
        QString name = pagesModel->data(pagesModel->index(m_rowMapping[i], 0), PagesModel::FileNameRole).toString();
        newOrder.append(name);
        if (m_hiddenProxy[m_rowMapping[i]]) {
            hiddenPages.append(name);
        }
    }
    pagesModel->setPageOrder(newOrder);
    pagesModel->setHiddenPages(hiddenPages);
}

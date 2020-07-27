#include "FacesModel.h"

#include <QQuickItem>
#include <QQmlListProperty>

#include <KLocalizedString>

#include <ksysguard/faces/SensorFaceController.h>

#include "PageDataObject.h"
#include "FaceLoader.h"

FacesModel::FacesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> FacesModel::roleNames() const
{
    return QAbstractListModel::roleNames().unite(
            {{IdRole, "id"}});
}

int FacesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_faceLoaders.size() + 1;
}

QVariant FacesModel::data(const QModelIndex& index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        if (index.row() == m_faceLoaders.count()) {
            return i18n("No Chart");
        } else {
            return m_faceLoaders[index.row()]->controller()->title();
        }
    case IdRole:
        if(index.row() == m_faceLoaders.count()) {
            return "";
        } else {
            return m_faceLoaders[index.row()]->dataObject()->value(QStringLiteral("face")).toString();
        }
    }
    return QVariant();
}

QQuickItem *FacesModel::faceAtIndex(int row) const
{
    return row == m_faceLoaders.count() ? nullptr : m_faceLoaders[row]->controller()->fullRepresentation();
}

PageDataObject *FacesModel::pageData() const
{
    return m_pageData;
}

void FacesModel::setPageData(PageDataObject *pageData)
{
    if (pageData == m_pageData) {
        return;
    }
    beginResetModel();
    disconnect(m_pageData, &PageDataObject::dirtyChanged, this, nullptr);
    m_faceLoaders.clear();
    m_pageData = pageData;
    Q_EMIT pageDataChanged();
    if (pageData) {
        findFaceLoaders(pageData);
        connect(m_pageData, &PageDataObject::dirtyChanged, this, [this] {
            beginResetModel();
            m_faceLoaders.clear();
            findFaceLoaders(m_pageData);
            endResetModel();
        });
    }
    endResetModel();
}

void FacesModel::findFaceLoaders(PageDataObject *pageData)
{
    if (pageData->faceLoader()) {
        m_faceLoaders.append(pageData->faceLoader());
    } else {
        for (auto child : pageData->children()) {
            findFaceLoaders(child);
        }
    }
}

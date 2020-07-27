#ifndef FACESMODEL_H
#define FACESMODEL_H

#include <QAbstractListModel>

class QQuickItem;

class FaceLoader;
class PageDataObject;

class FacesModel : public QAbstractListModel {

    Q_OBJECT
    Q_PROPERTY(PageDataObject *pageData READ pageData WRITE setPageData NOTIFY pageDataChanged)

    enum Roles {
        IdRole = Qt::UserRole
    };

public:
    FacesModel(QObject *parent = nullptr);

    Q_INVOKABLE QQuickItem *faceAtIndex(int row) const;

    PageDataObject *pageData() const;
    void setPageData(PageDataObject *pageData);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void pageDataChanged();

private:
    void findFaceLoaders(PageDataObject *pageData);

    PageDataObject *m_pageData;
    QVector<FaceLoader*> m_faceLoaders;
};

#endif

#ifndef _LC_TIMELINEWIDGET_H_
#define _LC_TIMELINEWIDGET_H_

class lcTimelineWidget : public QTreeWidget
{
	Q_OBJECT

public:
	lcTimelineWidget(QWidget* Parent);
	virtual ~lcTimelineWidget();

	void Update(bool Clear, bool UpdateItems);
	void UpdateSelection();

public slots:
	void InsertStep();
	void RemoveStep();
	void ItemSelectionChanged();
	void CustomMenuRequested(QPoint Pos);

protected slots:
	void MoveSelectionTo();

protected:
	virtual void dropEvent(QDropEvent* Event);
    virtual void mousePressEvent(QMouseEvent * event);
	void UpdateModel();

	QMap<int, QIcon> mIcons;
	QMap<lcPiece*, QTreeWidgetItem*> mItems;
	bool mIgnoreUpdates;
};

#endif // _LC_TIMELINEWIDGET_H_

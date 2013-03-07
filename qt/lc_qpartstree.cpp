#include "lc_global.h"
#include "lc_qpartstree.h"
#include "lc_application.h"
#include "lc_library.h"
#include "pieceinf.h"

static int lcQPartsTreeSortFunc(const PieceInfo* a, const PieceInfo* b, void* sortData)
{
	if (a->IsSubPiece())
	{
		if (b->IsSubPiece())
			return strcmp(a->m_strDescription, b->m_strDescription);
		else
			return 1;
	}
	else
	{
		if (b->IsSubPiece())
			return -1;
		else
			return strcmp(a->m_strDescription, b->m_strDescription);
	}

	return 0;
}

lcQPartsTree::lcQPartsTree(QWidget *parent) :
    QTreeWidget(parent)
{
	setHeaderHidden(true);
	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));

	lcPiecesLibrary* library = lcGetPiecesLibrary();

	for (int categoryIndex = 0; categoryIndex < library->mCategories.GetSize(); categoryIndex++)
	{
		QTreeWidgetItem* categoryItem = new QTreeWidgetItem(this, QStringList((const char*)library->mCategories[categoryIndex].Name));
		categoryItem->setData(0, ExpandedOnceRole, QVariant(false));
		categoryItem->setData(0, CategoryRole, QVariant(categoryIndex));
		new QTreeWidgetItem(categoryItem);
	}
}

void lcQPartsTree::itemExpanded(QTreeWidgetItem *expandedItem)
{
	QTreeWidgetItem *parent = expandedItem->parent();

	if (parent)
		return;

	if (expandedItem->data(0, ExpandedOnceRole).toBool())
		return;

	QTreeWidgetItem *child = expandedItem->child(0);
	expandedItem->removeChild(child);
	delete child;

	int categoryIndex = expandedItem->data(0, CategoryRole).toInt();

	lcPiecesLibrary* library = lcGetPiecesLibrary();
	PtrArray<PieceInfo> singleParts, groupedParts;

	library->GetCategoryEntries(categoryIndex, true, singleParts, groupedParts);

	singleParts += groupedParts;
	singleParts.Sort(lcQPartsTreeSortFunc, NULL);

	for (int partIndex = 0; partIndex < singleParts.GetSize(); partIndex++)
	{
		PieceInfo* partInfo = singleParts[partIndex];

		QTreeWidgetItem* partItem = new QTreeWidgetItem(expandedItem, QStringList(partInfo->m_strDescription));
		partItem->setData(0, PartInfoRole, qVariantFromValue((void*)partInfo));
		partItem->setToolTip(0, QString("%1 (%2)").arg(partInfo->m_strDescription, partInfo->m_strName));

		if (groupedParts.FindIndex(partInfo) != -1)
		{
			PtrArray<PieceInfo> patterns;
			library->GetPatternedPieces(partInfo, patterns);

			for (int patternIndex = 0; patternIndex < patterns.GetSize(); patternIndex++)
			{
				PieceInfo* patternedInfo = patterns[patternIndex];

				if (!library->PieceInCategory(patternedInfo, library->mCategories[categoryIndex].Keywords))
					continue;

				const char* desc = patternedInfo->m_strDescription;
				int len = strlen(partInfo->m_strDescription);

				if (!strncmp(patternedInfo->m_strDescription, partInfo->m_strDescription, len))
					desc += len;

				QTreeWidgetItem* patternedItem = new QTreeWidgetItem(partItem, QStringList(desc));
				patternedItem->setData(0, PartInfoRole, qVariantFromValue((void*)patternedInfo));
				patternedItem->setToolTip(0, QString("%1 (%2)").arg(patternedInfo->m_strDescription, patternedInfo->m_strName));
			}
		}
	}

	expandedItem->setData(0, ExpandedOnceRole, QVariant(true));
}

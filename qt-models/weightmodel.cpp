// SPDX-License-Identifier: GPL-2.0
#include "qt-models/weightmodel.h"
#include "core/subsurface-string.h"
#include "core/gettextfromc.h"
#include "core/metrics.h"
#include "core/qthelper.h"
#include "core/subsurface-qt/DiveListNotifier.h"
#include "qt-models/weightsysteminfomodel.h"

WeightModel::WeightModel(QObject *parent) : CleanerTableModel(parent),
	changed(false),
	rows(0)
{
	//enum Column {REMOVE, TYPE, WEIGHT};
	setHeaderDataStrings(QStringList() << tr("") << tr("Type") << tr("Weight"));
	connect(&diveListNotifier, &DiveListNotifier::weightsystemsReset, this, &WeightModel::weightsystemsReset);
}

weightsystem_t *WeightModel::weightSystemAt(const QModelIndex &index)
{
	return &displayed_dive.weightsystem[index.row()];
}

void WeightModel::remove(const QModelIndex &index)
{
	if (index.column() != REMOVE)
		return;
	beginRemoveRows(QModelIndex(), index.row(), index.row()); // yah, know, ugly.
	rows--;
	remove_weightsystem(&displayed_dive, index.row());
	changed = true;
	endRemoveRows();
}

void WeightModel::clear()
{
	if (rows > 0) {
		beginRemoveRows(QModelIndex(), 0, rows - 1);
		endRemoveRows();
	}
}

QVariant WeightModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= MAX_WEIGHTSYSTEMS)
		return QVariant();

	weightsystem_t *ws = &displayed_dive.weightsystem[index.row()];

	switch (role) {
	case Qt::FontRole:
		return defaultModelFont();
	case Qt::TextAlignmentRole:
		return Qt::AlignCenter;
	case Qt::DisplayRole:
	case Qt::EditRole:
		switch (index.column()) {
		case TYPE:
			return gettextFromC::tr(ws->description);
		case WEIGHT:
			return get_weight_string(ws->weight, true);
		}
		break;
	case Qt::DecorationRole:
		if (index.column() == REMOVE)
			return trashIcon();
	case Qt::SizeHintRole:
		if (index.column() == REMOVE)
			return trashIcon().size();
	case Qt::ToolTipRole:
		if (index.column() == REMOVE)
			return tr("Clicking here will remove this weight system.");
	}
	return QVariant();
}

// this is our magic 'pass data in' function that allows the delegate to get
// the data here without silly unit conversions;
// so we only implement the two columns we care about
void WeightModel::passInData(const QModelIndex &index, const QVariant &value)
{
	weightsystem_t *ws = &displayed_dive.weightsystem[index.row()];
	if (index.column() == WEIGHT) {
		if (ws->weight.grams != value.toInt()) {
			ws->weight.grams = value.toInt();
			dataChanged(index, index);
		}
	}
}


bool WeightModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	QString vString = value.toString();
	weightsystem_t *ws = &displayed_dive.weightsystem[index.row()];
	switch (index.column()) {
	case TYPE:
		if (!value.isNull()) {
			//TODO: C-function weight_system_set_description ?
			if (!ws->description || gettextFromC::tr(ws->description) != vString) {
				// loop over translations to see if one matches
				int i = -1;
				while (ws_info[++i].name && i < MAX_WS_INFO) {
					if (gettextFromC::tr(ws_info[i].name) == vString) {
						ws->description = copy_string(ws_info[i].name);
						break;
					}
				}
				if (ws_info[i].name == NULL) // didn't find a match
					ws->description = copy_qstring(vString);
				changed = true;
			}
		}
		break;
	case WEIGHT:
		if (CHANGED()) {
			ws->weight = string_to_weight(qPrintable(vString));
			// now update the ws_info
			changed = true;
			WSInfoModel *wsim = WSInfoModel::instance();
			QModelIndexList matches = wsim->match(wsim->index(0, 0), Qt::DisplayRole, gettextFromC::tr(ws->description));
			if (!matches.isEmpty())
				wsim->setData(wsim->index(matches.first().row(), WSInfoModel::GR), ws->weight.grams);
		}
		break;
	}
	dataChanged(index, index);
	return true;
}

Qt::ItemFlags WeightModel::flags(const QModelIndex &index) const
{
	if (index.column() == REMOVE)
		return Qt::ItemIsEnabled;
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

int WeightModel::rowCount(const QModelIndex&) const
{
	return rows;
}

void WeightModel::add()
{
	if (rows >= MAX_WEIGHTSYSTEMS)
		return;

	int row = rows;
	beginInsertRows(QModelIndex(), row, row);
	rows++;
	changed = true;
	endInsertRows();
}

void WeightModel::updateDive()
{
	clear();
	rows = 0;
	for (int i = 0; i < MAX_WEIGHTSYSTEMS; i++) {
		if (!weightsystem_none(&displayed_dive.weightsystem[i])) {
			rows = i + 1;
		}
	}
	if (rows > 0) {
		beginInsertRows(QModelIndex(), 0, rows - 1);
		endInsertRows();
	}
}

void WeightModel::weightsystemsReset(const QVector<dive *> &dives)
{
	// This model only concerns the currently displayed dive. If this is not among the
	// dives that had their cylinders reset, exit.
	if (!current_dive || std::find(dives.begin(), dives.end(), current_dive) == dives.end())
		return;

	// Copy the cylinders from the current dive to the displayed dive.
	copy_weights(current_dive, &displayed_dive);

	// And update the model..
	updateDive();
}

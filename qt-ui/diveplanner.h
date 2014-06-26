#ifndef DIVEPLANNER_H
#define DIVEPLANNER_H

#include <QGraphicsPathItem>
#include <QAbstractTableModel>

#include "dive.h"

class QListView;
class QModelIndex;

class DivePlannerPointsModel : public QAbstractTableModel {
	Q_OBJECT
public:
	static DivePlannerPointsModel *instance();
	enum Sections {
		REMOVE,
		DEPTH,
		DURATION,
		RUNTIME,
		GAS,
		CCSETPOINT,
		COLUMNS
	};
	enum Mode {
		NOTHING,
		PLAN,
		ADD
	};
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	void removeSelectedPoints(const QVector<int> &rows);
	void setPlanMode(Mode mode);
	bool isPlanner();
	void createSimpleDive();
	void clear();
	Mode currentMode() const;
	bool setRecalc(bool recalc);
	bool recalcQ();
	void tanksUpdated();
	void rememberTanks();
	bool tankInUse(struct gasmix gasmix);
	void copyCylinders(struct dive *d);
	void setupCylinders();
	/**
	 * @return the row number.
	 */
	void editStop(int row, divedatapoint newData);
	divedatapoint at(int row);
	int size();
	struct diveplan &getDiveplan();
	QStringList &getGasList();
	QVector<QPair<int, int> > collectGases(dive *d);
	int lastEnteredPoint();
	void removeDeco();
	static bool addingDeco;

public
slots:
	int addStop(int millimeters = 0, int seconds = 0, struct gasmix *gas = 0, int ccpoint = 0, bool entered = true);
	void addCylinder_clicked();
	void setGFHigh(const int gfhigh);
	void setGFLow(const int ghflow);
	void setSurfacePressure(int pressure);
	void setBottomSac(int sac);
	void setDecoSac(int sac);
	void setStartTime(const QTime &t);
	void setLastStop6m(bool value);
	void setDropStoneMode(bool value);
	void setVerbatim(bool value);
	void setDisplayRuntime(bool value);
	void setDisplayDuration(bool value);
	void setDisplayTransitions(bool value);
	void createPlan();
	void remove(const QModelIndex &index);
	void cancelPlan();
	void createTemporaryPlan();
	void deleteTemporaryPlan();
	void loadFromDive(dive *d);
	void restoreBackupDive();
	void emitDataChanged();

signals:
	void planCreated();
	void planCanceled();
	void cylinderModelEdited();

private:
	explicit DivePlannerPointsModel(QObject *parent = 0);
	bool addGas(struct gasmix mix);
	struct diveplan diveplan;
	Mode mode;
	bool recalc;
	QVector<divedatapoint> divepoints;
	struct dive *tempDive;
	struct dive backupDive;
	void deleteTemporaryPlan(struct divedatapoint *dp);
	QVector<sample> backupSamples; // For editing added dives.
	struct dive *stagingDive;
	QVector<QPair<int, int> > oldGases;
	bool drop_stone_mode;
};

class DiveHandler : public QObject, public QGraphicsEllipseItem {
	Q_OBJECT
public:
	DiveHandler();

protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
signals:
	void moved();
private:
	int parentIndex();
public
slots:
	void selfRemove();
	void changeGas();
};

#include "ui_diveplanner.h"

class DivePlannerWidget : public QWidget {
	Q_OBJECT
public:
	explicit DivePlannerWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

public
slots:
	void settingsChanged();
	void atmPressureChanged(const int pressure);
	void heightChanged(const int height);
	void bottomSacChanged(const int bottomSac);
	void decoSacChanged(const int decosac);
	void printDecoPlan();

private:
	Ui::DivePlanner ui;
};

#include "ui_plannerSettings.h"

class PlannerSettingsWidget : public QWidget {
	Q_OBJECT
public:
	explicit PlannerSettingsWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

public
slots:
	void settingsChanged();
	void atmPressureChanged(const QString &pressure);
	void bottomSacChanged(const int bottomSac);
	void decoSacChanged(const int decosac);
	void printDecoPlan();
	void setAscRate75(int rate);
	void setAscRate50(int rate);
	void setAscRateStops(int rate);
	void setAscRateLast6m(int rate);
	void setDescRate(int rate);
	void setBottomPo2(double po2);
	void setDecoPo2(double po2);

private:
	Ui::plannerSettingsWidget ui;
};

QString dpGasToStr(const divedatapoint &p);

#endif // DIVEPLANNER_H

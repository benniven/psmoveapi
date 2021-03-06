#ifndef MOVEGRAPH_H
#define MOVEGRAPH_H

#include <QtGui>

#include "psmove.h"
#include "psmove_filter.h"
#include "psmove_calibration.h"

#define MAX_READINGS 1050

class MoveGraph : public QWidget {
    Q_OBJECT

    public:
        MoveGraph();

    protected:
        virtual void paintEvent(QPaintEvent *event);

    public slots:
        void readSensors();
        void setAlpha(int);

    private:
        PSMove *move;
        PSMoveFilter *filter;
        PSMoveCalibration *calibration;

        QStaticText labelPositive;
        QStaticText labelNegative;

        float readings[MAX_READINGS][3];
        int offset;
};

#endif

#ifndef GIMAGEMULTI_H
#define GIMAGEMULTI_H

#include "gimage.h"
#include "modelimagepair.h"

#include <QObject>

class GImageMulti : public GImage
{
    Q_OBJECT
public:
    explicit GImageMulti(ModelImagePair *modelImagePairIn, QWidget *parent);

public slots:

    void toggleAB();

    void setCurrentImage(int row);
    void updateDisplay();

    void dispFlow();

    void goFirst();
    void goPrevious();
    void goNext();
    void goLast();
    void updateButtonState();

private:
    bool abSelector=true;
    bool imSelector=false;

    ModelImagePair * modelImagePair;
};

#endif // GIMAGEMULTI_H

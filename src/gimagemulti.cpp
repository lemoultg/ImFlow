#include "gimagemulti.h"
#include "gimage.h"
#include "ui_gimage.h"

#include <QDebug>

GImageMulti::GImageMulti(ModelImagePair * modelImagePairIn, QWidget * parent) :
    GImage(parent)
{
    modelImagePair=modelImagePairIn;

    //Build menu for selecting what is displayed in field mode
    QActionGroup * actionDisp = new QActionGroup(this);
    actionDisp->addAction(ui->actionDispMag);
    actionDisp->addAction(ui->actionDispU);
    actionDisp->addAction(ui->actionDispV);
    ui->actionDispMag->setChecked(true);
    connect(actionDisp,&QActionGroup::triggered, this, &GImageMulti::dispFlow);
    ui->displayFlow->addActions(actionDisp->actions());


    connect(ui->buttonFirst,SIGNAL(clicked(bool)),this,SLOT(goFirst()));
    connect(ui->buttonPrevious,SIGNAL(clicked(bool)),this,SLOT(goPrevious()));
    connect(ui->buttonNext,SIGNAL(clicked(bool)),this,SLOT(goNext()));
    connect(ui->buttonLast,SIGNAL(clicked(bool)),this,SLOT(goLast()));

    connect(modelImagePair,SIGNAL(selectedItemChanged(int)),this,SLOT(setCurrentImage(int)));

    connect(ui->toggleAB,SIGNAL(clicked(bool)),this,SLOT(toggleAB()));

    connect(ui->displayImAB,SIGNAL(clicked(bool)),this,SLOT(updateDisplay()));
    connect(ui->displayFlow,&QToolButton::clicked,this,&GImageMulti::updateDisplay);

    updateButtonState();
}

void GImageMulti::toggleAB()
{
    abSelector=!abSelector;
    if (abSelector)
        ui->toggleAB->setIcon(QIcon(":icons/toggleAB_A"));
    else
        ui->toggleAB->setIcon(QIcon(":icons/toggleAB_B"));
    setCurrentImage(modelImagePair->getSelectedItem());
}

void GImageMulti::dispFlow()
{
    ui->displayFlow->setChecked(true);
    updateDisplay();
}

void GImageMulti::updateDisplay()
{
    setCurrentImage(modelImagePair->getSelectedItem());
}

void GImageMulti::setCurrentImage(int row)
{
    if (row<0 || row>modelImagePair->getSize()-1)
        return;

    ImPairItem * item = modelImagePair->getItem(row);
    if (ui->displayFlow->isChecked()) {
        if (ui->actionDispMag->isChecked()) {
            Mat mag;
            if (item->imFlowU.empty())
                mag=Mat();
            else
                magnitude(item->imFlowU,item->imFlowV,mag);
            setImage(mag);
        }
        else if (ui->actionDispU->isChecked())
            setImage(item->imFlowU);
        else if (ui->actionDispV->isChecked())
            setImage(item->imFlowV);
    }
    else if (ui->displayImAB->isChecked()){
        if (abSelector)
            setImage(item->imAref);
        else
            setImage(item->imBref);
    }
    updateButtonState();
}

void GImageMulti::goFirst()
{
    modelImagePair->setActivatedItem(0);
}

void GImageMulti::goPrevious()
{
    int row = modelImagePair->getSelectedItem();
    modelImagePair->setActivatedItem(row-1);
}

void GImageMulti::goNext()
{
    int row = modelImagePair->getSelectedItem();
    modelImagePair->setActivatedItem(row+1);
}

void GImageMulti::goLast()
{
    modelImagePair->setActivatedItem(modelImagePair->getSize()-1);
}

void  GImageMulti::updateButtonState()
{
    int selectedRow = modelImagePair->getSelectedItem();
    int nRows = modelImagePair->getSize();

    ui->buttonFirst->setEnabled(nRows!=0 && selectedRow>0);
    ui->buttonPrevious->setEnabled(nRows!=0 && selectedRow>0);
    ui->buttonNext->setEnabled(nRows!=0 && selectedRow<nRows-1);
    ui->buttonLast->setEnabled(nRows!=0 && selectedRow<nRows-1);
}



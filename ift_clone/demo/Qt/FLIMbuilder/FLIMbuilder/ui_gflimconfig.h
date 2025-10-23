/********************************************************************************
** Form generated from reading UI file 'gflimconfig.ui'
**
** Created by: Qt User Interface Compiler version 6.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GFLIMCONFIG_H
#define UI_GFLIMCONFIG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_gflimconfig
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QSpinBox *sb_initialSeedsDISF;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QSpinBox *sb_superpixel_number;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *gflimconfig)
    {
        if (gflimconfig->objectName().isEmpty())
            gflimconfig->setObjectName(QString::fromUtf8("gflimconfig"));
        gflimconfig->resize(342, 157);
        QFont font;
        font.setPointSize(10);
        gflimconfig->setFont(font);
        verticalLayout = new QVBoxLayout(gflimconfig);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(gflimconfig);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        sb_initialSeedsDISF = new QSpinBox(gflimconfig);
        sb_initialSeedsDISF->setObjectName(QString::fromUtf8("sb_initialSeedsDISF"));

        horizontalLayout->addWidget(sb_initialSeedsDISF);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(gflimconfig);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        sb_superpixel_number = new QSpinBox(gflimconfig);
        sb_superpixel_number->setObjectName(QString::fromUtf8("sb_superpixel_number"));

        horizontalLayout_2->addWidget(sb_superpixel_number);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(gflimconfig);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(gflimconfig);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, gflimconfig, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, gflimconfig, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(gflimconfig);
    } // setupUi

    void retranslateUi(QDialog *gflimconfig)
    {
        gflimconfig->setWindowTitle(QCoreApplication::translate("gflimconfig", "GFLIM Configuration", nullptr));
        label_2->setText(QCoreApplication::translate("gflimconfig", "Initial seeds (DISF)", nullptr));
        label_3->setText(QCoreApplication::translate("gflimconfig", "Superpixels", nullptr));
    } // retranslateUi

};

namespace Ui {
    class gflimconfig: public Ui_gflimconfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GFLIMCONFIG_H

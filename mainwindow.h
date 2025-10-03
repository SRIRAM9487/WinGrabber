#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "MainWIndowWidgets.h"
#include "filemeta.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

    // Main Layout
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    // Search Bar Layout
    QHBoxLayout *searchBarLayout;
    QPushButton *searchBtn;
    QLineEdit *searchInput;

    // Table
    QTableWidget *resultTable;

    // Extracted Files
    std::vector<file_meta> files;

    // Debug
    QDebug *deb;

    // Data Base
    QSqlDatabase db ;

    // Methods
    void resizeEvent(QResizeEvent *resize);
    void newSearch();
    void populateTable();
    void updateResultTable();
    std::vector<file_meta> scanDirectory(const QString &root_path);

};
#endif // MAINWINDOW_H

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Win Grabber");

    for(const QFileInfo &drive : QDir::drives()){
        std::vector<file_meta> drive_files = scanDirectory(drive.absoluteFilePath());
        files.insert(files.end(),drive_files.begin(),drive_files.end());
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("file_meta.db");

    if (!db.open()) {
        std::cerr<<"Database connection terminated "<<std::endl;
        std::cerr<<"Error : "<<db.lastError().text().toStdString()<<std::endl;
    }else{
        std::cout<<"Database connection established"<<std::endl;
    }

    QSqlQuery createQuery(db);
    QSqlQuery insertQuery(db);

    //CREATE TABLE AND INDEX
    createQuery.exec("CREATE TABLE IF NOT EXISTS filesMeta("
                     "name TEXT,"
                     "path TEXT,"
                     "size double"
                     ")");

    createQuery.exec("CREATE INDEX IF NOT EXISTS file_name_index"
                     "ON filesMeta(name)");
    createQuery.exec("CREATE INDEX IF NOT EXISTS file_path_index"
                     "ON filesMeta(path)");

    // Insert Data
    insertQuery.prepare("INSERT OR REPLACE INTO filesMeta(name, path, size) VALUES(?, ?, ?)");

    db.transaction();

    for (const auto &file : files) {
        insertQuery.bindValue(0, file.name);
        insertQuery.bindValue(1, file.path);
        insertQuery.bindValue(2, file.size);
        insertQuery.exec();
        insertQuery.bindValue(0, QVariant());
        insertQuery.bindValue(1, QVariant());
        insertQuery.bindValue(2, QVariant());
    }

    db.commit();

    // Central Widget
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);

    // Search Layout
    searchBarLayout = new QHBoxLayout();

    // Search Input
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Search Here");
    searchBarLayout->addWidget(searchInput);

    // Search Button
    searchBtn = new QPushButton("Search");
    searchBarLayout->addWidget(searchBtn);

    // Add search bar layout to main layout
    mainLayout->addLayout(searchBarLayout);

    // Table
    resultTable = new QTableWidget();

    resultTable->setColumnCount(4);
    resultTable->setRowCount(50);
    resultTable->setHorizontalHeaderLabels({"Name", "Size", "Path","Action"});
    resultTable->setSortingEnabled(true);
    resultTable->setColumnWidth(0,500);
    mainLayout->addWidget(resultTable);

    connect(searchInput,&QLineEdit::textChanged,this,&MainWindow::updateResultTable);

}

MainWindow::~MainWindow() {}

void MainWindow::resizeEvent(QResizeEvent *resize){

    int totalWidth = resultTable->viewport()-> width();
    int totalHeight = resultTable->viewport()-> height();

    if (totalWidth <500 && totalHeight<500) {
        resultTable -> setColumnWidth(0,totalWidth*0.2);
        resultTable -> setColumnWidth(1,totalWidth*0.1);
        resultTable -> setColumnWidth(2,totalWidth*0.4);
        resultTable -> setColumnWidth(3,totalWidth*0.3);

    } else {
        resultTable -> setColumnWidth(0,totalWidth*0.2);
        resultTable -> setColumnWidth(1,totalWidth*0.1);
        resultTable -> setColumnWidth(2,totalWidth*0.6);
        resultTable -> setColumnWidth(3,totalWidth*0.1);
    }


    QMainWindow::resizeEvent(resize);
}

void MainWindow::populateTable(){
    for(int i = 0 ; i <50;i++){
        file_meta file = files[i];

        resultTable->setItem(i,0,new QTableWidgetItem(file.name));
        resultTable->setItem(i,1,new QTableWidgetItem(file.size));
        resultTable->setItem(i,2,new QTableWidgetItem(file.path));

        // Action Column
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionWidgetLayout=new QHBoxLayout(actionWidget);
        QPushButton *openBtn=new QPushButton("Open");
        QPushButton *copyBtn=new QPushButton("Copy");
        actionWidgetLayout->addWidget(openBtn);
        actionWidgetLayout->addWidget(copyBtn);

        resultTable->setCellWidget(i,3,actionWidget);

        connect(openBtn,&QPushButton::clicked,this,[file](){
            QString folderPath = QFileInfo(file.path).absolutePath();
            QProcess::startDetached("explorer",QStringList()<<QDir::toNativeSeparators(folderPath));
        });

        connect(copyBtn,&QPushButton::clicked,this,[file]{
            QApplication::clipboard()->setText(file.path);
        });

    }

}

void MainWindow::updateResultTable(){
    newSearch();
};

void MainWindow::newSearch() {
    QString searchText = searchInput->text();

    resultTable->setRowCount(0);

    if (searchText.isEmpty()) {
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT name, size, path FROM filesMeta "
                  "WHERE name LIKE :search OR path LIKE :search "
                  "LIMIT 100");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        std::cerr << "Search query failed: "
                  << query.lastError().text().toStdString() << std::endl;
        return;
    }

    int row = 0;
    while (query.next()) {
        resultTable->insertRow(row);

        QString name = query.value(0).toString();
        QString size = query.value(1).toString();
        QString path = query.value(2).toString();

        resultTable->setItem(row, 0, new QTableWidgetItem(name));
        resultTable->setItem(row, 1, new QTableWidgetItem(size));
        resultTable->setItem(row, 2, new QTableWidgetItem(path));

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        QPushButton *openBtn = new QPushButton("Open");
        QPushButton *copyBtn = new QPushButton("Copy");
        actionLayout->addWidget(openBtn);
        actionLayout->addWidget(copyBtn);
        actionLayout->setContentsMargins(0, 0, 0, 0); // neat look
        resultTable->setCellWidget(row, 3, actionWidget);

        connect(openBtn, &QPushButton::clicked, this, [path]() {
            QString folderPath = QFileInfo(path).absolutePath();
            QProcess::startDetached("explorer", QStringList() << QDir::toNativeSeparators(folderPath));
        });

        connect(copyBtn, &QPushButton::clicked, this, [path]() {
            QApplication::clipboard()->setText(path);
        });

        row++;
    }
}


std::vector<file_meta> MainWindow::scanDirectory(const QString &root_path){
    QDirIterator it(root_path,QDir::Files,QDirIterator::Subdirectories);

    QSqlQuery query(db);

    if(query.exec("SELECT COUNT(*) FROM filesMeta")) {
        if(query.next()&&  query.value(0).toInt() == files.size()) {
            std::cout<<"FILES IN DB AND FILES IN DIRECTORY ARE SAME"<<std::endl;
            return files;
        }
    } else {
        qDebug() << "Query failed:" << query.lastError().text();
    }

    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        file_meta entry;
        entry.name = info.fileName();
        entry.path = info.absoluteFilePath();
        entry.size = info.size();
        files.push_back(entry);
    }
    std::cout<<"TOTAL FILES : "<<files.size()<<std::endl;
    return files;
}

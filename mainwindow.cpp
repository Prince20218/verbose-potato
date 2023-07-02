#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlQuery>
#include <QScrollBar>
#include <QPair>
#include <QHeaderView>
#include <QDir>

#include "Dialogs/dialogdbsettings.h"
#include "Dialogs/dialogaddgoods.h"
#include "Dialogs/dialogaddbrand.h"
#include "Dialogs/dialogaboutprogram.h"
#include "Dialogs/dialogcalculateprofit.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString pathToTranslation;
#ifndef Q_OS_MAC
    pathToTranslation = "QtLanguage_ru.qm";
#endif

#ifdef Q_OS_MAC
    // использую цикл для нахождения пути каталога build. В разных системах путь
    // currentPath прописывается разный. На MacOs помимо папки build в пути
    // присутствуют дальнейшие директории - /*.app/Contents/MacOS
    // с помощью цикла ниже обрезается ненужная часть пути
    pathToTranslation = QDir::currentPath();
    pathToTranslation += "/../../../";
    qDebug() << pathToTranslation;
#endif

    translator.load("QtLanguage_ru.qm", pathToTranslation) ?
        qDebug() << "File with translation loaded" :
        qDebug() << "File with translation not loaded. Check path to file";

    qApp->installTranslator(&translator) ?
        qDebug() << "Translator installed" :
        qDebug() << "Translator not installed";

    if(QSqlDatabase::isDriverAvailable("QPSQL"))
        qDebug() << "QPSQL found!";
    else
    {
        qDebug() << "EROR! QPSQL not found!";
        return;
    }
    ui->retranslateUi(this);

    // Названия столбцов в комбобоксах
    columnsSpendsFilter = QStringList({tr("- Not selected -"), tr("spends_id"), tr("date_of_creating_act"),
                                        tr("spends_of_storage"), tr("spends_of_ads"),
                                        tr("delivery_spends"), tr("spends_of_purchase")});
    columnsGoodsFilter = QStringList({tr("- Not selected -"), tr("goods_id"), tr("goods_name"),
                                       tr("seller_article"), tr("brand_name")});
    columnsBrandsFilter = QStringList({tr("- Not selected -"), tr("brand_id"), tr("brand_name")});
    // Названия столбцов в комбобоксах на английском без перевода (необх, для запросов в БД)
    columnsSpendsFilter_EN = QStringList({"- Not selected -", "spends_id", "date_of_creating_act",
                                        "spends_of_storage", "spends_of_ads",
                                        "delivery_spends", "spends_of_purchase"});
    columnsGoodsFilter_EN = QStringList({"- Not selected -", "goods_id", "goods_name",
                                       "seller_article", "brand_name"});
    columnsBrandsFilter_EN = QStringList({"- Not selected -", "brand_id", "brand_name"});

    ui->pushButtonCreateAddDialog->hide();
    ui->pushButtonDeleteRecord->hide();

    // Установление даты по умолчаниию и ограничение по максимальной дате
    // (по текущему дню)
    ui->dateEditNewOrder->setDate(QDate::currentDate());
    ui->dateEditNewOrder->setMaximumDate(QDate::currentDate());
    ui->dateEditDelivery->setDate(QDate::currentDate());
    ui->dateEditDelivery->setMaximumDate(QDate::currentDate());
    ui->dateEditSupply->setDate(QDate::currentDate());
    ui->dateEditSupply->setDate(QDate::currentDate());
    // Устанавливаем валидатор на ввод supply_id
    QRegularExpression expr("[0-9]{1,18}");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(expr);
    ui->lineEditSupplyID->setValidator(validator);

    // установка выделения всей строки по нажатии на ячейку
    ui->tableViewOthersReadEdit->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewOthersRead->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewGoodsSupplies->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewSupplies->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewOrders->setSelectionBehavior(QAbstractItemView::SelectRows);

    // установка режима выделения строк (сейчас можно выделять несколько с помощью Shift)
    ui->tableViewOthersReadEdit->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->tableViewOthersRead->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->tableViewGoodsSupplies->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->tableViewSupplies->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->tableViewOrders->setSelectionMode(QAbstractItemView::ContiguousSelection);
    //setStyleSheet("QTableView::item:hover { color:rgb(0,0,255) }");

    // Установление столбцов таблиц по размеру tableView
    ui->tableViewOrders->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewGoodsSupplies->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewSupplies->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewOthersReadEdit->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewOthersRead->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    readSettings(); // чтение настроек, записанных после предыдущего использования

    setConnects(); // Подключение коннектов
}

MainWindow::~MainWindow()
{
    // освобождение паммяти
    if(brandsModel != nullptr)
    {
        delete brandsModel;
        brandsModel = nullptr;
    }
    if(citiesModel != nullptr)
    {
        delete citiesModel;
        citiesModel = nullptr;
    }
    if(countriesModel != nullptr)
    {
        delete countriesModel;
        countriesModel = nullptr;
    }
    if(goodsModel != nullptr)
    {
        delete goodsModel;
        goodsModel = nullptr;
    }
    if(goodsSuppliesModel != nullptr)
    {
        delete goodsSuppliesModel;
        goodsSuppliesModel = nullptr;
    }
    if(suppliesModel != nullptr)
    {
        delete suppliesModel;
        suppliesModel = nullptr;
    }
    if(ordersModel != nullptr)
    {
        delete ordersModel;
        ordersModel = nullptr;
    }
    if(spendsModel != nullptr)
    {
        delete spendsModel;
        spendsModel = nullptr;
    }

    db.close();

    QSqlDatabase::removeDatabase("qt_sql_default_connection");
    writeSettings();
    delete ui;
}


void MainWindow::connectDB()
{
    resetAllModels(); // обнуляем все модели

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setConnectOptions(QString("requiressl=%1;connect_timeout=%2").arg(DialogDBSettings::dbSettings.usingSSL)
                                                                    .arg(DialogDBSettings::dbSettings.connectTimeout));
    db.setHostName(DialogDBSettings::dbSettings.hostname);
    db.setPort(DialogDBSettings::dbSettings.port);
    db.setDatabaseName(DialogDBSettings::dbSettings.dbname);
    db.setUserName(DialogDBSettings::dbSettings.username);
    db.setPassword(DialogDBSettings::dbSettings.password);

    if(!db.open())
    {
        QMessageBox::information(this, tr("Connection error"),
                                 tr("Unable connect to database\nCheck parameters and "
                                 "be sure if server is turned on"),
                                 QMessageBox::Ok);
        qDebug() << "Connection failed!" << db.lastError().text();
        return;
    }
    QString msg = tr("Successful connection to host ") +
            DialogDBSettings::dbSettings.hostname +
            tr(" , database ") +
            DialogDBSettings::dbSettings.dbname;
    qDebug() << msg;
    ui->statusbar->showMessage(msg, 5000);

    // Подключение таблицы заказов
    ordersModel = new OrdersModel(this, db);
    ui->tableViewOrders->setModel(ordersModel);
    ui->tableViewOrders->setItemDelegateForColumn(ordersModel->fieldIndex("city_id"),
                                            new QSqlRelationalDelegate(ui->tableViewOrders));
    ui->tableViewOrders->setItemDelegateForColumn(ordersModel->fieldIndex("goods_supplies_id"),
                                            new QSqlRelationalDelegate(ui->tableViewOrders));
    if(!ordersModel->select())
        qDebug() << ordersModel->lastError().text();
    connect(ordersModel, &OrdersModel::signalSelectAll, this, &MainWindow::selectAll);

    // Подключение таблицы поставок (таблица поставка-товар, goods_supplies)
    goodsSuppliesModel = new GoodsSuppliesModel(this, db);
    ui->tableViewGoodsSupplies->setModel(goodsSuppliesModel);
    ui->tableViewGoodsSupplies->setItemDelegateForColumn(goodsSuppliesModel->fieldIndex("goods_id"),
                                                         new QSqlRelationalDelegate(ui->tableViewGoodsSupplies));
    if(!goodsSuppliesModel->select())
        qDebug() << goodsSuppliesModel->lastError().text();
    connect(goodsSuppliesModel, &GoodsSuppliesModel::signalSelectAll, this, &MainWindow::selectAll);

    // Подключение таблицы supplies
    suppliesModel = new SuppliesModel(this, db);
    ui->tableViewSupplies->setModel(suppliesModel);
    if(!suppliesModel->select())
        qDebug() << suppliesModel->lastError().text();
    connect(suppliesModel, &SuppliesModel::signalSelectAll, this, &MainWindow::selectAll); // ?

    // Подключение таблицы трат в таблицу для чтения и редактирования
    chooseReadEditTable();

    // Подключение таблицы областей в таблицу чтения
    chooseReadTable();

    updateVarsInComboBoxGoodsIDs();
    updateVarsInComboBoxCities();

    // Вызывается при подключении БД для обновления записей spends.
    // Обновляются старые записи и добавляются новые вплоть до текущей даты
    // (1 запись = 1 день)
    updateSpendsTable();

/*    tableModel = new QSqlRelationalTableModel(this);
    tableModel->setTable("goods");
    tableModel->setRelation(tableModel->fieldIndex("brand_id"),
                            QSqlRelation("brands", "brand_id", "brand_name"));

    // Установка делегата на столбец
    ui->tableView->setItemDelegateForColumn(tableModel->fieldIndex("brand_id"),
                                            new QSqlRelationalDelegate(ui->tableView));

    ui->tableView->setModel(tableModel);

    if(!tableModel->select())
    {
        qDebug() << tableModel->lastError().text();
    } */
}

void MainWindow::chooseReadEditTable()
{
    // Освобождение памяти от предыдущих моделей
    if(spendsModel != nullptr)
    {
        delete spendsModel;
        spendsModel = nullptr;
    }
    if(goodsModel != nullptr)
    {
        delete goodsModel;
        goodsModel = nullptr;
    }
    if(brandsModel != nullptr)
    {
        delete brandsModel;
        brandsModel = nullptr;
    }

    // Подключение выбранной из списка модели
    // Т.к. текст в комбобоксах переводится, производим выбор по индексам
    //QString chosenTable = ui->comboBoxReadEditTables->currentText();
    int chosenIndex = ui->comboBoxReadEditTables->currentIndex();
    if(chosenIndex == 0) // spends
    {
        spendsModel = new SpendsModel(this, db);
        ui->tableViewOthersReadEdit->setModel(spendsModel);
        if(!spendsModel->select())
            qDebug() << spendsModel->lastError().text();
        qDebug() << "Table spends was selected";
        connect(spendsModel, &SpendsModel::signalSelectAll, this, &MainWindow::selectAll);
        // Убираем кнопки для spends. Вносить изменения можно только в существующие записи
        ui->pushButtonDeleteRecord->hide();
        ui->pushButtonCreateAddDialog->hide();
        // вставка наимменований столбцов в combobox
        ui->comboBoxFilterReadEditTable->clear();
        ui->comboBoxFilterReadEditTable->insertItems(0, columnsSpendsFilter);
            ui->retranslateUi(this);
        return;
    }
    if(chosenIndex == 1) // goods
    {
        goodsModel = new GoodsModel(this, db);
        ui->tableViewOthersReadEdit->setModel(goodsModel);
        ui->tableViewOthersReadEdit->setItemDelegateForColumn(goodsModel->fieldIndex("brand_id"),
                                                              new QSqlRelationalDelegate(ui->tableViewOthersReadEdit));
        if(!goodsModel->select())
            qDebug() << goodsModel->lastError().text();
        qDebug() << "Table goods was selected";
        connect(goodsModel, &GoodsModel::signalSelectAll, this, &MainWindow::selectAll);
        // Показываем кнопку "добавить". В данную таблицу можно добавлять новые записи
        // Кнопку "Удалить" убираем, т.к. удалять товары запрещено
        ui->pushButtonCreateAddDialog->show();
        ui->pushButtonDeleteRecord->hide();
        // вставка наимменований столбцов в combobox
        ui->comboBoxFilterReadEditTable->clear();
        ui->comboBoxFilterReadEditTable->insertItems(0, columnsGoodsFilter);
            ui->retranslateUi(this);
        return;
    }
    if(chosenIndex == 2) // brands
    {
        brandsModel = new BrandsModel(this, db);
        ui->tableViewOthersReadEdit->setModel(brandsModel);
        if(!brandsModel->select())
            qDebug() << brandsModel->lastError().text();
        qDebug() << "Table brands was selected";
        connect(brandsModel, &BrandsModel::signalSelectAll, this, &MainWindow::selectAll);
        // Показываем кнопки "Добавить" и "Удалить". Для данной таблицы доступны обе опции
        ui->pushButtonDeleteRecord->show();
        ui->pushButtonCreateAddDialog->show();
        // вставка наимменований столбцов в combobox
        ui->comboBoxFilterReadEditTable->clear();
        ui->comboBoxFilterReadEditTable->insertItems(0, columnsBrandsFilter);
        ui->retranslateUi(this);
        return;
    }
}

void MainWindow::chooseReadTable()
{
    // Освобождение памяти от предыдущих моделей
    if(citiesModel != nullptr)
    {
        delete citiesModel;
        citiesModel = nullptr;
    }
    if(countriesModel != nullptr)
    {
        delete countriesModel;
        countriesModel = nullptr;
    }

    // Подключение выбранной из списка модели
    // Т.к. текст в комбобоксах переводится, производим выбор по индексам
    //QString chosenTable = ui->comboBoxReadTables->currentText();
    int chosenIndex = ui->comboBoxReadTables->currentIndex();
    if(chosenIndex == 0) // cities
    {
        citiesModel = new CitiesModel(this, db);
        ui->tableViewOthersRead->setModel(citiesModel);
        ui->tableViewOthersRead->setItemDelegateForColumn(citiesModel->fieldIndex("country_id"),
                                                          new QSqlRelationalDelegate(ui->tableViewOthersRead));
        if(!citiesModel->select())
            qDebug() << citiesModel->lastError().text();
        qDebug() << "Table cities was selected";
        connect(citiesModel, &CitiesModel::signalSelectAll, this, &MainWindow::selectAll);
        return;
    }
    if(chosenIndex == 1) // countries
    {
        countriesModel = new CountriesModel(this, db);
        ui->tableViewOthersRead->setModel(countriesModel);
        if(!countriesModel->select())
            qDebug() << countriesModel->lastError().text();
        qDebug() << "Table countries was selected";
        connect(countriesModel, &CountriesModel::signalSelectAll, this, &MainWindow::selectAll);
        return;
    }
}

void MainWindow::selectAll(void* parent)
{
    qDebug() << "Select all";
    QVector<GeneralTableModel*> models;
    models << brandsModel << citiesModel << countriesModel << goodsModel <<
              goodsSuppliesModel << ordersModel << spendsModel << suppliesModel;
    for(auto &model: models)
    {
        if(model != nullptr && model != parent)
        {
            if(!model->select())
                qDebug() << model->lastError().text();
        }
    }
}


void MainWindow::readSettings()
{
    QSettings settings("Glebov", "cw-cleaning");
    settings.beginGroup("GraphicalInterface");
    resize(settings.value("size", QSize(650, 440)).toSize());
    // определение координат х, у, чтобы по умолчанию окно находилось в центре экрана
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - window()->width())/2;
    int y = (screenGeometry.height() - window()->height())/2;
    move(settings.value("pos", QPoint(x, y)).toPoint());
    QString dataOfLanguageAction = settings.value("language", "ru_RU").toString();
 /*   // установка языка через switchlanguage по dataOfLanguageAction
    int countOfActions = languageActionGroup->actions().count();
    for(int i = 0; i < countOfActions; i++)
        if(languageActionGroup->actions().at(i)->data() == dataOfLanguageAction)
        {
            switchLanguage(languageActionGroup->actions().at(i));
            break;
        } */
    settings.endGroup();

    settings.beginGroup("DataBaseSettings");
    DialogDBSettings::dbSettings.dbname = settings.value("dbname", "").toString();
    DialogDBSettings::dbSettings.hostname = settings.value("hostname", "").toString();
    DialogDBSettings::dbSettings.username = settings.value("username", "").toString();
    DialogDBSettings::dbSettings.password = settings.value("password", "").toString();
    DialogDBSettings::dbSettings.port = settings.value("port", "").toInt();
    DialogDBSettings::dbSettings.usingSSL = settings.value("usingSSL", "true").toBool();
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings("Glebov", "cw-cleaning");
    settings.beginGroup("GraphicalInterface");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    /* if(languageActionGroup->actions().count() > 0)
        settings.setValue("language", languageActionGroup->checkedAction()->data().toString()); */
    settings.endGroup();

    settings.beginGroup("DataBaseSettings");
    settings.setValue("dbname", DialogDBSettings::dbSettings.dbname);
    settings.setValue("hostname", DialogDBSettings::dbSettings.hostname);
    settings.setValue("username", DialogDBSettings::dbSettings.username);
    settings.setValue("password", DialogDBSettings::dbSettings.password);
    settings.setValue("port", DialogDBSettings::dbSettings.port);
    settings.setValue("usingSSL", DialogDBSettings::dbSettings.usingSSL);
    settings.endGroup();
}

void MainWindow::updateVarsInComboBoxGoodsIDs()
{
    QSqlQuery query;
    if(!query.exec("SELECT DISTINCT GOODS_ID FROM GOODS"))
    {
        qDebug() << query.lastError();
        return;
    }
    ui->comboBoxGoodsID->clear();
    QStringList goodsIDs;
    while(query.next())
        goodsIDs << query.value(0).toString();
    ui->comboBoxGoodsID->insertItems(0, goodsIDs); // на вкладке Orders
    ui->comboBoxGoodsID_tabSupplies->insertItems(0, goodsIDs); // на вкладке Supplies
}

void MainWindow::updateVarsInComboBoxCities()
{
    QSqlQuery query;
    if(!query.exec("SELECT DISTINCT CITY_FULL_NAME FROM CITIES"))
    {
        qDebug() << query.lastError();
        return;
    }
    ui->comboBoxCity->clear();
    QStringList cities;
    while(query.next())
        cities << query.value(0).toString();
    cities.sort(); // Сортировка по алфавиту
    ui->comboBoxCity->insertItems(0, cities);
}

void MainWindow::updateSpendsTable()
{
    // Применяется исключительно в начале подключения к БД для добавления последних записей
    // При изменении таблиц supplies или goods_supplies срабатывает триггер, и
    // таблица spends полностью обновляется. Обновляются все записи (1 запись на 1
    // день), а также добавляются новые до текущей даты. Если пользователь не заходил
    // в программу несколько дней, то БД создаст ежедневные записи сама
    QSqlQuery query;
    if(!query.exec("UPDATE supplies SET supply_id = supply_id "
                   "WHERE supply_id = (SELECT supply_id FROM supplies ORDER BY supply_id LIMIT 1);"))
        qDebug() << query.lastError().text();
    else
        qDebug() << "Table spends updated successfully";
    if(spendsModel)
        spendsModel->select();
}

void MainWindow::setConnects()
{
    // Связи на кнопки меню
    connect(ui->actionConnectDB, &QAction::triggered, this, &MainWindow::connectDB);
    connect(ui->actionDBSettings, &QAction::triggered, this, [&](){
                                                            DialogDBSettings dialogSettings;
                                                            dialogSettings.exec();
                                                                });
    connect(ui->actionAboutProgram, &QAction::triggered, this, &MainWindow::showDialogAboutProgram);
    connect(ui->actionProfitCalculation, &QAction::triggered, this, &MainWindow::calculateProfit);

    // Связи на элементы UI
    connect(ui->comboBoxReadEditTables, &QComboBox::currentTextChanged, this, &MainWindow::chooseReadEditTable);
    connect(ui->comboBoxReadTables, &QComboBox::currentTextChanged, this, &MainWindow::chooseReadTable);

    // СИГНАЛ-СЛОТЫ НА СОРТИРОВКУ
    // Создаем вектор пар "представление"-"модель" для более короткой записи коннектов
    QVector<QPair<QHeaderView*, GeneralTableModel*> > pairsViewModel;
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewOrders->horizontalHeader(), ordersModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewGoodsSupplies->horizontalHeader(), goodsSuppliesModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewSupplies->horizontalHeader(), suppliesModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewOthersReadEdit->horizontalHeader(), spendsModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewOthersReadEdit->horizontalHeader(), goodsModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewOthersReadEdit->horizontalHeader(), brandsModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewOthersRead->horizontalHeader(), citiesModel));
    pairsViewModel.append(QPair<QHeaderView*, GeneralTableModel*>(ui->tableViewOthersRead->horizontalHeader(), countriesModel));

    // Создаем связи сигнал-слотов для каждой пары представление - модель
    // В нашем случае к 1 таблице может быть прикреплено несколько моделей
    // И некоторые модели могут быть пустые (nullptr). Поэтому делаем проверку
    // в слоте при испускании сигнала
    for(auto &viewModel: pairsViewModel)
    {
        QHeaderView *headerView = viewModel.first;
        GeneralTableModel *model = viewModel.second;
        connect(headerView, &QHeaderView::sectionClicked, [=](int logicalIndex) {
            Qt::SortOrder order = (headerView->sortIndicatorSection() == logicalIndex &&
                                   headerView->sortIndicatorOrder() == Qt::AscendingOrder) ?
                                    Qt::DescendingOrder : Qt::AscendingOrder;
            if(model != nullptr) model->sort(logicalIndex, order);             });
    }
}

void MainWindow::resetAllModels()
{
    // Обнуляем все модели, если они не nullptr
    if(ordersModel != nullptr)
    {
        delete ordersModel;
        ordersModel = nullptr;
    }
    if(goodsSuppliesModel != nullptr)
    {
        delete goodsSuppliesModel;
        goodsSuppliesModel = nullptr;
    }
    if(suppliesModel != nullptr)
    {
        delete suppliesModel;
        suppliesModel = nullptr;
    }
    if(spendsModel != nullptr)
    {
        delete spendsModel;
        spendsModel = nullptr;
    }
    if(goodsModel != nullptr)
    {
        delete goodsModel;
        goodsModel = nullptr;
    }
    if(brandsModel != nullptr)
    {
        delete brandsModel;
        brandsModel = nullptr;
    }
    if(citiesModel != nullptr)
    {
        delete citiesModel;
        citiesModel = nullptr;
    }
    if(countriesModel != nullptr)
    {
        delete countriesModel;
        countriesModel = nullptr;
    }
}

void MainWindow::on_checkBoxDelivery_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->dateEditDelivery->setEnabled(ui->checkBoxDelivery->isChecked());
}


void MainWindow::on_pushButtonInsertNewOrder_clicked()
{
    if(!db.isOpen())
    {
        qDebug() << "database is not connected";
        return;
    }

    QMessageBox::StandardButton res = QMessageBox::question(this, tr("Inserting value"),
                                     tr("Are you sure you want to insert the record?"),
                                     QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Cancel)
        return;

    if(!db.transaction())
    {
        qDebug() << db.lastError().text();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with opening transaction.\n%1 \nInsert failed").arg(db.lastError().text()));
        return;
    }
    qDebug() << "Transaction start";

    QString dateOfOrder = ui->dateEditNewOrder->date().toString("yyyy-MM-dd");
    QString dateOfDelivery = "";
    ui->checkBoxDelivery->isChecked() ? dateOfDelivery = ui->dateEditDelivery->date().toString("yyyy-MM-dd") :
            dateOfDelivery = "";
    double orderPrice = ui->doubleSpinBoxOrderPrice->value();
    double logistics = ui->doubleSpinBoxLogistics->value();
    // Нахождение записи goods_supplies с нужным товаром, где на складе еще есть остатки
    // (то есть count_in_stock > 0)
    long long goodsID = ui->comboBoxGoodsID->currentText().toLongLong();
    long long goodsSuppliesID;
    QSqlQuery query;
    if(!query.exec(QString("SELECT GOODS_SUPPLIES_ID FROM GOODS_SUPPLIES  "
                           "WHERE GOODS_ID = %1 AND COUNT_IN_STOCK > 0"
                           "LIMIT 1;").arg(QString::number(goodsID))))
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with taking data from DB. \nInsert failed"));
        return;
    }
    query.next();
    goodsSuppliesID = query.value(0).toLongLong();

    // Нахождение city_id по city_full_name
    QString cityID;
    QString cityFullName = ui->comboBoxCity->currentText();
    if(!query.exec(QString("SELECT CITY_ID FROM CITIES "
                           "WHERE CITY_FULL_NAME = '%1';").arg(cityFullName)))
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with taking data from DB. \nInsert failed"));
        return;
    }
    query.next();
    cityID = query.value(0).toString();

    // Внесение новой строки в таблицу ORDERS
    query.prepare("INSERT INTO orders VALUES (DEFAULT, :date_of_order, :date_delivered_order, "
                  ":is_cancelled, :earnings, :logistics_cost, :goods_supplies_id, "
                  ":city_id);");
    //query.bindValue(":order_id", orderID);
    query.bindValue(":date_of_order", dateOfOrder);
    if(ui->checkBoxDelivery->isChecked())
        query.bindValue(":date_delivered_order", dateOfDelivery);
    query.bindValue(":is_cancelled", "FALSE");
    query.bindValue(":earnings", orderPrice);
    query.bindValue(":logistics_cost", logistics);
    query.bindValue(":goods_supplies_id", goodsSuppliesID);
    query.bindValue(":city_id", cityID);
    if(!query.exec())
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with inserting data in table orders. \nInsert failed"));
        return;
    }

    // Уменьшаем количество товаров на складе на единицу
    if(!query.exec(QString("UPDATE goods_supplies SET count_in_stock = count_in_stock - 1 "
                       "WHERE goods_supplies_id = %1").arg(QString::number(goodsSuppliesID))))
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with updating data in table goods_supplies. \nInsert failed"));
        return;
    }

    // Если все успешно, то коммитим изменения в БД
    if(!db.commit())
    {
        // Если коммит не удался, то уведомляем пользователя и откатываемся
        qDebug() << db.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with commiting changes in DB. \nInsert failed"));
    }
    qDebug() << "Transaction end";
    QMessageBox::information(this, tr("Insert completed"),
                             tr("Inserting completed successfully"));

    selectAll();
}


void MainWindow::on_pushButtonInsertSupply_clicked()
{
    if(!db.isOpen())
    {
        qDebug() << "database is not connected";
        return;
    }

    QMessageBox::StandardButton res = QMessageBox::question(this, tr("Inserting value"),
                                     tr("Are you sure you want to insert the record?"),
                                     QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Cancel)
        return;

    if(!db.transaction())
    {
        qDebug() << db.lastError().text();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with opening transaction. \nInsert failed"));
        return;
    }
    qDebug() << "Transaction start";

    long long supplyID = ui->lineEditSupplyID->text().toLongLong();
    double deliverySpends = ui->doubleSpinBoxDelivSpends->value();
    QString dateOfSupply = ui->dateEditSupply->date().toString("yyyy-MM-dd");

    // Нахождение ID трат (spends_id) для той даты, которая в новой поставке.
    // Прикрепленние данной поставки к тратам на конкретный день
    QSqlQuery query;
    if(!query.exec(QString("SELECT spends_id FROM spends "
                           "WHERE date_of_creating_act = '%1'").arg(dateOfSupply)))
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with getting data from table spends. \nInsert failed"));
        return;
    }
    query.next();
    int spendsID = query.value(0).toInt();

    // Внесение новой строки в таблицу supplies
    if(!query.exec(QString("INSERT INTO supplies VALUES(%1, %2, '%3', %4)")
                        .arg(supplyID)
                        .arg(deliverySpends)
                        .arg(dateOfSupply)
                        .arg(spendsID)))
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with inserting data in table supply. \nInsert failed"));
        return;
    }

    // Работа с goods_supplies
    long long goodsID = ui->comboBoxGoodsID_tabSupplies->currentText().toLongLong();
    int countInSupply = ui->spinBoxCountOfGoods->value();
    int countInStock = countInSupply;
    double costPerOne = ui->doubleSpinBoxCostPerOne->value();

    if(!query.exec(QString("INSERT INTO goods_supplies VALUES(DEFAULT, %1, %2, %3, "
                           "%4, %5)")
                        .arg(goodsID)
                        .arg(supplyID)
                        .arg(countInSupply)
                        .arg(countInStock)
                        .arg(costPerOne)))
    {
        qDebug() << query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with inserting data in table supply. \nInsert failed"));
        return;
    }

    // Если все успешно, то коммитим изменения в БД
    if(!db.commit())
    {
        // Если коммит не удался, то уведомляем пользователя и откатываемся
        qDebug() << db.lastError().text();
        db.rollback();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with commiting changes in DB. \nInsert failed"));
    }
    qDebug() << "Transaction end";
    QMessageBox::information(this, tr("Insert completed"),
                             tr("Inserting completed successfully"));

    selectAll();
}


void MainWindow::on_pushButtonCreateAddDialog_clicked()
{
    if(!db.isOpen())
    {
        qDebug() << "database is not connected";
        return;
    }

    // Сравнение с индексом в комбобоксе, т.к. по именам не проверить, они
    // меняются из-за смены языка
    if(ui->comboBoxReadEditTables->currentIndex() == 1)
    {
        // Получение списка брендов
        QSqlQuery query;
        if(!query.exec("SELECT brand_name FROM brands;"))
        {
            qDebug() << query.lastError().text();
            QMessageBox::critical(this, tr("Select error"),
                                  tr("Error with getting data from DB (brands table). \nAdding failed"));
            return;
        }
        QStringList brands;
        while(query.next())
            brands << query.value(0).toString();

        DialogAddGoods *dialog = new DialogAddGoods(brands);
        connect(dialog, &DialogAddGoods::dataAccepted, this, &MainWindow::insertDataIntoGoodsTable);
        if(dialog->exec() == QDialog::Rejected)
        {
            qDebug() << "DialogAddGoods rejected";
            return;
        }
        qDebug() << "DialogAddGoods accepted";
        delete dialog;
        return;
    }

    if(ui->comboBoxReadEditTables->currentIndex() == 2)
    {
        DialogAddBrand *dialog = new DialogAddBrand();
        connect(dialog, &DialogAddBrand::dataAccepted, this, &MainWindow::insertDataIntoBrandsTable);
        if(dialog->exec() == QDialog::Rejected)
        {
            qDebug() << "DialogAddBrand rejected";
            return;
        }
        qDebug() << "DialogAddBrand accepted";
        delete dialog;
        return;
    }
}

void MainWindow::insertDataIntoGoodsTable(const long long goodsID,
                                          const QString goodsName,
                                          const QString sellerArticle,
                                          const QString brandName)
{
    QSqlQuery query;
    if(!query.exec(QString("SELECT brand_id FROM brands "
                           "WHERE brand_name = '%1';").arg(brandName)))
    {
        qDebug() << query.lastError().text();
        QMessageBox::critical(this, tr("Select error"),
                              tr("Error with getting data from DB (brands table). \nAdding failed"));
        return;
    }
    query.next();
    int brandID = query.value(0).toInt();
    if(!query.exec(QString("INSERT INTO goods VALUES "
                           "(%1, '%2', '%3', %4);").arg(goodsID)
                                                    .arg(goodsName)
                                                    .arg(sellerArticle)
                                                    .arg(brandID)))
    {
        qDebug() << query.lastError().text();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with inserting data into DB (goods table). \nAdding failed"));
        return;
    }

    QMessageBox::information(this, tr("Insert completed"),
                             tr("Inserting completed successfully"));
    selectAll();
    updateVarsInComboBoxGoodsIDs();
}

void MainWindow::insertDataIntoBrandsTable(const QString brandName)
{
    QSqlQuery query;
    if(!query.exec(QString("INSERT INTO brands VALUES "
                           "(DEFAULT, '%1');").arg(brandName)))
    {
        qDebug() << query.lastError().text();
        QMessageBox::critical(this, tr("Insert error"),
                              tr("Error with inserting data into DB (brands table). \nAdding failed"));
        return;
    }

    QMessageBox::information(this, tr("Insert completed"),
                             tr("Inserting completed successfully"));
    selectAll();
}


void MainWindow::on_pushButtonDeleteRecord_clicked()
{
    QModelIndexList selectedRows = ui->tableViewOthersReadEdit->selectionModel()->selectedRows();
    if(selectedRows.isEmpty())
        return;

    int firstSelectedRow = selectedRows[0].row();
    int countOfSelectedRows = selectedRows.count();
    QModelIndex index = selectedRows[0].parent();

    if(countOfSelectedRows > 1)
    {
        qDebug() << "For now you can delete only 1 record a time";
        QMessageBox::critical(this, tr("Deleting warning"),
                             tr("For now you can delete only 1 record a time"));
        return;
    }
    if(!brandsModel->removeRows(firstSelectedRow, countOfSelectedRows, index))
    {
        qDebug() << "Error with deleting rows from table brands";
        QMessageBox::critical(this, tr("Deleting error"),
                             tr("Deleting from table Brands failed"));
        return;
    }
    qDebug() << "Selected rows deleted";
    selectAll();
    return;
}


// /////////////////////////// Фильтрация по столбцам /////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButtonApplyFilterOrders_clicked()
{
    if(ordersModel == nullptr)
        return;

    QString columnName;
    // по колонке индексу 3 не фильтруемся, там "Yes/No"
    switch(ui->comboBoxFilterOrders->currentIndex())
    {
    case 0: return; // фильтр не выбран
    case 1: columnName = "order_id";             break;
    case 2: columnName = "date_of_order";        break;
    case 3: columnName = "date_delivered_order"; break;
    case 4: columnName = "earnings";             break;
    case 5: columnName = "logistic_cost";        break;
    case 6: columnName = "goods_id";             break;
    case 7: columnName = "city_full_name";       break;
    }

    ordersModel->setFilter(QString("CAST(%1 AS TEXT) ILIKE '%%2%'").arg(columnName)
                                                                   .arg(ui->lineEditFilterOrders->text()));
    if(!ordersModel->select())
        qDebug() << ordersModel->lastError().text();
}

void MainWindow::on_pushButtonApplyFilterGoodsSupplies_clicked()
{
    if(goodsSuppliesModel == nullptr)
        return;

    QString columnName;
    switch(ui->comboBoxFilterGoodsSupplies->currentIndex())
    {
    case 0: return; // фильтр не выбран
    case 1: columnName = "goods_supplies_id";    break;
    case 2: columnName = "seller_article";       break;
    case 3: columnName = "supply_id";            break;
    case 4: columnName = "count_in_supply";      break;
    case 5: columnName = "count_in_stock";       break;
    case 6: columnName = "cost_per_one";         break;
    }

    goodsSuppliesModel->setFilter(QString("CAST(%1 AS TEXT) ILIKE '%%2%'").arg(columnName)
                                                                   .arg(ui->lineEditFilterGoodsSupplies->text()));
    if(!goodsSuppliesModel->select())
        qDebug() << goodsSuppliesModel->lastError().text();
}

void MainWindow::on_pushButtonApplyFilterSupplies_clicked()
{
    if(suppliesModel == nullptr)
        return;

    QString columnName;
    switch(ui->comboBoxFilterSupplies->currentIndex())
    {
    case 0: return; // фильтр не выбран
    case 1: columnName = "supply_id";      break;
    case 2: columnName = "delivery_spends";break;
    case 3: columnName = "date_of_supply"; break;
    case 4: columnName = "spends_id";      break;
    }

    suppliesModel->setFilter(QString("CAST(%1 AS TEXT) ILIKE '%%2%'").arg(columnName)
                                                                   .arg(ui->lineEditFilterSupplies->text()));
    if(!suppliesModel->select())
        qDebug() << suppliesModel->lastError().text();
}

void MainWindow::on_pushButtonApplyFilterReadEditTable_clicked()
{
    GeneralTableModel *model;
    QStringList columnsOfTable;
    int columnIndex = ui->comboBoxFilterReadEditTable->currentIndex();
    if(spendsModel != nullptr)
    {
        model = spendsModel;
        columnsOfTable = columnsSpendsFilter_EN;
    }
    else if(goodsModel != nullptr)
    {
        model = goodsModel;
        columnsOfTable = columnsGoodsFilter_EN;
    }
    else if(brandsModel != nullptr)
    {
        model = brandsModel;
        columnsOfTable = columnsBrandsFilter_EN;
    }
    else
        return;

    // Если выбран первый пункт, то означает, что фильтр не используется
    // Там пункт "- Not selected -"
    if(columnIndex == 0)
        return;

    model->setFilter(QString("CAST(%1 AS TEXT) ILIKE '%%2%'").arg(columnsOfTable[columnIndex])
                                                             .arg(ui->lineEditFilterReadEditTable->text()));
    if(!model->select())
        qDebug() << model->lastError().text();
}

void MainWindow::on_pushButtonResetFilterOrders_clicked()
{
    if(ordersModel == nullptr)
        return;
    ui->lineEditFilterOrders->setText("");
    ordersModel->setFilter("");
    ordersModel->select();
}

void MainWindow::on_pushButtonResetFilterGoodsSupplies_clicked()
{
    if(goodsSuppliesModel == nullptr)
        return;
    ui->lineEditFilterGoodsSupplies->setText("");
    goodsSuppliesModel->setFilter("");
    goodsSuppliesModel->select();
}


void MainWindow::on_pushButtonResetFilterSupplies_clicked()
{
    if(suppliesModel == nullptr)
        return;
    ui->lineEditFilterSupplies->setText("");
    suppliesModel->setFilter("");
    suppliesModel->select();
}

void MainWindow::on_pushButtonResetFilterReadEditTable_clicked()
{
    GeneralTableModel *model;
    if(spendsModel != nullptr)
        model = spendsModel;
    else if(goodsModel != nullptr)
        model = goodsModel;
    else if(brandsModel != nullptr)
        model = brandsModel;
    else
        return;

    ui->lineEditFilterReadEditTable->setText("");
    model->setFilter("");
    model->select();
}

void MainWindow::showDialogAboutProgram()
{
    DialogAboutProgram dialog;
    dialog.exec();
}

void MainWindow::calculateProfit()
{
    if(!db.isOpen())
    {
        qDebug() << "database is not connected";
        QMessageBox::information(this, tr("Database is not connected"),
                             tr("Connect to database to use this tool"));
        return;
    }

    DialogCalculateProfit dialog(this, &db);
    dialog.exec();
}


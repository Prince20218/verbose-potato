/**
 * @file mainwindow.h
 * @brief Заголовочный файл класса MainWindow
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRelation>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QTranslator>

#include "Tables/ordersmodel.h"
#include "Tables/goodssuppliesmodel.h"
#include "Tables/brandsmodel.h"
#include "Tables/citiesmodel.h"
#include "Tables/countriesmodel.h"
#include "Tables/goodsmodel.h"
#include "Tables/spendsmodel.h"
#include "Tables/suppliesmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Класс главного окна приложения
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    // Поля класса
    //////////////
private:
    Ui::MainWindow *ui; /**< Указатель на объект интерфейса главного окна */
    QSqlDatabase db; /**< Объект базы данных */
    QTranslator translator; /**< Объект перевода */

    // модели таблиц
    OrdersModel        *ordersModel {nullptr}; /**< Модель таблицы заказов */
    GoodsSuppliesModel *goodsSuppliesModel {nullptr}; /**< Модель таблицы поставок_товаров */
    BrandsModel        *brandsModel {nullptr}; /**< Модель таблицы брендов */
    CitiesModel        *citiesModel {nullptr}; /**< Модель таблицы городов */
    CountriesModel     *countriesModel {nullptr}; /**< Модель таблицы стран */
    GoodsModel         *goodsModel {nullptr}; /**< Модель таблицы товаров */
    SpendsModel        *spendsModel {nullptr}; /**< Модель таблицы расходов */
    SuppliesModel      *suppliesModel {nullptr}; /**< Модель таблицы поставщиков */

    QStringList columnsSpendsFilter; /**< Список столбцов для фильтрации таблицы расходов */
    QStringList columnsGoodsFilter; /**< Список столбцов для фильтрации таблицы товаров */
    QStringList columnsBrandsFilter; /**< Список столбцов для фильтрации таблицы брендов */
    // на английском языке для запросов в БД (без перевода)
    QStringList columnsSpendsFilter_EN; /**< Список столбцов для фильтрации таблицы расходов */
    QStringList columnsGoodsFilter_EN; /**< Список столбцов для фильтрации таблицы товаров */
    QStringList columnsBrandsFilter_EN; /**< Список столбцов для фильтрации таблицы брендов */

    // прокси-модели таблиц
//    QSortFilterProxyModel *proxyOrders {nullptr};
//    QSortFilterProxyModel *proxyGoodsSupplies {nullptr};
//    QSortFilterProxyModel *proxySupplies {nullptr};
//    QSortFilterProxyModel *proxyReadEditTable {nullptr}; // для моделей с view ReadEdit (3 вкладка)
//    QSortFilterProxyModel *proxyReadTable {nullptr}; // для моделей с view Read (3 вкладка)


    // Методы класса
    //////////////
public:
    /**
     * @brief Конструктор класса MainWindow
     * @param parent Указатель на родительский объект
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса MainWindow
     */
    ~MainWindow();

private slots:
    /**
     * @brief Метод для подключения к базе данных
     */
    void connectDB();

    /**
     * @brief Метод для выбора таблицы для просмотра и редактирования
     */
    void chooseReadEditTable();

    /**
     * @brief Метод для выбора таблицы для просмотра
     */
    void chooseReadTable();

    /**
     * @brief Метод для выбора всех записей в таблице
     * @param parent Указатель на родительский объект
     */
    void selectAll(void* parent = nullptr);

    /**
     * @brief Слот для изменения состояния флажка "Доставка"
     * @param arg1 Новое состояние флажка
     */
    void on_checkBoxDelivery_stateChanged(int arg1);

    /**
     * @brief Слот для добавления нового заказа
     */
    void on_pushButtonInsertNewOrder_clicked();

    /**
     * @brief Слот для добавления новой поставки товаров
     */
    void on_pushButtonInsertSupply_clicked();

    /**
     * @brief Слот для создания диалогового окна добавления записи (для goods и brands)
     */
    void on_pushButtonCreateAddDialog_clicked();

    /**
     * @brief Метод для добавления данных в таблицу товаров
     * @param goodsID Идентификатор товара
     * @param goodsName Название товара
     * @param sellerArticle Артикул продавца
     * @param brandName Название бренда
     */
    void insertDataIntoGoodsTable(const long long goodsID,
                                  const QString goodsName,
                                  const QString sellerArticle,
                                  const QString brandName);

    /**
     * @brief Метод для добавления данных в таблицу брендов
     * @param brandName Название бренда
     */
    void insertDataIntoBrandsTable(const QString brandName);

private slots:
    /**
     * @brief Слот для удаления записи из таблицы
     */
    void on_pushButtonDeleteRecord_clicked();

    /**
     * @brief Слот для применения фильтра к таблице заказов
     */
    void on_pushButtonApplyFilterOrders_clicked();

    /**
     * @brief Слот для сброса фильтра в таблице заказов
     */
    void on_pushButtonResetFilterOrders_clicked();

    /**
     * @brief Слот для применения фильтра к таблице
     * поставок товаров
     */
    void on_pushButtonApplyFilterGoodsSupplies_clicked();

    /**
     * @brief Слот для сброса фильтра в таблице поставок товаров
     */
    void on_pushButtonResetFilterGoodsSupplies_clicked();

    /**
     * @brief Слот для применения фильтра к таблице поставщиков
     */
    void on_pushButtonApplyFilterSupplies_clicked();

    /**
     * @brief Слот для сброса фильтра в таблице поставщиков
     */
    void on_pushButtonResetFilterSupplies_clicked();

    /**
     * @brief Слот для применения фильтра к таблице с view ReadEdit (3 вкладка)
     */
    void on_pushButtonApplyFilterReadEditTable_clicked();

    /**
     * @brief Слот для сброса фильтра в таблице с view ReadEdit (3 вкладка)
     */
    void on_pushButtonResetFilterReadEditTable_clicked();

    /**
     * @brief Метод для отображения диалогового окна "О программе"
     */
    void showDialogAboutProgram();

    /**
     * @brief Метод для расчета прибыли
     */
    void calculateProfit();

    private:
    /**
     * @brief Метод для чтения настроек
     */
    void readSettings();

    /**
     * @brief Метод для записи настроек
     */
    void writeSettings();

    /**
     * @brief Метод для обновления переменных в выпадающем списке идентификаторов товаров
     */
    void updateVarsInComboBoxGoodsIDs();

    /**
     * @brief Метод для обновления переменных в выпадающем списке городов
     */
    void updateVarsInComboBoxCities();

    /**
     * @brief Метод для обновления таблицы расходов
     */
    void updateSpendsTable();

    /**
     * @brief Метод для установки соединений
     */
    void setConnects();

    /**
     * @brief Метод для сброса всех моделей таблиц
     */
    void resetAllModels();
};
#endif // MAINWINDOW_H

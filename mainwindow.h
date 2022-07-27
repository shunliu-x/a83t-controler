#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QTime>
#include <QVector>
#include <QListWidgetItem>
#include "QtCharts/QChart"
#include "QTimer"
#include "QTime"
#include "QList"
#include "qmath.h"
#include "QPointF"
#include <qdebug.h>
#include <QProcess>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QTextStream>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();





    //bottom tip recv message show different info,operate:0~5,connect->...->restart;message,0:failed,1:sucess,2:other
    void change_text_connect_info(int operate , int message);



    //init mainwindow
    void init_photo_sign(void);
    void init_area_model_clk_set(void);
    void init_area_singnal_model(void);
    void init_area_gpio_set(void);
    void init_area_pwm_set(void);



    //singnal model area combo box fill
    void area_mipi_fill(void);
    void area_mipi_fill(int cbb_0_value , int cbb_1_value , int cbb_2_value);
    void area_lvds_fill(void);
    void area_lvds_fill(int cbb_0_value , int cbb_1_value , int cbb_2_value);
    void area_rgb_fill(void);
    void area_rgb_fill(int cbb_0_value ,  int cbb_1_value , int cbb_2_value);

    //set and get all input value
    void set_all_value(void);
    void get_all_value(void);

    //initial code get and set
    void get_initial_code(void);
    void set_initial_code();

    //save config to /disp/ to product 3 file,display.cfg , disp_init.cfg , display_timeseq.cfg
    void generate_display(void);
    void generate_disp_init(void);
    void generate_display_timeseq(void);
    void save_config_to_temp_file(void);

    //adb process
    void initial_adb_process(void);
    int adb_get_producter(void);
    int adb_get_product_model(void);



    //check necessary file
    bool check_adb_file(void);
    void check_disp_param_dir(void);

    //all button on/off
    void all_button_unenable(void);
    void all_button_enable(void);

    //widget list initial
    void all_value_widget_initial();


public slots:
    //adb process callback
    void on_readoutput(void);
    void on_readerror(void);

    //switch singnal model area ,0:mipi , 1:lvds , 2:rgb
    void switch_area_singnal_model();

    //config operate, 1:sucess , 0: failed
    bool on_bu_connect_clicked(void);
    void on_bu_update_clicked(void);
    bool on_bu_download_clicked(void);
    bool on_bu_save_clicked(void);
    int  on_bu_load_clicked(void); //0:sucess,1:cancel,2:file type error
    bool on_bu_restart_clicked(void);


    //online debug,only return connect or not,0:no deivece,1:connect
    bool on_bu_android_on_clicked(void);
    bool on_bu_android_off_clicked(void);
    bool on_bu_image_next_clicked(void);
    bool on_bu_image_prev_clicked(void);

    //initial code click
    void on_bu_load_init_code_clicked(void);

    //key board click menthod
    void download_image_to_device(void);
    void clear_all_value(void);
    void clear_gpio_set(void);

private:
    Ui::MainWindow *ui;
    bool is_connect_device;
    QProcess *adb_process;
    QString recv_data;
    QStringList load_file_data;
    QStringList save_file_data;
    QStringList display_cfg_file_data;
    QStringList disp_init_cfg_file_data;
    QStringList display_timeseq_file_data;
    QList<QWidget*> all_value_widget;
    QList<QPushButton*> all_button;
    QStringList init_code;
    QString command;
    QString initial_code_file_dir;
protected:
    void keyPressEvent(QKeyEvent *event);
};
#endif // MAINWINDOW_H

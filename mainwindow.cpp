#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    check_disp_param_dir();
    if(!check_adb_file()){
        QMessageBox::question(this,
                              tr("错误"),
                              tr("缺少adb文件"),
                              QMessageBox::Ok,
                              QMessageBox::Ok);
        all_button_unenable();
    }
    adb_process = new QProcess(this);
    adb_process->setReadChannel(QProcess::StandardOutput);
    adb_process->setWorkingDirectory(QDir::currentPath()+"/adb/");
    connect(adb_process , SIGNAL(readyReadStandardOutput()) , this , SLOT(on_readoutput()));
    connect(adb_process , SIGNAL(errorOccurred(QProcess::ProcessError)) , this , SLOT(on_readerror()));
    is_connect_device = false;
    recv_data = "";
    command="./adb/adb.exe";
    all_value_widget_initial();
    init_photo_sign();
    init_area_pwm_set();
    init_area_gpio_set();
    initial_adb_process();
    init_area_model_clk_set();
    init_area_singnal_model();
}

MainWindow::~MainWindow()
{
    adb_process->kill();
    delete adb_process;
    delete ui;
}

bool MainWindow::on_bu_connect_clicked(void){
    all_button_unenable();
    is_connect_device = false;
    int step1 = adb_get_producter();
    if(step1!=0){
        change_text_connect_info(0,0);
        all_button_enable();
        return false;
    }
    int step2 = adb_get_product_model();
    if(step2!=0){
        change_text_connect_info(0,0);
        all_button_enable();
        return false;
    }
    is_connect_device = true;
    change_text_connect_info(0,1);
    all_button_enable();
    return true;
}

void MainWindow::on_bu_update_clicked(){

}

bool MainWindow::on_bu_download_clicked(){
    all_button_unenable();
    save_config_to_temp_file();
    if(!is_connect_device){
        change_text_connect_info(2,0);
        all_button_enable();
        return false;
    }


    QStringList args;
    QString path = QDir::currentPath();
    args.append("push");
    args.append(path+"/disp_param/display.cfg");
    args.append("/bootloader/display.cfg");
    adb_process->start(command , args);
    adb_process->waitForFinished();
    args.clear();
    args.append("push");
    args.append(path+"/disp_param/disp_init.cfg");
    args.append("/data/disp_init.cfg");
    adb_process->start(command , args);
    adb_process->waitForFinished();
    args.clear();
    args.append("push");
    args.append(path+"/disp_param/display_timeseq.cfg");
    args.append("/data/display_timeseq.cfg");
    adb_process->start(command , args);
    adb_process->waitForFinished();
    all_button_enable();
    return true;
}

bool MainWindow::on_bu_save_clicked(){
    all_button_unenable();
    generate_display_timeseq();
    QString save_file_name = QFileDialog::getSaveFileName(this,
                                               tr("Open Directory"),
                                               "",
                                               tr("Config Files (*.cfg)"));
    if(save_file_name.isNull()){
        all_button_enable();
        return false;
    }
    QFile f(save_file_name);
    if(!f.open(QFile::WriteOnly|QFile::Text)){
        all_button_enable();
        return false;
    }
    get_all_value();
    QTextStream out(&f);
    for(QString s:save_file_data){
        out<<s<<"\n";
    }
    f.close();
    all_button_enable();
    return true;
}

int MainWindow::on_bu_load_clicked(){
    all_button_unenable();
    QString load_file_name = QFileDialog::getOpenFileName(this,
                                                          tr("Open File"),
                                                          "",
                                                          tr("Config Files (*.cfg)"),
                                                          0);
    if(load_file_name.isNull()){
        all_button_enable();
        //change_text_connect_info(4,0);
        return 1;
    }
    qDebug()<<"load config from:"<<load_file_name;
    QFile f(load_file_name);
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text)){
        change_text_connect_info(4,1);
        all_button_enable();
        return 1;
    }
    clear_all_value();
    QString str = f.readAll();

    load_file_data = str.split("\n");
    qDebug()<<"load file size:"<<load_file_data.size();
    set_all_value();
    change_text_connect_info(4,2);
    all_button_enable();
    return 0;
}

bool MainWindow::on_bu_restart_clicked(){
    QStringList args;
    args.append("reboot");
    adb_process->start(command , args);
    return true;
}

void MainWindow::change_text_connect_info(int operate, int message){
    switch (operate) {
    case 0:
        if(message){
            ui->text_connect_info->setText("连接成功");
        }
        else {
           ui->text_connect_info->setText("无法连接到设备，请检查usb线或驱动");
        }
        break;
    case 1:
        break;
    case 2:
        if(message){
            ui->text_connect_info->setText("下载配置成功");
        }
        else {
            ui->text_connect_info->setText("下载失败，请检查usb线或驱动");
        }
        break;
    case 3:
        break;
    case 4:
        if(message==2){
            ui->text_connect_info->setText("加载配置成功");
        }
        else if(message == 1){
            ui->text_connect_info->setText("加载配置文件失败，请检查文件格式");
        }else{

        }
        break;
    case 5:
        break;
    case 7:
        if(message == 0){
            ui->text_connect_info->setText("无效文件");
        }else if(message == 1){

        }
    default:
        break;
    }
}

bool MainWindow::on_bu_android_on_clicked(){
    QStringList args;
    args.append("shell");
    args.append("cat");
    args.append("/sys/kernel/wschipadbinterface/enable2");
    adb_process->start(command,args);
    adb_process->waitForFinished();
    
    return true;
}

bool MainWindow::on_bu_android_off_clicked(){
    QStringList args;
    args.append("shell");
    args.append("cat");
    args.append("/sys/kernel/wschipadbinterface/enable3");
    adb_process->start(command,args);
    adb_process->waitForFinished();
    return true;
}

bool MainWindow::on_bu_image_next_clicked(){
    QStringList args;
    args.append("shell");
    args.append("cat");
    args.append("/sys/kernel/wschipadbinterface/enable4");
    adb_process->start(command,args);
    adb_process->waitForFinished();
    return true;
}

bool MainWindow::on_bu_image_prev_clicked(){
    QStringList args;
    args.append("shell");
    args.append("cat");
    args.append("/sys/kernel/wschipadbinterface/enable5");
    adb_process->start(command,args);
    adb_process->waitForFinished();
    return true;
}

void MainWindow::init_photo_sign(){
    QImage *img = new QImage;
    if(!img->load("./image/knox.png")){
        ui->photo_sign->setText("");
        qDebug()<<"no image file int /image/knox.png";
        return ;
    }
    ui->photo_sign->setPixmap(QPixmap::fromImage(*img));
}

void MainWindow::init_area_model_clk_set(){
    QStringList strlist;
    strlist<<"rgb"<<"lvds"<<"mipi";
    ui->cbb_model_select->addItems(strlist);
    ui->cbb_model_select->setCurrentIndex(-1);
    connect(ui->cbb_model_select, SIGNAL(currentIndexChanged(int)) , this , SLOT(switch_area_singnal_model()));
}

void MainWindow::init_area_singnal_model(){
    ui->text_0->setText("");
    ui->text_1->setText("");
    ui->text_2->setText("");
    ui->cbb_0->clear();
    ui->cbb_1->clear();
    ui->cbb_2->clear();
    qDebug()<<"init singnal model area sucess";
}

void MainWindow::init_area_gpio_set(){
    QStringList strlist;
    strlist<<"RSTH"<<"VDDIO"<<"VDD"<<"VSP"<<"VSN"<<"TPVDDIO"<<"TPAVDD"<<"OLED"<<"MTP"<<"VBL"<<"Gpio1"<<"Gpio2"<<"VDD_5V"<<"RSTL";
    QList<QComboBox*> all_combo_box;
    int cnt = all_value_widget.count();

    for(int i = cnt-40 ; i<cnt ; i+=2){
        QComboBox *it = (QComboBox *)all_value_widget[i];
        it->addItems(strlist);
        it->setCurrentIndex(-1);
    }
    qDebug()<<"init gpio area sucess";
}

void MainWindow::init_area_pwm_set(){
    ui->cbb_pwm_pwm->addItem("Used");
    ui->cbb_pwm_pwm->addItem("No Used");
    ui->cbb_pwm_pwm->setCurrentIndex(-1);
    ui->cbb_pwm_pol->addItem("active Low");
    ui->cbb_pwm_pol->addItem("active High");
    ui->cbb_pwm_pol->setCurrentIndex(-1);
}

void MainWindow::switch_area_singnal_model(){
    int model_type = ui->cbb_model_select->currentIndex();
    qDebug()<<"model type select:"<<model_type;
    if(model_type == 2){
        ui->text_0->clear();
        ui->text_0->setText("Mipi Model");
        ui->text_1->clear();
        ui->text_1->setText("Mipi_lane");
        ui->text_2->clear();
        ui->text_2->setText("Data_Bit");
        QStringList tem;
        tem<<"Video"<<"Command"<<"burst";
        ui->cbb_0->clear();
        ui->cbb_0->addItems(tem);
        tem.clear();
        tem<<"1lane"<<"2lane"<<"3lane"<<"4lane";
        ui->cbb_1->clear();
        ui->cbb_1->addItems(tem);
        tem.clear();
        tem<<"rgb888"<<"rgb666"<<"rgb666Package"<<"rgb565";
        ui->cbb_2->clear();
        ui->cbb_2->addItems(tem);
        area_mipi_fill();
    }else if(model_type == 1){
        ui->text_0->clear();
        ui->text_0->setText("Pixel Num");
        ui->text_1->clear();
        ui->text_1->setText("Data_Bit");
        ui->text_2->clear();
        ui->text_2->setText("Config");

        QStringList tem;
        tem<<"Singal"<<"Double";
        ui->cbb_0->clear();
        ui->cbb_0->addItems(tem);
        tem.clear();
        tem<<"8bit"<<"6bit";
        ui->cbb_1->clear();
        ui->cbb_1->addItems(tem);
        tem.clear();
        tem<<"NS"<<"JEIDA";
        ui->cbb_2->clear();
        ui->cbb_2->addItems(tem);
        area_lvds_fill();
    }else if(model_type == 0){
        ui->text_0->clear();
        ui->text_0->setText("RGB Model");
        ui->text_1->clear();
        ui->text_1->setText("Sync Pol");
        ui->text_2->clear();
        ui->text_2->setText("Data_Bit");

        QStringList tem;
        tem<<"Parallel"<<"Serial"<<"DummyRGB"<<"RGBDummy"<<"YUV";
        ui->cbb_0->clear();
        ui->cbb_0->addItems(tem);
        tem.clear();
        tem<<"V-H-"<<"V+H-"<<"V-H+"<<"V+H+";
        ui->cbb_1->clear();
        ui->cbb_1->addItems(tem);
        tem.clear();
        tem<<"disable"<<"enable rgb666"<<"enable rgb656";
        ui->cbb_2->clear();
        ui->cbb_2->addItems(tem);
        area_rgb_fill();
    }
}

void MainWindow::area_mipi_fill(){
    ui->cbb_0->setCurrentIndex(-1);
    ui->cbb_1->setCurrentIndex(-1);
    ui->cbb_2->setCurrentIndex(-1);
}

void MainWindow::area_mipi_fill(int cbb_0_value , int cbb_1_value , int cbb_2_value){
    ui->cbb_0->setCurrentIndex(cbb_0_value);
    ui->cbb_1->setCurrentIndex(cbb_1_value);
    ui->cbb_2->setCurrentIndex(cbb_2_value);
}

void MainWindow::area_lvds_fill(){
    ui->cbb_0->setCurrentIndex(-1);
    ui->cbb_1->setCurrentIndex(-1);
    ui->cbb_2->setCurrentIndex(-1);
}

void MainWindow::area_lvds_fill(int cbb_0_value , int cbb_1_value , int cbb_2_value){
    ui->cbb_0->setCurrentIndex(cbb_0_value);
    ui->cbb_1->setCurrentIndex(cbb_1_value);
    ui->cbb_2->setCurrentIndex(cbb_2_value);
}

void MainWindow::area_rgb_fill(){
    ui->cbb_0->setCurrentIndex(-1);
    ui->cbb_1->setCurrentIndex(-1);
    ui->cbb_2->setCurrentIndex(-1);
}

void MainWindow::area_rgb_fill(int cbb_0_value, int cbb_1_value, int cbb_2_value){
    ui->cbb_0->setCurrentIndex(cbb_0_value);
    ui->cbb_1->setCurrentIndex(cbb_1_value);
    ui->cbb_2->setCurrentIndex(cbb_2_value);
}

void MainWindow::set_all_value(){
    int cnt = all_value_widget.size();
    for(int i = 0 ; i < cnt ; ++ i){
        if(QString(all_value_widget[i]->metaObject()->className())=="QComboBox"){
            QComboBox *it = (QComboBox*)all_value_widget[i];
            if(load_file_data[i]!="-1" &&load_file_data[i]!="")it->setCurrentIndex(load_file_data[i].toInt());
        }else if(QString(all_value_widget[i]->metaObject()->className())=="QLineEdit"){
            QLineEdit *it = (QLineEdit*)all_value_widget[i];
            if(load_file_data[i]!="-1" &&load_file_data[i]!="")it->setText(load_file_data[i]);
        }else if(QString(all_value_widget[i]->metaObject()->className())=="QTextEdit"){
            if(load_file_data[i]=="-1"){

            }
            else {
                if(QFile::exists(load_file_data[i])){
                    initial_code_file_dir = load_file_data[i];
                    QFile f(initial_code_file_dir);
                    if(f.open(QIODevice::ReadOnly|QIODevice::Text)){
                        QString str = f.readAll();
                        init_code.clear();
                        init_code = str.split("\n");
                        for(QString s:init_code){
                            s.replace(" ","");
                            if(s[0]=='R' || s[0] == 'r'){
                                s=s.toUpper();
                            }
                            else {
                                s=s.toLower();
                            }
                            ui->ti_initial_code->append(s);
                        }
                        str.clear();



                       f.close();
                    }
                }else{
                    initial_code_file_dir = "-1";
                }
            }
        }
    }
}

void MainWindow::get_all_value(){
    int cnt = all_value_widget.size();
    save_file_data.clear();
    for(int i = 0 ; i  < 3 ; ++ i){
        if(QString(all_value_widget[i]->metaObject()->className())=="QComboBox"){
            QComboBox *it = (QComboBox*)all_value_widget[i];
            save_file_data.append(QString::number(it->currentIndex()));
        }else if(QString(all_value_widget[i]->metaObject()->className())=="QLineEdit"){
            QLineEdit *it = (QLineEdit*)all_value_widget[i];
            QString tem = it->text();
            if(tem.isEmpty()){
                save_file_data.append("-1");
            }else{
                save_file_data.append(tem);
            }

        }else if(QString(all_value_widget[i]->metaObject()->className())=="QTextEdit"){
            init_code.clear();
            QString str = ui->ti_initial_code->toPlainText();
            //qDebug()<<"initial code is:"<<str;
            if(str!="")init_code = str.split("\n");
            else initial_code_file_dir = "-1";
            save_file_data.append(initial_code_file_dir);

        }
    }
    int mode_type = ui->cbb_model_select->currentIndex();
    if(mode_type==-1){
        for(int i = 0; i < 9; ++i)save_file_data.append("-1");
    }else{
        for(int i = 0 ; i < 6-mode_type*3 ;++i){
            save_file_data.append("-1");
        }
        save_file_data.append(QString::number(ui->cbb_0->currentIndex()));
        save_file_data.append(QString::number(ui->cbb_1->currentIndex()));
        save_file_data.append(QString::number(ui->cbb_2->currentIndex()));
        for(int i = 0 ; i < mode_type*3;++i)save_file_data.append("-1");
    }
    for(int i = 12 ; i< cnt ; ++ i){
        //
        qDebug()<<"now get value:"<<i;
        if(QString(all_value_widget[i]->metaObject()->className())=="QComboBox"){
            QComboBox *it = (QComboBox*)all_value_widget[i];
            save_file_data.append(QString::number(it->currentIndex()));
        }else if(QString(all_value_widget[i]->metaObject()->className())=="QLineEdit"){
            QLineEdit *it = (QLineEdit*)all_value_widget[i];
            QString tem = it->text();
            if(tem.isEmpty()){
                save_file_data.append("-1");
            }else{
                save_file_data.append(tem);
            }

        }else if(QString(all_value_widget[i]->metaObject()->className())=="QTextEdit"){
            init_code.clear();
            QString str = ui->ti_initial_code->toPlainText();
            //qDebug()<<"initial code is:"<<str;
            if(str!="")init_code = str.split("\n");
            else initial_code_file_dir = "-1";
            save_file_data.append(initial_code_file_dir);

        }
    }
}

void MainWindow::get_initial_code(){
    QString str = ui->ti_initial_code->toPlainText();
    init_code.clear();
    init_code = str.split("\n");
    for(int i = 0 ; i < init_code.size() ; ++ i){
        init_code[i].replace(" ","");
        if(init_code[i].at(0)=='R' ||init_code[i].at(0) == 'r'){
            init_code[i]=init_code[i].toUpper();
        }else{
            init_code[i]=init_code[i].toLower();
        }
    }
}

void MainWindow::set_initial_code(){
    ui->ti_initial_code->clear();
    for(int i = 0 ; i < init_code.size() ; ++ i){
        ui->ti_initial_code->append(init_code[i]);
    }
}

void MainWindow::generate_display(){
    display_cfg_file_data.clear();
    
    if(ui->cbb_model_select->currentIndex()!=-1){
        display_cfg_file_data.append("[disp_param]");
        if(ui->cbb_model_select->currentIndex() == 0){
            display_cfg_file_data.append("lcd_if=0;");
        }else if(ui->cbb_model_select->currentIndex() == 1){
            display_cfg_file_data.append("lcd_if=3;");
        }else if(ui->cbb_model_select->currentIndex() == 2){
            display_cfg_file_data.append("lcd_if=4;");
        }
        if(ui->le_ver_display->text()!=""){
            display_cfg_file_data.append("lcd_x="+ui->le_hor_display->text()+";");
        }else{
            display_cfg_file_data.append("lcd_x=0;");
        }
        if(ui->le_hor_display->text()!=""){
            display_cfg_file_data.append("lcd_y="+ui->le_ver_display->text()+";");
        }else{
            display_cfg_file_data.append("lcd_y=0;");
        }
        if(ui->le_clk->text()!=""){
            display_cfg_file_data.append("lcd_dckl_freq="+ui->le_clk->text()+";");
        }else{
            display_cfg_file_data.append("lcd_dckl_freq=0;");
        }
        if(ui->le_ver_total_line->text()!=""){
            display_cfg_file_data.append("lcd_vt="+ui->le_ver_total_line->text()+";");
        }else{
             display_cfg_file_data.append("lcd_vt=0;");
        }
        if(ui->le_ver_back_porch->text()!=""){
            display_cfg_file_data.append("lcd_vbp="+ui->le_ver_back_porch->text()+";");
        }else{
            display_cfg_file_data.append("lcd_vbp=0;");
        }
        if(ui->le_ver_sync_width->text()!=""){
            display_cfg_file_data.append("lcd_vspw="+ui->le_ver_sync_width->text()+";");
        }else{
            display_cfg_file_data.append("lcd_vspw=0;");
        }
        if(ui->le_hor_total_line->text()!=""){
            display_cfg_file_data.append("lcd_ht="+ui->le_hor_total_line->text()+";");
        }else{
            display_cfg_file_data.append("lcd_ht=0;");
        }
        if(ui->le_hor_back_porch->text()!=""){
            display_cfg_file_data.append("lcd_hbp="+ui->le_hor_back_porch->text()+";");
        }else{
            display_cfg_file_data.append("lcd_hbp=0;");
        }
        if(ui->le_hor_sync_width->text()!=""){
            display_cfg_file_data.append("lcd_hspw="+ui->le_hor_sync_width->text()+";");
        }else{
            display_cfg_file_data.append("lcd_hspw=0;");
        }
        if(ui->cbb_model_select->currentIndex() == 0){
            display_cfg_file_data.append("lcd_lvds_if=0;");
            display_cfg_file_data.append("lcd_lvdscolordepth=0;");
            display_cfg_file_data.append("lcd_lvds_mode=0;");
            if(ui->cbb_0->currentIndex()==0){
                display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=0;");
                display_cfg_file_data.append("lcd_hv_sync_polarity=0;");
            }else if(ui->cbb_0->currentIndex()==1){
                display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=8;");
                display_cfg_file_data.append("lcd_hv_sync_polarity=1;");
            }else if(ui->cbb_0->currentIndex()==2){
                display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=10;");
                display_cfg_file_data.append("lcd_hv_sync_polarity=2;");
            }else if(ui->cbb_0->currentIndex()==3){
                display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=11;");
                display_cfg_file_data.append("lcd_hv_sync_polarity=3;");
            }else if(ui->cbb_0->currentIndex()==1){
                display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=12;");
                display_cfg_file_data.append("lcd_hv_sync_polarity=4;");
            }else{
                display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=0;");
                display_cfg_file_data.append("lcd_hv_sync_polarity=0;");
            }
            if(ui->cbb_2->currentIndex()==0||ui->cbb_2->currentIndex()==-1){
                display_cfg_file_data.append("lcd_frm=0;");
            }else if(ui->cbb_2->currentIndex()==1){
                display_cfg_file_data.append("lcd_frm=1;");
            }else{
                display_cfg_file_data.append("lcd_frm=2;");
            }
            display_cfg_file_data.append("lcd_dsi_if=0;");
            display_cfg_file_data.append("lcd_dsi_lane=0;");
            display_cfg_file_data.append("lcd_dsi_format=0;");
        }else if(ui->cbb_model_select->currentIndex()==1){
            if(ui->cbb_0->currentIndex()==0 || ui->cbb_0->currentIndex()==-1){
                display_cfg_file_data.append("lcd_lvds_if=0;");
            }else{
                display_cfg_file_data.append("lcd_lvds_if=1;");
            }
            if(ui->cbb_1->currentIndex()==0|| ui->cbb_0->currentIndex()==-1){
                display_cfg_file_data.append("lcd_lvds_if=0;");
            }else{
                display_cfg_file_data.append("lcd_lvds_if=1;");
            }
            if(ui->cbb_2->currentIndex()==0||ui->cbb_2->currentIndex()==-1){
                display_cfg_file_data.append("lcd_lvds_mode=0;");
            }else{
                display_cfg_file_data.append("lcd_lvds_mode=1;");
            }
            display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=0;");
            display_cfg_file_data.append("lcd_hv_sync_polarity=0;");
            display_cfg_file_data.append("lcd_frm=0;");
            display_cfg_file_data.append("lcd_dsi_if=0;");
            display_cfg_file_data.append("lcd_dsi_lane=0;");
            display_cfg_file_data.append("lcd_dsi_format=0;");
        }else{
            display_cfg_file_data.append("lcd_lvds_if=0;");
            display_cfg_file_data.append("lcd_lvdscolordepth=0;");
            display_cfg_file_data.append("lcd_lvds_mode=0;");
            display_cfg_file_data.append("lcd_lvlcd_hv_ifds_mode=0;");
            display_cfg_file_data.append("lcd_hv_sync_polarity=0;");
            display_cfg_file_data.append("lcd_frm=0;");
            if(ui->cbb_0->currentIndex()==0 || ui->cbb_0->currentIndex()==-1){
                display_cfg_file_data.append("lcd_dsi_if=0;");
            }else if(ui->cbb_0->currentIndex() == 1){
                display_cfg_file_data.append("lcd_dsi_if=1;");
            }else{
                display_cfg_file_data.append("lcd_dsi_if=2;");
            }
            display_cfg_file_data.append("lcd_dsi_lane="+QString::number(ui->cbb_1->currentIndex()+1) + ";");
            if(ui->cbb_2->currentIndex()==0||ui->cbb_2->currentIndex()==-1){
                display_cfg_file_data.append("lcd_dsi_format=0;");
            }else{
                display_cfg_file_data.append("lcd_lvds_mode="+QString::number(ui->cbb_2->currentIndex()) + ";");
            }
        }
        if(ui->cbb_pwm_pwm->currentIndex()!=-1){
            if(ui->cbb_pwm_pwm->currentIndex()==0)display_cfg_file_data.append("lcd_pwm_used=1;");
            else{
                display_cfg_file_data.append("lcd_pwm_used=0;");
            }
        }
        if(ui->le_pwm_freq->text()!=""){
            bool ok;
            int k = ui->le_pwm_freq->text().toInt(&ok);
            if(ok)display_cfg_file_data.append("lcd_pwm_freq="+ui->le_pwm_freq->text()+";");
        }
        if(ui->cbb_pwm_pol->currentIndex()!=-1){
            display_cfg_file_data.append("lcd_pwm_pol="+QString::number(ui->cbb_pwm_pol->currentIndex()));
        }
        display_cfg_file_data.append("[disp_power]");
        display_cfg_file_data.append(display_timeseq_file_data);
        if(ui->ti_initial_code->toPlainText()!=""){
            display_cfg_file_data.append("[disp_init]");
            display_cfg_file_data.append(disp_init_cfg_file_data);
        }
    }else{

    }
    
    
}

void MainWindow::generate_disp_init(){
    disp_init_cfg_file_data.clear();
    QString str = ui->ti_initial_code->toPlainText();
    str = str.replace(" ","");
    disp_init_cfg_file_data = str.split("\n");
    qDebug()<<"initial code size:"<<disp_init_cfg_file_data.size();
    //qDebug()<<"intial code:"<<str;
    for(QString s:disp_init_cfg_file_data){

        if(s[0] == 'R' || s[0] == 'r'){
            s = s.toUpper();
        }else{
            s = s.toLower();
        }
    }
}

void MainWindow::generate_display_timeseq(){
    display_timeseq_file_data.clear();
    int cnt = all_value_widget.size();
    int ind = 0;
    for(int i = cnt-40 ; i <cnt ;i+=2){
        ind++;
        QComboBox *it = (QComboBox*)all_value_widget[i];
        QLineEdit *it1 = (QLineEdit*)all_value_widget[i+1];
        if(it->currentIndex()!=-1){
            display_timeseq_file_data.append(QString::number(it->currentIndex(),16).toUpper());
            if(it1->text()!="")display_timeseq_file_data.append(it1->text());
            else display_timeseq_file_data.append("0");
        }else{
            display_timeseq_file_data.append("R");
            display_timeseq_file_data.append("0");
        }
        
    }
    qDebug()<<"timeseq file:"<<display_timeseq_file_data.size();
}

void MainWindow::save_config_to_temp_file(){

    generate_disp_init();
    generate_display_timeseq();
    generate_display();
    QString path1="./disp_param/display.cfg";
    QString path2="./disp_param/disp_init.cfg";
    QString path3="./disp_param/display_timeseq.cfg";
    QFile f1(path1),f2(path2),f3(path3);
    f1.open(QIODevice::WriteOnly|QIODevice::Text);
    f2.open(QIODevice::WriteOnly|QIODevice::Text);
    f3.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream in1(&f1);
    for(auto s:display_cfg_file_data ){
        in1<<s<<"\n";
    }
    QTextStream in2(&f2);
    for(auto s:disp_init_cfg_file_data){
        in2<<s<<"\n";
    }
    QTextStream in3(&f3);
    for(auto s:display_timeseq_file_data ){
        in3<<s<<"\n";
    }
    f1.close();
    f2.close();
    f3.close();
}

void MainWindow::initial_adb_process(){

}

int MainWindow::adb_get_producter(){
    QStringList args;
    //args.append("adb");
    args.append("shell");
    args.append("getprop");
    args.append("ro.product.brand");
    qDebug()<<"start get producter";
    adb_process->start(command,args);
    bool f = adb_process->waitForReadyRead(3000);
    if(!f)return 1;
    //QString res = QString::fromLocal8Bit(adb_process->readAllStandardOutput());
    if(recv_data.mid(0,9) != "Allwinner"){
        return 2;
    }
    adb_process->waitForFinished(1000);
    qDebug()<<"get producter sucess:"<<recv_data;
    return 0;
}

int MainWindow::adb_get_product_model(){
    QStringList args;
    args.append("shell");
    args.append("getprop");
    args.append("ro.product.model");
    adb_process->start(command,args);
    bool f = adb_process->waitForReadyRead(3000);
    if(!f)return 1;
    //QString res = QString::fromLocal8Bit(adb_process->readAllStandardOutput());
    if(recv_data.mid(0,11) != "SINLINX A83"){
        return 2;
    }
    qDebug()<<"get product model sucess!";
    adb_process->waitForFinished(1000);
    return 0;
}

void MainWindow::on_readoutput(){
    recv_data.clear();
    recv_data = QString::fromLocal8Bit(adb_process->readAllStandardOutput());
    //recv_data.replace(" ","");
    qDebug()<<"recv data:"<<recv_data<<"\n";
}

void MainWindow::on_readerror(){
    qDebug()<<"read error:"<<QString::fromLocal8Bit(adb_process->readAllStandardError());
}

bool MainWindow::check_adb_file(){
    QString f1 = QDir::currentPath()+"/adb/adb.exe";
    QString f2 = QDir::currentPath()+"/adb/AdbWinApi.dll";
    QString f3 = QDir::currentPath()+"/adb/AdbWinUsbApi.dll";
    qDebug()<<f1<<"\n"<<f2<<"\n"<<f3;
    if(QFile::exists(f1) && QFile::exists(f2) && QFile::exists(f3))return true;
    qDebug()<<"adb file not exist";
    return false;
}

void MainWindow::check_disp_param_dir(){
    QDir dir(QDir::currentPath()+"/disp_param");

    if(!dir.exists()){
        bool ismkdir = dir.mkdir(QDir::currentPath()+"/disp_param");
        if(ismkdir)qDebug()<<"create disp param path sucess";
        else qDebug()<<"create disp param path failed";
    }

}

void MainWindow::all_button_enable(){
    qDebug()<<"open all button";
    for(int i = 0 ; i < all_button.size() ; ++ i){
        QPushButton *it = all_button[i];
        it->setEnabled(true);
    }
    qDebug()<<"open all button sucess";
}

void MainWindow::all_button_unenable(){
    qDebug()<<"close all button";
    for(int i = 0 ; i < all_button.size() ; ++ i){
        all_button[i]->setEnabled(false);
    }
    //ui->bu_load->setEnabled(false);
    qDebug()<<"close all button sucess";
}

void MainWindow::all_value_widget_initial(){
    all_value_widget.append(ui->le_model);
    all_value_widget.append(ui->cbb_model_select);
    all_value_widget.append(ui->le_clk);
    all_value_widget.append(ui->cbb_0);
    all_value_widget.append(ui->cbb_1);
    all_value_widget.append(ui->cbb_2);
    all_value_widget.append(ui->cbb_0);
    all_value_widget.append(ui->cbb_1);
    all_value_widget.append(ui->cbb_2);
    all_value_widget.append(ui->cbb_0);
    all_value_widget.append(ui->cbb_1);
    all_value_widget.append(ui->cbb_2);
    all_value_widget.append(ui->cbb_pwm_pwm);
    all_value_widget.append(ui->le_pwm_freq);
    all_value_widget.append(ui->cbb_pwm_pol);
    all_value_widget.append(ui->ti_initial_code);
    all_value_widget.append(ui->le_hor_total_line);
    all_value_widget.append(ui->le_hor_display);
    all_value_widget.append(ui->le_hor_back_porch);
    all_value_widget.append(ui->le_hor_sync_width);
    all_value_widget.append(ui->le_ver_total_line);
    all_value_widget.append(ui->le_ver_display);
    all_value_widget.append(ui->le_ver_back_porch);
    all_value_widget.append(ui->le_ver_sync_width);
    all_value_widget.append(ui->cbb_on_0);
    all_value_widget.append(ui->le_on_0);
    all_value_widget.append(ui->cbb_on_1);
    all_value_widget.append(ui->le_on_1);
    all_value_widget.append(ui->cbb_on_2);
    all_value_widget.append(ui->le_on_2);
    all_value_widget.append(ui->cbb_on_3);
    all_value_widget.append(ui->le_on_3);
    all_value_widget.append(ui->cbb_on_4);
    all_value_widget.append(ui->le_on_4);
    all_value_widget.append(ui->cbb_on_5);
    all_value_widget.append(ui->le_on_5);
    all_value_widget.append(ui->cbb_on_6);
    all_value_widget.append(ui->le_on_6);
    all_value_widget.append(ui->cbb_on_7);
    all_value_widget.append(ui->le_on_7);
    all_value_widget.append(ui->cbb_on_8);
    all_value_widget.append(ui->le_on_8);
    all_value_widget.append(ui->cbb_on_9);
    all_value_widget.append(ui->le_on_9);
    all_value_widget.append(ui->cbb_off_0);
    all_value_widget.append(ui->le_off_0);
    all_value_widget.append(ui->cbb_off_1);
    all_value_widget.append(ui->le_off_1);
    all_value_widget.append(ui->cbb_off_2);
    all_value_widget.append(ui->le_off_2);
    all_value_widget.append(ui->cbb_off_3);
    all_value_widget.append(ui->le_off_3);
    all_value_widget.append(ui->cbb_off_4);
    all_value_widget.append(ui->le_off_4);
    all_value_widget.append(ui->cbb_off_5);
    all_value_widget.append(ui->le_off_5);
    all_value_widget.append(ui->cbb_off_6);
    all_value_widget.append(ui->le_off_6);
    all_value_widget.append(ui->cbb_off_7);
    all_value_widget.append(ui->le_off_7);
    all_value_widget.append(ui->cbb_off_8);
    all_value_widget.append(ui->le_off_8);
    all_value_widget.append(ui->cbb_off_9);
    all_value_widget.append(ui->le_off_9);
    all_button.append(ui->bu_connect);
    all_button.append(ui->bu_update);
    all_button.append(ui->bu_download);
    all_button.append(ui->bu_save);
    all_button.append(ui->bu_load);
    all_button.append(ui->bu_restart);
    all_button.append(ui->bu_load_init_code);
    all_button.append(ui->bu_android_on);
    all_button.append(ui->bu_android_off);
    all_button.append(ui->bu_image_next);
    all_button.append(ui->bu_image_prev);
    qDebug()<<"all button init :"<<all_button.size()<<" all value widget size:"<<all_value_widget.size();
}

void MainWindow::on_bu_load_init_code_clicked(){
    all_button_unenable();
    QString load_file_name = QFileDialog::getOpenFileName(this,
                                                          tr("Open File"),
                                                          "./disp_param/",
                                                          tr("Config File(*.cfg)")
                                                          );
    if(load_file_name.isNull()){
        all_button_enable();
        return ;
    }
    QFile f(load_file_name);
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text)){
        all_button_enable();
        change_text_connect_info(7,0);
        return ;
    }
    initial_code_file_dir = load_file_name;
    ui->ti_initial_code->setPlainText(f.readAll());
    all_button_enable();
    change_text_connect_info(7,1);
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
    case Qt::Key_F4:
        download_image_to_device();
        break;
    case Qt::Key_F5:
        clear_all_value();
        break;
    case Qt::Key_F6:
        clear_gpio_set();
        break;
    default:
        break;
    }
}

void MainWindow::download_image_to_device(){
    all_button_unenable();
    QString path = QDir::currentPath();
    QDir dir(path+"/imageplay/");
    if(!dir.exists()){
        change_text_connect_info(8,0);
        all_button_enable();
        return ;
    }
    QStringList nameFliters;
    nameFliters<<"*.jpg"<<"*.png"<<"*.bmp"<<"*.txt";
    QStringList files = dir.entryList(nameFliters,QDir::Files|QDir::Readable,QDir::Name);
    qDebug()<<"image play files path:"<<path;
    qDebug()<<"image play files size:"<<files.size();
    int step1 = adb_get_producter();
    if(step1!=0){
        change_text_connect_info(8,1);
        all_button_enable();
        return ;
    }
    int step2 = adb_get_product_model();
    if(step2!=0){
        change_text_connect_info(8,1);
        all_button_enable();
        return ;
    }
    QStringList args;
    args.append("shell");
    args.append("rm");
    args.append("-r");
    args.append("/mnt/sdcard/Pictures/imagePlay");
    adb_process->start(command,args);
    adb_process->waitForFinished();
    args.clear();
    args.append("shell");
    args.append("mkdir");
    args.append("/mnt/sdcard/Pictures/imagePlay");
    adb_process->start(command,args);
    adb_process->waitForFinished();
    args.clear();
    for(QString s:files){
        args.append("push");
        args.append(path+"/imageplay/"+s);
        args.append("/mnt/sdcard/Pictures/imagePlay/");
        adb_process->start(command,args);

        adb_process->waitForFinished();
        args.clear();
    }
    change_text_connect_info(8,2);
    all_button_enable();
}

void MainWindow::clear_all_value(){
    all_button_unenable();
    init_area_singnal_model();
    ui->ti_initial_code->clear();
    initial_code_file_dir.clear();
    for(int i = 0 ; i < all_value_widget.size() ;++ i){
        if(QString(all_value_widget[i]->metaObject()->className())=="QLineEdit"){
            QLineEdit *it = (QLineEdit*)all_value_widget[i];
            it->setText("");

        }else if(QString(all_value_widget[i]->metaObject()->className())=="QComboBox"){
            QComboBox *it = (QComboBox*)all_value_widget[i];
            it->setCurrentIndex(-1);

        }
    }
    all_button_enable();
}

void MainWindow::clear_gpio_set(){
    all_button_unenable();
    int cnt = all_value_widget.size();
    for(int i = cnt -40; i<cnt ;i+=2){
        QComboBox *it1 = (QComboBox*)all_value_widget[i];
        QLineEdit *it2 = (QLineEdit*)all_value_widget[i+1];
        it1->setCurrentIndex(-1);
        it2->setText("");
    }
    all_button_enable();
}

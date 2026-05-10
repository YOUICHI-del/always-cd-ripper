#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QProgressBar>
#include <QTimer>
#include <QLineEdit>
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include "../ripper/CdDrive.h"
#include "../ripper/ParanoiaReader.h"
#include "../ripper/FlacEncoder.h"
#include "../ripper/RipWorker.h"
#include "../metadata/MusicBrainz.h"
#include "SectorMap.h"
#include "LogViewer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNext();
    void onBack();
    void onRipClicked();
    void onRipFinished();
    void onBrowseOutput();
    void onSettingsClicked();
    void onMetadataReady(QVector<MbRelease> releases);

private:
    void setupUi();
    void applyStyle();
    void goToStep(int step);
    void updateStepIndicator();
    void startVerify();

    // ページビルダー
    QWidget *buildStep0_Source();
    QWidget *buildStep1_Output();
    QWidget *buildStep2_Metadata();
    QWidget *buildStep3_Rip();
    QWidget *buildStep4_Verify();

    // ステップインジケーター
    QWidget     *m_stepBar      = nullptr;
    QLabel      *m_stepLabels[5] = {};

    // ナビボタン
    QPushButton *m_backBtn      = nullptr;
    QPushButton *m_nextBtn      = nullptr;

    // スタック
    QStackedWidget *m_stack     = nullptr;
    int             m_step      = 0;

    // Step0: ソース選択
    QComboBox   *m_driveCombo     = nullptr;
    QLineEdit   *m_cuePath        = nullptr;
    QLabel      *m_driveModel     = nullptr;
    QLabel      *m_driveOffset    = nullptr;
    QLabel      *m_driveStatus    = nullptr;

    // Step1: 出力設定
    QLineEdit   *m_outputPath   = nullptr;
    QComboBox   *m_formatCombo  = nullptr;

    // Step2: メタデータ
    QLabel      *m_artLabel     = nullptr;
    QLineEdit   *m_edAlbum      = nullptr;
    QLineEdit   *m_edArtist     = nullptr;
    QLineEdit   *m_edYear       = nullptr;
    QTableWidget *m_trackTable  = nullptr;

    // Step3: RIP
    SectorMap   *m_sectorMap    = nullptr;
    QProgressBar *m_progress    = nullptr;
    QLabel      *m_progressLabel= nullptr;
    QWidget     *m_statPurity   = nullptr;
    QWidget     *m_statRetries  = nullptr;
    QWidget     *m_statErrors   = nullptr;
    QWidget     *m_statSpeed    = nullptr;
    LogViewer   *m_logViewer    = nullptr;
    QPushButton *m_ripBtn       = nullptr;

    // Step4: ベリファイ
    QLabel       *m_verifyResult   = nullptr;
    QLabel       *m_verifyDetail   = nullptr;
    QProgressBar *m_verifyProgress = nullptr;
    QTableWidget *m_verifyTable    = nullptr;

    // コア
    CdDrive          m_drive;
    DiscInfo         m_discInfo;
    ParanoiaReader  *m_reader    = nullptr;
    FlacEncoder      m_encoder;
    MusicBrainz     *m_mb        = nullptr;

    // ダミーRIP用
    QTimer *m_ripTimer     = nullptr;
    int     m_ripCounter   = 0;
    int     m_ripTotal     = 0;
    int     m_totalErrors  = 0;
    int     m_totalRetries = 0;

    QWidget *buildStatCard(const QString &label, const QString &value);
};

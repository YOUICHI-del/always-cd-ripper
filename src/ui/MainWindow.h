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
#include "../metadata/GnuDb.h"
#include "../metadata/CoverArt.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
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

private:
    void setupUi();
    void applyStyle();
    void goToStep(int step);
    void updateStepIndicator();
    void startVerify();
    QString calcDiscId(const DiscInfo &info);

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
    QLineEdit   *m_wavPath        = nullptr;

    // Step1: 出力設定
    QLineEdit   *m_outputPath   = nullptr;
    QComboBox   *m_formatCombo  = nullptr;

    // Step2: メタデータ
    QLabel      *m_artLabel     = nullptr;
    QLineEdit   *m_edAlbum      = nullptr;
    QLineEdit   *m_edArtist     = nullptr;
    QLineEdit   *m_edYear       = nullptr;
    QSpinBox    *m_discNumberSpin = nullptr;
    QSpinBox    *m_discTotalSpin  = nullptr;
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
    CoverArt         m_cover;
    CdDrive          m_drive;
    DiscInfo         m_discInfo;
    MbRelease        m_mbRelease;   // 複数DISC情報保持用
    QNetworkAccessManager *m_nam = nullptr;
    QPixmap          m_coverArt;
    ParanoiaReader  *m_reader    = nullptr;
    FlacEncoder      m_encoder;

    // ダミーRIP用
    QTimer *m_ripTimer     = nullptr;
    int     m_ripCounter   = 0;
    int     m_ripTotal     = 0;
    int     m_totalErrors  = 0;
    int     m_totalRetries = 0;

    // リッピング速度（KB/s）デフォルト2x
    int     m_readSpeed    = CdDrive::SPEED_2X;

    QWidget *buildStatCard(const QString &label, const QString &value);
};

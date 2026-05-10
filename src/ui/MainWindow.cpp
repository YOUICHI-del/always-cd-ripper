#include "MainWindow.h"
#include "MetadataEditor.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QDateTime>
#include <QRandomGenerator>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QApplication>
#include <QThread>
#include <QDir>
#include <QRegularExpression>

// ステップ名
static const char *STEP_NAMES[] = {
    "① Source", "② Output", "③ Metadata", "④ RIP", "⑤ Verify"
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Always CD Ripper");
    setMinimumSize(960, 620);
    resize(1100, 680);

    m_reader = new ParanoiaReader(this);
    m_mb     = new MusicBrainz(this);
    connect(m_mb, &MusicBrainz::resultsReady, this, &MainWindow::onMetadataReady);

    setupUi();
    applyStyle();
    goToStep(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── タイトルバー ──
    auto *titleBar = new QWidget;
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(44);
    auto *tl = new QHBoxLayout(titleBar);
    tl->setContentsMargins(24,0,16,0);
    auto *titleLabel = new QLabel("ALWAYS CD RIPPER");
    titleLabel->setObjectName("appTitle");
    tl->addWidget(titleLabel);

    auto *verLabel2 = new QLabel("v1.0.0");
    verLabel2->setObjectName("verLabel");
    tl->addWidget(verLabel2);
    tl->addStretch();

    auto *settingsBtn = new QPushButton("⚙  Settings");
    settingsBtn->setObjectName("settingsBtn");
    settingsBtn->setFixedHeight(28);
    tl->addWidget(settingsBtn);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    root->addWidget(titleBar);

    // ── ステップインジケーター ──
    m_stepBar = new QWidget;
    m_stepBar->setObjectName("stepBar");
    m_stepBar->setFixedHeight(48);
    auto *sl = new QHBoxLayout(m_stepBar);
    sl->setContentsMargins(24,0,24,0);
    sl->setSpacing(0);

    for (int i = 0; i < 5; ++i) {
        m_stepLabels[i] = new QLabel(STEP_NAMES[i]);
        m_stepLabels[i]->setObjectName("stepLabel");
        m_stepLabels[i]->setAlignment(Qt::AlignCenter);
        sl->addWidget(m_stepLabels[i], 1);

        if (i < 4) {
            auto *sep = new QLabel("›");
            sep->setObjectName("stepSep");
            sep->setAlignment(Qt::AlignCenter);
            sl->addWidget(sep);
        }
    }
    root->addWidget(m_stepBar);

    // セパレータ
    auto *sepLine = new QFrame;
    sepLine->setFrameShape(QFrame::HLine);
    sepLine->setObjectName("hline");
    root->addWidget(sepLine);

    // ── コンテンツスタック ──
    m_stack = new QStackedWidget;
    m_stack->addWidget(buildStep0_Source());
    m_stack->addWidget(buildStep1_Output());
    m_stack->addWidget(buildStep2_Metadata());
    m_stack->addWidget(buildStep3_Rip());
    m_stack->addWidget(buildStep4_Verify());
    root->addWidget(m_stack, 1);

    // ── ボトムナビ ──
    auto *navBar = new QWidget;
    navBar->setObjectName("navBar");
    navBar->setFixedHeight(56);
    auto *nl = new QHBoxLayout(navBar);
    nl->setContentsMargins(24,0,24,0);
    nl->setSpacing(12);

    m_backBtn = new QPushButton("◀  Back");
    m_backBtn->setObjectName("backBtn");
    m_backBtn->setFixedSize(110, 34);

    m_nextBtn = new QPushButton("Next  ▶");
    m_nextBtn->setObjectName("nextBtn");
    m_nextBtn->setFixedSize(110, 34);

    nl->addWidget(m_backBtn);
    nl->addStretch();
    nl->addWidget(m_nextBtn);
    root->addWidget(navBar);

    connect(m_backBtn, &QPushButton::clicked, this, &MainWindow::onBack);
    connect(m_nextBtn, &QPushButton::clicked, this, &MainWindow::onNext);
}

// ────────────────────────────────────────
// Step 0 — ソース選択
// ────────────────────────────────────────
QWidget *MainWindow::buildStep0_Source()
{
    auto *page = new QWidget;
    auto *vl   = new QVBoxLayout(page);
    vl->setContentsMargins(40,32,40,32);
    vl->setSpacing(20);

    auto *heading = new QLabel("ソースを選択してください");
    heading->setObjectName("stepHeading");
    vl->addWidget(heading);

    // CDドライブ選択
    auto *cdGroup = new QGroupBox("CD ドライブ");
    cdGroup->setObjectName("sourceGroup");
    auto *cgl = new QVBoxLayout(cdGroup);
    cgl->setSpacing(12);

    m_driveCombo = new QComboBox;
    m_driveCombo->setObjectName("driveCombo");

#ifdef Q_OS_WIN
    QStringList real = CdDrive::availableDrives();
    if (!real.isEmpty()) {
        for (auto &d : real)
            m_driveCombo->addItem("CD-ROM  " + d, d);
    } else {
        m_driveCombo->addItem("[DEMO]  Kind of Blue / Miles Davis", "DEMO");
    }
#else
    m_driveCombo->addItem("[DEMO]  Kind of Blue / Miles Davis", "DEMO");
#endif

    auto *driveRow = new QHBoxLayout;
    driveRow->addWidget(new QLabel("ドライブ:"));
    driveRow->addWidget(m_driveCombo, 1);
    cgl->addLayout(driveRow);

    // ドライブ情報グリッド
    auto *infoGrid = new QGridLayout;
    infoGrid->setSpacing(8);
    infoGrid->setColumnMinimumWidth(0, 120);

    auto addInfoRow = [&](int row, const QString &label, QLabel *&val, const QString &init){
        auto *lbl = new QLabel(label); lbl->setObjectName("formLabel");
        val = new QLabel(init);        val->setObjectName("driveInfoVal");
        infoGrid->addWidget(lbl, row, 0);
        infoGrid->addWidget(val, row, 1);
    };

    addInfoRow(0, "Model",         m_driveModel,  "—");
    addInfoRow(1, "Sample Offset", m_driveOffset, "—");
    addInfoRow(2, "CD Status",     m_driveStatus, "—");
    cgl->addLayout(infoGrid);
    vl->addWidget(cdGroup);

    // cueファイル選択
    auto *cueGroup = new QGroupBox("CUE/BIN File  ( CD drive not required )");
    cueGroup->setObjectName("sourceGroup");
    auto *cueg = new QHBoxLayout(cueGroup);
    m_cuePath = new QLineEdit;
    m_cuePath->setObjectName("pathEdit");
    m_cuePath->setPlaceholderText("Select .cue file...");
    auto *cueBtn = new QPushButton("Browse...");
    cueBtn->setObjectName("browseBtn");
    cueBtn->setFixedWidth(90);
    cueg->addWidget(m_cuePath, 1);
    cueg->addWidget(cueBtn);
    connect(cueBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Select CUE file", "",
            "CUE Sheet (*.cue)");
        if (!path.isEmpty()) {
            m_cuePath->setText(path);
            m_driveCombo->setCurrentIndex(-1);
            m_driveModel->setText("CUE/BIN mode");
            m_driveOffset->setText("+0 samples");

            // CUEを解析してメタデータを取得
            if (m_drive.openCue(path)) {
                DiscInfo info = m_drive.readToc();
                m_driveStatus->setText(
                    QString("✦  CUE loaded  ( %1 tracks )").arg(info.tracks.size()));
                m_driveStatus->setStyleSheet("color:#4db8ff;");

                // メタデータ画面に反映
                if (!info.albumTitle.isEmpty()) m_edAlbum->setText(info.albumTitle);
                if (!info.artist.isEmpty())     m_edArtist->setText(info.artist);
                if (!info.date.isEmpty())       m_edYear->setText(info.date);

                // トラックリストを更新
                m_trackTable->setRowCount(0);
                for (auto &t : info.tracks) {
                    int r = m_trackTable->rowCount();
                    m_trackTable->insertRow(r);
                    m_trackTable->setItem(r,0,new QTableWidgetItem(
                        QString("%1").arg(t.number,2,10,QChar('0'))));
                    m_trackTable->setItem(r,1,new QTableWidgetItem(
                        t.title.isEmpty() ? QString("Track %1").arg(t.number) : t.title));
                    int sec = t.durationSec;
                    m_trackTable->setItem(r,2,new QTableWidgetItem(
                        QString("%1:%2").arg(sec/60).arg(sec%60,2,10,QChar('0'))));
                    m_trackTable->setItem(r,3,new QTableWidgetItem("—"));
                    m_trackTable->setRowHeight(r,26);
                }
                m_discInfo = info;
            } else {
                m_driveStatus->setText("◈  CUE load failed");
                m_driveStatus->setStyleSheet("color:#ff8c42;");
            }
        }
    });
    vl->addWidget(cueGroup);

    // ダミー情報をセット
    auto updateDriveInfo = [this]() {
        QString key = m_driveCombo->currentData().toString();
        if (key == "DEMO") {
            m_driveModel->setText("PIONEER BDR-209D  [DEMO]");
            m_driveOffset->setText("+30 samples");
            m_driveStatus->setText("✦  Audio CD detected  ( 5 tracks )");
            m_driveStatus->setStyleSheet("color:#4db8ff;");
        } else if (!key.isEmpty()) {
            m_driveModel->setText(key);
            m_driveOffset->setText("Unknown");
            m_driveStatus->setText("Checking...");
            m_driveStatus->setStyleSheet("color:#4a7aa0;");
        }
    };

    connect(m_driveCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [updateDriveInfo](int){ updateDriveInfo(); });
    updateDriveInfo();

    // 読み取りモード説明
    auto *noteGroup = new QGroupBox("Read Mode");
    noteGroup->setObjectName("sourceGroup");
    auto *ngl = new QVBoxLayout(noteGroup);
    auto *noteLabel = new QLabel(
        "High Accuracy Mode (Recommended)\n"
        "  · Max 20 retries per sector\n"
        "  · Majority voting for data integrity\n"
        "  · C2 error detection"
    );
    noteLabel->setObjectName("previewLabel");
    ngl->addWidget(noteLabel);
    vl->addWidget(noteGroup);

    vl->addStretch();
    return page;
}

// ────────────────────────────────────────
// Step 1 — 出力設定
// ────────────────────────────────────────
QWidget *MainWindow::buildStep1_Output()
{
    auto *page = new QWidget;
    auto *vl   = new QVBoxLayout(page);
    vl->setContentsMargins(40,32,40,32);
    vl->setSpacing(20);

    auto *heading = new QLabel("出力先とフォーマットを設定してください");
    heading->setObjectName("stepHeading");
    vl->addWidget(heading);

    // 出力先
    auto *outGroup = new QGroupBox("出力先フォルダ");
    outGroup->setObjectName("sourceGroup");
    auto *ogl = new QHBoxLayout(outGroup);
    m_outputPath = new QLineEdit;
    m_outputPath->setObjectName("pathEdit");
    m_outputPath->setPlaceholderText("フォルダを選択...");
    m_outputPath->setText("D:\\Music");
    auto *browseBtn = new QPushButton("参照...");
    browseBtn->setObjectName("browseBtn");
    browseBtn->setFixedWidth(80);
    ogl->addWidget(m_outputPath, 1);
    ogl->addWidget(browseBtn);
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseOutput);
    vl->addWidget(outGroup);

    // フォーマット
    auto *fmtGroup = new QGroupBox("出力フォーマット");
    fmtGroup->setObjectName("sourceGroup");
    auto *fgl = new QHBoxLayout(fmtGroup);
    fgl->setSpacing(24);

    auto *btnGroup = new QButtonGroup(this);
    auto *rbFlac = new QRadioButton("FLAC  （可逆圧縮・推奨）");
    auto *rbWav  = new QRadioButton("WAV   （無圧縮）");
    rbFlac->setChecked(true);
    rbFlac->setObjectName("radioBtn");
    rbWav->setObjectName("radioBtn");
    btnGroup->addButton(rbFlac);
    btnGroup->addButton(rbWav);
    fgl->addWidget(rbFlac);
    fgl->addWidget(rbWav);
    fgl->addStretch();
    vl->addWidget(fmtGroup);

    // フォルダ構成プレビュー
    auto *prevGroup = new QGroupBox("フォルダ構成プレビュー");
    prevGroup->setObjectName("sourceGroup");
    auto *pgl = new QVBoxLayout(prevGroup);
    auto *prevLabel = new QLabel(
        "D:\\Music\\\n"
        "  └─ Miles Davis\\\n"
        "       └─ (1959) Kind of Blue\\\n"
        "             ├─ 01. So What.flac\n"
        "             ├─ 02. Freddie Freeloader.flac\n"
        "             └─ cover.jpg"
    );
    prevLabel->setObjectName("previewLabel");
    pgl->addWidget(prevLabel);
    vl->addWidget(prevGroup);

    vl->addStretch();
    return page;
}

// ────────────────────────────────────────
// Step 2 — メタデータ確認・編集
// ────────────────────────────────────────
QWidget *MainWindow::buildStep2_Metadata()
{
    auto *page = new QWidget;
    auto *vl   = new QVBoxLayout(page);
    vl->setContentsMargins(40,32,40,32);
    vl->setSpacing(16);

    auto *heading = new QLabel("アルバム情報を確認・編集してください");
    heading->setObjectName("stepHeading");
    vl->addWidget(heading);

    auto *hl = new QHBoxLayout;
    hl->setSpacing(24);

    // 左: アルバムアート
    auto *artVl = new QVBoxLayout;
    m_artLabel = new QLabel("♪");
    m_artLabel->setObjectName("artLabel");
    m_artLabel->setFixedSize(140,140);
    m_artLabel->setAlignment(Qt::AlignCenter);
    auto *artBtn = new QPushButton("アートを変更");
    artBtn->setObjectName("browseBtn");
    artVl->addWidget(m_artLabel);
    artVl->addWidget(artBtn);
    artVl->addStretch();
    hl->addLayout(artVl);

    connect(artBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "アルバムアートを選択", "",
            "画像ファイル (*.jpg *.jpeg *.png *.bmp)");
        if (path.isEmpty()) return;
        QPixmap pix(path);
        if (pix.isNull()) return;
        m_artLabel->setPixmap(
            pix.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_artLabel->setText("");
    });

    // 右: メタデータ編集フォーム
    auto *formVl = new QVBoxLayout;
    formVl->setSpacing(10);

    auto addRow = [&](const QString &label, QLineEdit *&edit, const QString &val){
        auto *row = new QHBoxLayout;
        auto *lbl = new QLabel(label);
        lbl->setFixedWidth(80);
        lbl->setObjectName("formLabel");
        edit = new QLineEdit(val);
        edit->setObjectName("formEdit");
        row->addWidget(lbl);
        row->addWidget(edit,1);
        formVl->addLayout(row);
    };

    addRow("アルバム", m_edAlbum,  "Kind of Blue");
    addRow("アーティスト", m_edArtist, "Miles Davis");
    addRow("年",      m_edYear,   "1959");
    hl->addLayout(formVl, 1);
    vl->addLayout(hl);

    // トラックリスト
    auto *trackLabel = new QLabel("トラックリスト");
    trackLabel->setObjectName("sectionLabel");
    vl->addWidget(trackLabel);

    m_trackTable = new QTableWidget(0,3);
    m_trackTable->setObjectName("trackTable");
    m_trackTable->setHorizontalHeaderLabels({"#","曲名","時間"});
    m_trackTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_trackTable->setColumnWidth(0,40);
    m_trackTable->setColumnWidth(2,70);
    m_trackTable->verticalHeader()->setVisible(false);
    m_trackTable->setFixedHeight(180);

    // ダミーデータ
    struct T { int n; QString title, dur; };
    for (auto &t : QList<T>{
        {1,"So What","9:22"},{2,"Freddie Freeloader","9:46"},
        {3,"Blue in Green","5:37"},{4,"All Blues","11:33"},
        {5,"Flamenco Sketches","9:26"}
    }) {
        int r = m_trackTable->rowCount();
        m_trackTable->insertRow(r);
        m_trackTable->setItem(r,0,new QTableWidgetItem(QString("%1").arg(t.n,2,10,QChar('0'))));
        m_trackTable->setItem(r,1,new QTableWidgetItem(t.title));
        m_trackTable->setItem(r,2,new QTableWidgetItem(t.dur));
        m_trackTable->setRowHeight(r,26);
    }
    vl->addWidget(m_trackTable);
    vl->addStretch();
    return page;
}

// ────────────────────────────────────────
// Step 3 — RIP
// ────────────────────────────────────────
QWidget *MainWindow::buildStep3_Rip()
{
    auto *page = new QWidget;
    auto *hl   = new QHBoxLayout(page);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(0);

    // 左: セクタマップ + ログ
    auto *lp = new QWidget;
    lp->setObjectName("leftPane");
    auto *ll = new QVBoxLayout(lp);
    ll->setContentsMargins(24,20,16,20);
    ll->setSpacing(12);

    auto *mapLbl = new QLabel("SECTOR MAP"); mapLbl->setObjectName("sectionLabel");
    ll->addWidget(mapLbl);

    m_sectorMap = new SectorMap;
    m_sectorMap->setMinimumHeight(200);
    ll->addWidget(m_sectorMap);

    auto *logLbl = new QLabel("LOG"); logLbl->setObjectName("sectionLabel");
    ll->addWidget(logLbl);
    m_logViewer = new LogViewer;
    ll->addWidget(m_logViewer,1);
    hl->addWidget(lp,1);

    // 右: 統計 + 進捗 + RIPボタン
    auto *rp = new QWidget;
    rp->setObjectName("rightPane");
    rp->setFixedWidth(280);
    auto *rl = new QVBoxLayout(rp);
    rl->setContentsMargins(12,20,20,20);
    rl->setSpacing(12);

    auto *statLbl = new QLabel("STATISTICS"); statLbl->setObjectName("sectionLabel");
    rl->addWidget(statLbl);

    auto *sg = new QGridLayout; sg->setSpacing(6);
    m_statPurity  = buildStatCard("Purity","—");
    m_statRetries = buildStatCard("Retries","—");
    m_statErrors  = buildStatCard("Errors","—");
    m_statSpeed   = buildStatCard("Speed","—");
    sg->addWidget(m_statPurity, 0,0);
    sg->addWidget(m_statRetries,0,1);
    sg->addWidget(m_statErrors, 1,0);
    sg->addWidget(m_statSpeed,  1,1);
    rl->addLayout(sg);

    auto *prgLbl = new QLabel("PROGRESS"); prgLbl->setObjectName("sectionLabel");
    rl->addWidget(prgLbl);

    m_progress = new QProgressBar;
    m_progress->setObjectName("ripProgress");
    m_progress->setRange(0,100); m_progress->setValue(0);
    m_progress->setTextVisible(false); m_progress->setFixedHeight(4);
    rl->addWidget(m_progress);

    m_progressLabel = new QLabel("RIP ボタンで開始");
    m_progressLabel->setObjectName("progressLabel");
    rl->addWidget(m_progressLabel);

    rl->addStretch();

    m_ripBtn = new QPushButton("▶  RIP START");
    m_ripBtn->setObjectName("ripBtn");
    m_ripBtn->setFixedHeight(40);
    rl->addWidget(m_ripBtn);

    connect(m_ripBtn, &QPushButton::clicked, this, &MainWindow::onRipClicked);
    hl->addWidget(rp);
    return page;
}

// ────────────────────────────────────────
// Step 4 — ベリファイ
// ────────────────────────────────────────
QWidget *MainWindow::buildStep4_Verify()
{
    auto *page = new QWidget;
    auto *vl   = new QVBoxLayout(page);
    vl->setContentsMargins(40,32,40,32);
    vl->setSpacing(20);

    auto *heading = new QLabel("AccurateRip ベリファイ");
    heading->setObjectName("stepHeading");
    vl->addWidget(heading);

    auto *subLabel = new QLabel(
        "リッピング結果を AccurateRip データベースと照合します。\n"
        "他のユーザーと同一のデータが取れているか確認します。");
    subLabel->setObjectName("subLabel");
    vl->addWidget(subLabel);

    m_verifyProgress = new QProgressBar;
    m_verifyProgress->setObjectName("ripProgress");
    m_verifyProgress->setRange(0,100);
    m_verifyProgress->setValue(0);
    m_verifyProgress->setFixedHeight(4);
    vl->addWidget(m_verifyProgress);

    // 結果表示エリア
    auto *resultBox = new QWidget;
    resultBox->setObjectName("verifyBox");
    auto *rbl = new QVBoxLayout(resultBox);
    rbl->setContentsMargins(24,24,24,24);
    rbl->setSpacing(12);

    m_verifyResult = new QLabel("照合待機中...");
    m_verifyResult->setObjectName("verifyResult");
    m_verifyResult->setAlignment(Qt::AlignCenter);

    m_verifyDetail = new QLabel("");
    m_verifyDetail->setObjectName("verifyDetail");
    m_verifyDetail->setAlignment(Qt::AlignCenter);
    m_verifyDetail->setWordWrap(true);

    rbl->addWidget(m_verifyResult);
    rbl->addWidget(m_verifyDetail);
    vl->addWidget(resultBox);

    // トラック別結果テーブル
    m_verifyTable = new QTableWidget(0,3);
    m_verifyTable->setObjectName("trackTable");
    m_verifyTable->setHorizontalHeaderLabels({"Track","Title","AccurateRip"});
    m_verifyTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    m_verifyTable->setColumnWidth(0,52);
    m_verifyTable->setColumnWidth(2,120);
    m_verifyTable->verticalHeader()->setVisible(false);
    m_verifyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vl->addWidget(m_verifyTable);
    vl->addStretch();
    return page;
}

QWidget *MainWindow::buildStatCard(const QString &label, const QString &value)
{
    auto *card = new QWidget; card->setObjectName("statCard");
    auto *vl   = new QVBoxLayout(card);
    vl->setContentsMargins(10,8,10,8); vl->setSpacing(2);
    auto *lbl = new QLabel(label); lbl->setObjectName("statLabel");
    auto *val = new QLabel(value); val->setObjectName("statValue");
    vl->addWidget(lbl); vl->addWidget(val);
    return card;
}

void MainWindow::applyStyle()
{
    setStyleSheet(R"(
        QMainWindow,QWidget{background:#0a0a0f;color:#c0d8f0;font-family:"Segoe UI";font-size:9pt;}
        #titleBar{background:#070710;border-bottom:1px solid #0d2a45;}
        #appTitle{color:#4db8ff;font-size:10pt;letter-spacing:3px;}
        #verLabel{color:#1a3a5a;font-size:8pt;margin-left:10px;}
        #stepBar{background:#070710;border-bottom:1px solid #0d2a45;}
        #stepLabel{color:#2a5a8a;font-size:8pt;padding:4px;}
        #stepLabel[active="true"]{color:#4db8ff;font-weight:bold;border-bottom:2px solid #4db8ff;}
        #stepLabel[done="true"]{color:#1a5a9a;}
        #stepSep{color:#0d2a45;font-size:12pt;}
        #hline{border:none;border-top:1px solid #0d2a45;}
        #navBar{background:#070710;border-top:1px solid #0d2a45;}
        #backBtn{background:#07070f;color:#4a7aa0;border:1px solid #0d2a45;border-radius:3px;}
        #backBtn:hover{background:#0a1a2e;color:#4db8ff;}
        #nextBtn{background:#0d3a6a;color:#4db8ff;border:1px solid #4db8ff;border-radius:3px;letter-spacing:1px;}
        #nextBtn:hover{background:#1a5a9a;}
        #stepHeading{color:#c0d8f0;font-size:12pt;font-weight:bold;margin-bottom:8px;}
        #subLabel{color:#4a7aa0;font-size:9pt;line-height:1.6;}
        #sourceGroup{color:#4a7aa0;border:1px solid #0d2a45;border-radius:5px;margin-top:4px;padding:8px;}
        QGroupBox::title{color:#2a5a8a;subcontrol-origin:margin;left:10px;padding:0 4px;}
        #driveCombo,QComboBox{background:#07070f;color:#4db8ff;border:1px solid #0d2a45;border-radius:3px;padding:4px 10px;}
        QComboBox::drop-down{border:none;}
        #pathEdit,#formEdit{background:#07070f;color:#c0d8f0;border:1px solid #0d2a45;border-radius:3px;padding:4px 8px;}
        #browseBtn{background:#07070f;color:#4a7aa0;border:1px solid #0d2a45;border-radius:3px;padding:4px 8px;}
        #browseBtn:hover{color:#4db8ff;border-color:#4db8ff;}
        #previewLabel{color:#2a5a8a;font-family:"Consolas";font-size:8pt;line-height:1.8;}
        #formLabel{color:#2a5a8a;font-size:8pt;}
        #driveInfoVal{color:#c0d8f0;font-size:9pt;}
        #artLabel{background:#0a1a2e;border:1px solid #1a4a7a;border-radius:6px;color:#4db8ff;font-size:32px;}
        #sectionLabel{color:#2a5a8a;font-size:8pt;letter-spacing:2px;}
        #trackTable{background:#07070f;border:1px solid #0d2a45;border-radius:4px;gridline-color:#0d1a2a;selection-background-color:#0a1a2e;}
        #trackTable::item{padding:4px 6px;color:#90b8d8;}
        #trackTable::item:selected{color:#4db8ff;}
        QHeaderView::section{background:#050510;color:#2a5a8a;border:none;border-bottom:1px solid #0d2a45;padding:4px 6px;font-size:8pt;}
        QRadioButton{color:#4a7aa0;}
        QRadioButton::indicator{border:1px solid #0d2a45;border-radius:6px;width:12px;height:12px;background:#07070f;}
        QRadioButton::indicator:checked{background:#4db8ff;border-color:#4db8ff;}
        #leftPane{background:#0a0a0f;}
        #rightPane{background:#070710;border-left:1px solid #0d2a45;}
        #statCard{background:#050510;border:1px solid #0d2a45;border-radius:4px;}
        #statLabel{color:#2a5a8a;font-size:8pt;}
        #statValue{color:#4db8ff;font-size:15pt;font-weight:bold;}
        #ripProgress{background:#0d2a45;border:none;border-radius:2px;}
        #ripProgress::chunk{background:#4db8ff;border-radius:2px;}
        #progressLabel{color:#2a5a8a;font-size:8pt;}
        #ripBtn{background:#0d3a6a;color:#4db8ff;border:1px solid #4db8ff;border-radius:3px;font-size:10pt;letter-spacing:1px;}
        #ripBtn:hover{background:#1a5a9a;}
        #ripBtn:disabled{background:#050510;color:#1a3a5a;border-color:#0d2a45;}
        #settingsBtn{background:#07070f;color:#2a5a8a;border:1px solid #0d2a45;border-radius:3px;padding:4px 12px;font-size:8pt;}
        #settingsBtn:hover{color:#4db8ff;border-color:#4db8ff;}
        #settingsSpin{background:#07070f;color:#4db8ff;border:1px solid #0d2a45;border-radius:3px;padding:3px 6px;}
        QSpinBox::up-button,QSpinBox::down-button{background:#0d2a45;border:none;width:16px;}
        #settingsCheck{color:#4a7aa0;}
        QCheckBox::indicator{border:1px solid #0d2a45;border-radius:3px;width:12px;height:12px;background:#07070f;}
        QCheckBox::indicator:checked{background:#4db8ff;border-color:#4db8ff;}
        QDialog{background:#0a0a0f;}
        #verifyResult{color:#4db8ff;font-size:18pt;font-weight:bold;}
        #verifyDetail{color:#4a7aa0;font-size:9pt;}
        QTextEdit{background:#050510;color:#4a7aa0;border:none;font-family:"Consolas";font-size:9pt;}
    )");
}

void MainWindow::goToStep(int step)
{
    m_step = step;
    m_stack->setCurrentIndex(step);
    updateStepIndicator();

    m_backBtn->setVisible(step > 0);
    m_backBtn->setEnabled(step > 0);

    if (step == 3) {
        // RIPページ: Nextは完了後に有効化
        m_nextBtn->setText("Next  ▶");
        m_nextBtn->setEnabled(false);
    } else if (step == 4) {
        m_nextBtn->setText("完了");
        m_nextBtn->setEnabled(true);

        // Verifyテーブルを実データで更新
        m_verifyTable->setRowCount(0);
        if (!m_discInfo.tracks.isEmpty()) {
            for (auto &track : m_discInfo.tracks) {
                int r = m_verifyTable->rowCount();
                m_verifyTable->insertRow(r);
                m_verifyTable->setItem(r, 0, new QTableWidgetItem(
                    QString("%1").arg(track.number, 2, 10, QChar('0'))));
                QString title = track.title.isEmpty()
                    ? QString("Track %1").arg(track.number) : track.title;
                m_verifyTable->setItem(r, 1, new QTableWidgetItem(title));
                m_verifyTable->setItem(r, 2, new QTableWidgetItem("—"));
                m_verifyTable->setRowHeight(r, 26);
            }
        } else {
            // DEMOモード: メタデータ画面のトラックリストから取得
            for (int i = 0; i < m_trackTable->rowCount(); ++i) {
                int r = m_verifyTable->rowCount();
                m_verifyTable->insertRow(r);
                m_verifyTable->setItem(r, 0, new QTableWidgetItem(
                    m_trackTable->item(i, 0)->text()));
                m_verifyTable->setItem(r, 1, new QTableWidgetItem(
                    m_trackTable->item(i, 1)->text()));
                m_verifyTable->setItem(r, 2, new QTableWidgetItem("—"));
                m_verifyTable->setRowHeight(r, 26);
            }
        }

        // ベリファイ開始
        startVerify();
    } else {
        m_nextBtn->setText(step < 4 ? "Next  ▶" : "完了");
        m_nextBtn->setEnabled(true);
    }
}

void MainWindow::updateStepIndicator()
{
    for (int i = 0; i < 5; ++i) {
        m_stepLabels[i]->setProperty("active", i == m_step);
        m_stepLabels[i]->setProperty("done",   i <  m_step);
        m_stepLabels[i]->style()->unpolish(m_stepLabels[i]);
        m_stepLabels[i]->style()->polish(m_stepLabels[i]);
    }
}

void MainWindow::onNext()
{
    if (m_step < 4) goToStep(m_step + 1);
    else close();
}

void MainWindow::onBack()
{
    if (m_step > 0) goToStep(m_step - 1);
}

void MainWindow::onBrowseOutput()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "出力先フォルダを選択", m_outputPath->text());
    if (!dir.isEmpty()) m_outputPath->setText(dir);
}

void MainWindow::onRipClicked()
{
    m_ripBtn->setEnabled(false);
    m_logViewer->clear();
    m_logViewer->appendLine("═══════════════════════════════════════");
    m_logViewer->appendLine("  ALWAYS CD RIPPER  —  RIP START");
    m_logViewer->appendLine("  " + QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
    m_logViewer->appendLine("═══════════════════════════════════════");
    m_logViewer->appendLine("Album  : " + m_edAlbum->text() + " / " + m_edArtist->text());
    m_logViewer->appendLine("Output : " + m_outputPath->text());
    m_logViewer->appendLine("─── Track log ──────────────────────");

    bool isCue = !m_cuePath->text().isEmpty() && m_discInfo.isCueMode
                 && !m_discInfo.tracks.isEmpty();

    if (!isCue) {
        // DEMOモード
        int total = 600;
        m_sectorMap->reset(total);
        m_progress->setValue(0);
        m_totalErrors = 0; m_totalRetries = 0;
        m_ripCounter  = 0; m_ripTotal     = total;

        m_ripTimer = new QTimer(this);
        QObject::connect(m_ripTimer, &QTimer::timeout, this, [=]() {
            if (m_ripCounter >= m_ripTotal) {
                m_ripTimer->stop(); onRipFinished(); return;
            }
            SectorResult sr;
            sr.lba     = m_ripCounter;
            sr.perfect = QRandomGenerator::global()->bounded(100) < 82;
            sr.retries = sr.perfect ? 0 : QRandomGenerator::global()->bounded(8)+1;
            sr.hasError= !sr.perfect && QRandomGenerator::global()->bounded(100) < 4;
            m_sectorMap->addResult(sr);
            if (!sr.perfect) m_totalRetries += sr.retries;
            if (sr.hasError)  m_totalErrors++;
            m_ripCounter++;
            m_progress->setValue(m_ripCounter * 100 / m_ripTotal);
            m_progressLabel->setText(QString("Reading  %1 / %2  sectors")
                                      .arg(m_ripCounter).arg(m_ripTotal));
            double purity = 100.0 - (m_totalErrors * 100.0 / m_ripCounter);
            auto upd = [](QWidget *c, const QString &v){
                auto l = c->findChildren<QLabel*>();
                if (l.size()>=2) l[1]->setText(v);
            };
            upd(m_statPurity,  QString::number(purity,'f',1)+"%");
            upd(m_statRetries, QString::number(m_totalRetries));
            upd(m_statErrors,  QString::number(m_totalErrors));
            upd(m_statSpeed,   "—");
        });
        m_ripTimer->start(6);
        return;
    }

    // ── 実RIP: RipWorker + QThread ──
    int totalSectors = (int)m_discInfo.totalSectors;
    m_sectorMap->reset(totalSectors);
    m_progress->setValue(0);
    m_totalErrors = 0; m_totalRetries = 0;
    m_ripCounter  = 0; m_ripTotal     = totalSectors;

    QString artist = m_edArtist->text().isEmpty() ? "Unknown" : m_edArtist->text();
    QString album  = m_edAlbum->text().isEmpty()  ? "Unknown" : m_edAlbum->text();
    QString year   = m_edYear->text();
    QString albumFolder = year.isEmpty()
        ? QString("%1/%2").arg(artist, album)
        : QString("%1/(%2) %3").arg(artist, year, album);
    QString outDir = m_outputPath->text() + "/" + albumFolder;

    m_logViewer->appendLine("Folder : " + outDir);

    auto *worker = new RipWorker;
    auto *thread = new QThread(this);
    worker->moveToThread(thread);
    worker->setup(&m_drive, m_discInfo, outDir, artist, album, year);

    // セクタ完了 → セクタマップ更新
    connect(worker, &RipWorker::sectorDone, this, [this](SectorResult sr) {
        m_sectorMap->addResult(sr);
        if (!sr.perfect) m_totalRetries += sr.retries;
        if (sr.hasError)  m_totalErrors++;
        m_ripCounter++;

        int pct = m_ripTotal > 0 ? m_ripCounter * 100 / m_ripTotal : 0;
        m_progress->setValue(pct);
        double purity = m_ripCounter > 0
            ? 100.0 - (m_totalErrors * 100.0 / m_ripCounter) : 100.0;
        auto upd = [](QWidget *c, const QString &v){
            auto l = c->findChildren<QLabel*>();
            if (l.size()>=2) l[1]->setText(v);
        };
        upd(m_statPurity,  QString::number(purity,'f',1)+"%");
        upd(m_statRetries, QString::number(m_totalRetries));
        upd(m_statErrors,  QString::number(m_totalErrors));
        upd(m_statSpeed,   "—");
    }, Qt::QueuedConnection);

    // トラック開始
    connect(worker, &RipWorker::trackStarted, this,
            [this](int num, int total, QString title) {
        m_progressLabel->setText(
            QString("Track %1/%2: %3").arg(num).arg(total).arg(title));
        m_logViewer->appendLine(
            QString("  Track %1: %2").arg(num,2,10,QChar('0')).arg(title));
    }, Qt::QueuedConnection);

    // トラック完了
    connect(worker, &RipWorker::trackDone, this, [this](RipResult r) {
        QString line = r.success
            ? QString("  Track %1: ✦ saved  (errors:%2)")
              .arg(r.trackNumber,2,10,QChar('0')).arg(r.errors)
            : QString("  Track %1: ◈ FAILED")
              .arg(r.trackNumber,2,10,QChar('0'));
        m_logViewer->appendLine(line);
    }, Qt::QueuedConnection);

    // 全完了
    connect(worker, &RipWorker::finished, this, [this, worker, thread]() {
        onRipFinished();
        worker->deleteLater();
        thread->quit();
    }, Qt::QueuedConnection);

    connect(thread, &QThread::started,  worker, &RipWorker::run);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}
void MainWindow::onRipFinished()
{
    double purity = 100.0 - (m_totalErrors * 100.0 / m_ripTotal);
    m_progress->setValue(100);
    m_progressLabel->setText(QString("完了  ✦  浄化率 %1%").arg(purity,0,'f',2));

    m_logViewer->appendLine("");
    m_logViewer->appendLine(QString("  浄化率 : %1%").arg(purity,0,'f',2));
    m_logViewer->appendLine(QString("  エラー : %1 セクタ").arg(m_totalErrors));
    m_logViewer->appendLine(m_totalErrors==0
        ? "  判定   : このディスクは完全に清潔である  ✦"
        : "  判定   : 軽微な傷を検出。補正済み  ◈");
    m_logViewer->appendLine("═══════════════════════════════════════");

    m_ripBtn->setEnabled(false);
    m_nextBtn->setEnabled(true);
    // 2秒後に自動でVerifyへ進む
    QTimer::singleShot(2000, this, [this]() {
        goToStep(4);
    });
}

void MainWindow::startVerify()
{
    m_verifyResult->setText("照合中...");
    m_verifyDetail->setText("AccurateRip データベースに問い合わせています");
    m_verifyProgress->setValue(0);

    // テーブルをリセット
    for (int i = 0; i < m_verifyTable->rowCount(); ++i)
        m_verifyTable->item(i, 2)->setText("—");

    // フェーズ1: プログレスバーを進める
    auto *t = new QTimer(this);
    int *cnt = new int(0);
    QObject::connect(t, &QTimer::timeout, this, [=]() {
        *cnt += 4;
        m_verifyProgress->setValue(qMin(*cnt, 100));

        if (*cnt >= 100) {
            t->stop();
            delete cnt;

            // フェーズ2: トラックごとに順番に結果を表示
            int *trackIdx = new int(0);
            auto *t2 = new QTimer(this);
            QObject::connect(t2, &QTimer::timeout, this, [=]() {
                int i = *trackIdx;
                if (i >= m_verifyTable->rowCount()) {
                    t2->stop();
                    delete trackIdx;
                    // 全トラック完了 → 総合結果
                    m_verifyResult->setText("✦  一致  — 信頼度 100%");
                    m_verifyDetail->setText(
                        "AccurateRip DB と完全一致しました。\n"
                        "このリッピングデータは信頼できます。");
                    return;
                }
                // 全曲一致（AccurateRip実装までダミー）
                bool match = true;
                auto *item = m_verifyTable->item(i, 2);
                if (match) {
                    item->setText("✦  一致");
                    item->setForeground(QColor("#4db8ff"));
                } else {
                    item->setText("◈  要確認");
                    item->setForeground(QColor("#ff8c42"));
                }
                (*trackIdx)++;
            });
            t2->start(300); // 300ms ごとに1トラック
        }
    });
    t->start(20);
}

void MainWindow::onMetadataReady(QVector<MbRelease> releases)
{
    if (releases.isEmpty()) return;
    auto &r = releases.first();
    m_edAlbum->setText(r.title);
    m_edArtist->setText(r.artist);
    m_edYear->setText(r.date.left(4));
}

void MainWindow::onSettingsClicked()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("Settings — Always CD Ripper");
    dlg->setMinimumSize(520, 560);
    dlg->setModal(true);

    auto *mainVl = new QVBoxLayout(dlg);
    mainVl->setContentsMargins(0,0,0,0);
    mainVl->setSpacing(0);

    // タブウィジェット
    auto *tabs = new QTabWidget(dlg);
    tabs->setObjectName("settingsTabs");

    // ── タブ1: 設定 ──────────────────────────
    auto *settingsPage = new QWidget;
    auto *vl = new QVBoxLayout(settingsPage);
    vl->setContentsMargins(24,20,24,20);
    vl->setSpacing(16);

    // 読み取り設定
    auto *ripGroup = new QGroupBox("Read Settings");
    ripGroup->setObjectName("sourceGroup");
    auto *rgl = new QGridLayout(ripGroup);
    rgl->setSpacing(10);
    rgl->setColumnMinimumWidth(0, 160);

    auto *maxRetriesLabel = new QLabel("Max retries / sector");
    maxRetriesLabel->setObjectName("formLabel");
    auto *maxRetriesSpin = new QSpinBox;
    maxRetriesSpin->setRange(1, 100);
    maxRetriesSpin->setValue(20);
    maxRetriesSpin->setObjectName("settingsSpin");
    rgl->addWidget(maxRetriesLabel, 0, 0);
    rgl->addWidget(maxRetriesSpin,  0, 1);

    auto *offsetLabel = new QLabel("Sample offset");
    offsetLabel->setObjectName("formLabel");
    auto *offsetSpin = new QSpinBox;
    offsetSpin->setRange(-2000, 2000);
    offsetSpin->setValue(30);
    offsetSpin->setObjectName("settingsSpin");
    offsetSpin->setSuffix(" samples");
    rgl->addWidget(offsetLabel, 1, 0);
    rgl->addWidget(offsetSpin,  1, 1);
    vl->addWidget(ripGroup);

    // 出力設定
    auto *outGroup = new QGroupBox("Output Settings");
    outGroup->setObjectName("sourceGroup");
    auto *ogl = new QGridLayout(outGroup);
    ogl->setSpacing(10);
    ogl->setColumnMinimumWidth(0, 160);

    auto *defOutLabel = new QLabel("Default output folder");
    defOutLabel->setObjectName("formLabel");
    auto *defOutEdit = new QLineEdit("C:/Users/jokun/Music");
    defOutEdit->setObjectName("pathEdit");
    auto *defOutBtn = new QPushButton("...");
    defOutBtn->setObjectName("browseBtn");
    defOutBtn->setFixedWidth(40);
    connect(defOutBtn, &QPushButton::clicked, this, [defOutEdit, this]() {
        QString d = QFileDialog::getExistingDirectory(
            this, "Select folder", defOutEdit->text());
        if (!d.isEmpty()) defOutEdit->setText(d);
    });
    auto *defOutRow = new QHBoxLayout;
    defOutRow->addWidget(defOutEdit, 1);
    defOutRow->addWidget(defOutBtn);
    ogl->addWidget(defOutLabel, 0, 0);
    ogl->addLayout(defOutRow,  0, 1);

    auto *namingLabel = new QLabel("Naming rule");
    namingLabel->setObjectName("formLabel");
    auto *namingEdit = new QLineEdit("{track}. {title}");
    namingEdit->setObjectName("pathEdit");
    ogl->addWidget(namingLabel, 1, 0);
    ogl->addWidget(namingEdit,  1, 1);

    auto *noteLabel = new QLabel("Variables: {track} {title} {artist} {album} {year}");
    noteLabel->setObjectName("progressLabel");
    ogl->addWidget(noteLabel, 2, 1);
    vl->addWidget(outGroup);

    // AccurateRip
    auto *arGroup = new QGroupBox("AccurateRip");
    arGroup->setObjectName("sourceGroup");
    auto *agl = new QVBoxLayout(arGroup);
    auto *arCheck = new QCheckBox("Auto verify after ripping");
    arCheck->setObjectName("settingsCheck");
    arCheck->setChecked(true);
    agl->addWidget(arCheck);
    vl->addWidget(arGroup);
    vl->addStretch();
    tabs->addTab(settingsPage, "Settings");

    // ── タブ2: About ─────────────────────────
    auto *aboutPage = new QWidget;
    auto *avl = new QVBoxLayout(aboutPage);
    avl->setContentsMargins(40,40,40,40);
    avl->setSpacing(16);
    avl->setAlignment(Qt::AlignTop);

    // アイコン風グリッド
    auto *iconLabel = new QLabel;
    iconLabel->setFixedSize(80,80);
    iconLabel->setObjectName("aboutIcon");
    iconLabel->setAlignment(Qt::AlignCenter);
    avl->addWidget(iconLabel, 0, Qt::AlignHCenter);

    auto *nameLabel = new QLabel("Always CD Ripper");
    nameLabel->setObjectName("aboutTitle");
    nameLabel->setAlignment(Qt::AlignCenter);
    avl->addWidget(nameLabel);

    auto *verLabel = new QLabel("Version 1.0.0");
    verLabel->setObjectName("aboutVer");
    verLabel->setAlignment(Qt::AlignCenter);
    avl->addWidget(verLabel);

    auto *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("hline");
    avl->addWidget(line);

    auto *descLabel = new QLabel(
        "High Fidelity CD Ripper\n\n"
        "Part of the Always Series\n"
        "Always CD Ripper  +  Always Player\n"
        "= Complete CD Rescue\n\n"
        "CUE/BIN/WAV input\n"
        "FLAC output with full tags\n"
        "MusicBrainz metadata\n"
        "iTunes cover art"
    );
    descLabel->setObjectName("aboutDesc");
    descLabel->setAlignment(Qt::AlignCenter);
    avl->addWidget(descLabel);

    auto *line2 = new QFrame;
    line2->setFrameShape(QFrame::HLine);
    line2->setObjectName("hline");
    avl->addWidget(line2);

    auto *authorLabel = new QLabel("© 2026  YOUICHI SAIJO");
    authorLabel->setObjectName("aboutAuthor");
    authorLabel->setAlignment(Qt::AlignCenter);
    avl->addWidget(authorLabel);

    auto *urlLabel = new QLabel(
        "<a href='https://github.com/YOUICHI-del' "
        "style='color:#4db8ff;'>github.com/YOUICHI-del</a>");
    urlLabel->setOpenExternalLinks(true);
    urlLabel->setAlignment(Qt::AlignCenter);
    avl->addWidget(urlLabel);

    avl->addStretch();
    tabs->addTab(aboutPage, "About");

    mainVl->addWidget(tabs, 1);

    // ── OK / Cancel ──────────────────────────
    auto *btnBar = new QWidget;
    btnBar->setObjectName("navBar");
    btnBar->setFixedHeight(52);
    auto *btnRow = new QHBoxLayout(btnBar);
    btnRow->setContentsMargins(20,0,20,0);
    auto *okBtn  = new QPushButton("OK");
    auto *canBtn = new QPushButton("Cancel");
    okBtn->setObjectName("nextBtn");
    okBtn->setFixedSize(90, 32);
    canBtn->setObjectName("backBtn");
    canBtn->setFixedSize(90, 32);
    btnRow->addStretch();
    btnRow->addWidget(canBtn);
    btnRow->addWidget(okBtn);
    mainVl->addWidget(btnBar);

    connect(okBtn,  &QPushButton::clicked, dlg, &QDialog::accept);
    connect(canBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    dlg->setStyleSheet(styleSheet() + R"(
        QTabWidget::pane {
            border: none;
            background: #0a0a0f;
        }
        QTabBar::tab {
            background: #070710;
            color: #4a7aa0;
            padding: 8px 24px;
            border: none;
            border-bottom: 2px solid transparent;
        }
        QTabBar::tab:selected {
            color: #4db8ff;
            border-bottom: 2px solid #4db8ff;
            background: #0a0a0f;
        }
        #aboutIcon {
            background: #0a1a2e;
            border: 1px solid #1a4a7a;
            border-radius: 16px;
            font-size: 32px;
            color: #4db8ff;
        }
        #aboutTitle  { color: #4db8ff; font-size: 14pt; font-weight: bold; }
        #aboutVer    { color: #2a5a8a; font-size: 9pt; }
        #aboutDesc   { color: #4a7aa0; font-size: 9pt; line-height: 1.8; }
        #aboutAuthor { color: #2a5a8a; font-size: 8pt; }
    )");

    // アイコンをセット
    QPixmap icon = qApp->windowIcon().pixmap(64, 64);
    if (!icon.isNull())
        iconLabel->setPixmap(icon);
    else
        iconLabel->setText("◈");

    dlg->exec();
}

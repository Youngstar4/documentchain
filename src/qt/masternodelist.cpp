#include <qt/masternodelist.h>
#include <qt/forms/ui_masternodelist.h>

#include <bls/bls.h>
#include <qt/clientmodel.h>
#include <clientversion.h>
#include <coins.h>
#include <qt/guiutil.h>
#include <netbase.h>
#include <qt/walletmodel.h>
#include <qt/rpcconsole.h>
#include <univalue.h>

#include <QCheckBox>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>
#include <QSettings>
#include <QStringList>
#include <QTableWidgetItem>
#include <QtGui/QClipboard>

int GetOffsetFromUtc()
{
#if QT_VERSION < 0x050200
    const QDateTime dateTime1 = QDateTime::currentDateTime();
    const QDateTime dateTime2 = QDateTime(dateTime1.date(), dateTime1.time(), Qt::UTC);
    return dateTime1.secsTo(dateTime2);
#else
    return QDateTime::currentDateTime().offsetFromUtc();
#endif
}

template <typename T>
class CMasternodeListWidgetItem : public QTableWidgetItem
{
    T itemData;

public:
    explicit CMasternodeListWidgetItem(const QString& text, const T& data, int type = Type) :
        QTableWidgetItem(text, type),
        itemData(data) {}

    bool operator<(const QTableWidgetItem& other) const
    {
        return itemData < ((CMasternodeListWidgetItem*)&other)->itemData;
    }
};

MasternodeList::MasternodeList(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MasternodeList),
    clientModel(0),
    walletModel(0),
    fFilterUpdatedDIP3(true),
    nTimeFilterUpdatedDIP3(0),
    nTimeUpdatedDIP3(0),
    mnListChanged(true)
{
    ui->setupUi(this);

    GUIUtil::setFont({ui->label_count_2,
                      ui->countLabelDIP3
                     }, GUIUtil::FontWeight::Bold, 14);
    GUIUtil::setFont({ui->label_filter_2}, GUIUtil::FontWeight::Normal, 15);

    int columnAddressWidth = 200;
    int columnStatusWidth = 80;
    int columnPoSeScoreWidth = 80;
    int columnRegisteredWidth = 80;
    int columnLastPaidWidth = 80;
    int columnNextPaymentWidth = 100;
    int columnPayeeWidth = 130;
    int columnOperatorRewardWidth = 130;
    int columnCollateralWidth = 130;
    int columnOwnerWidth = 130;
    int columnVotingWidth = 130;

    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_SERVICE, columnAddressWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_STATUS, columnStatusWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_POSE, columnPoSeScoreWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_REGISTERED, columnRegisteredWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_LAST_PAYMENT, columnLastPaidWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_NEXT_PAYMENT, columnNextPaymentWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_PAYOUT_ADDRESS, columnPayeeWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_OPERATOR_REWARD, columnOperatorRewardWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_COLLATERAL_ADDRESS, columnCollateralWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_OWNER_ADDRESS, columnOwnerWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(COLUMN_VOTING_ADDRESS, columnVotingWidth);

    // dummy column for proTxHash
    // TODO use a proper table model for the MN list
    ui->tableWidgetMasternodesDIP3->insertColumn(COLUMN_PROTX_HASH);
    ui->tableWidgetMasternodesDIP3->setColumnHidden(COLUMN_PROTX_HASH, true);

    ui->tableWidgetMasternodesDIP3->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->filterLineEditDIP3->setPlaceholderText(tr("Filter by any property (e.g. address or protx hash)"));

    QAction* copyProTxHashAction = new QAction(tr("Copy ProTx Hash"), this);
    QAction* copyCollateralOutpointAction = new QAction(tr("Copy Collateral Outpoint"), this);
    QAction* sendProtxUpdateServiceAction = new QAction(tr("Provider Update Service Transaction..."), this);
    sendProtxUpdateServiceAction->setStatusTip(tr("Reactivate POSE_BANNED or specify new IP"));
    contextMenuDIP3 = new QMenu(this);
    contextMenuDIP3->addAction(copyProTxHashAction);
    contextMenuDIP3->addAction(copyCollateralOutpointAction);
    contextMenuDIP3->addAction(sendProtxUpdateServiceAction);
    connect(ui->tableWidgetMasternodesDIP3, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenuDIP3(const QPoint&)));
    connect(ui->tableWidgetMasternodesDIP3, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(extraInfoDIP3_clicked()));
    connect(copyProTxHashAction, SIGNAL(triggered()), this, SLOT(copyProTxHash_clicked()));
    connect(copyCollateralOutpointAction, SIGNAL(triggered()), this, SLOT(copyCollateralOutpoint_clicked()));
    connect(sendProtxUpdateServiceAction, SIGNAL(triggered()), this, SLOT(sendProtxUpdateService_clicked()));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDIP3ListScheduled()));
    timer->start(1000);

    GUIUtil::updateFonts();
}

MasternodeList::~MasternodeList()
{
    delete ui;
}

void MasternodeList::setClientModel(ClientModel* model)
{
    this->clientModel = model;
    if (model) {
        // try to update list when masternode count changes
        connect(clientModel, SIGNAL(masternodeListChanged()), this, SLOT(handleMasternodeListChanged()));
    }
}

void MasternodeList::setWalletModel(WalletModel* model)
{
    this->walletModel = model;
}

void MasternodeList::showContextMenuDIP3(const QPoint& point)
{
    QTableWidgetItem* item = ui->tableWidgetMasternodesDIP3->itemAt(point);
    if (item) contextMenuDIP3->exec(QCursor::pos());
}

void MasternodeList::handleMasternodeListChanged()
{
    LOCK(cs_dip3list);
    mnListChanged = true;
}

void MasternodeList::updateDIP3ListScheduled()
{
    TRY_LOCK(cs_dip3list, fLockAcquired);
    if (!fLockAcquired) return;

    if (!clientModel || clientModel->node().shutdownRequested()) {
        return;
    }

    // To prevent high cpu usage update only once in MASTERNODELIST_FILTER_COOLDOWN_SECONDS seconds
    // after filter was last changed unless we want to force the update.
    if (fFilterUpdatedDIP3) {
        int64_t nSecondsToWait = nTimeFilterUpdatedDIP3 - GetTime() + MASTERNODELIST_FILTER_COOLDOWN_SECONDS;
        ui->countLabelDIP3->setText(tr("Please wait...") + " " + QString::number(nSecondsToWait));

        if (nSecondsToWait <= 0) {
            updateDIP3List();
            fFilterUpdatedDIP3 = false;
        }
    } else if (mnListChanged) {
        int64_t nMnListUpdateSecods = clientModel->masternodeSync().isBlockchainSynced() ? MASTERNODELIST_UPDATE_SECONDS : MASTERNODELIST_UPDATE_SECONDS * 10;
        int64_t nSecondsToWait = nTimeUpdatedDIP3 - GetTime() + nMnListUpdateSecods;

        if (nSecondsToWait <= 0) {
            updateDIP3List();
            mnListChanged = false;
        }
    }
}

void MasternodeList::updateDIP3List()
{
    if (!clientModel || clientModel->node().shutdownRequested()) {
        return;
    }

    auto mnList = clientModel->getMasternodeList();
    std::map<uint256, CTxDestination> mapCollateralDests;

    {
        // Get all UTXOs for each MN collateral in one go so that we can reduce locking overhead for cs_main
        // We also do this outside of the below Qt list update loop to reduce cs_main locking time to a minimum
        mnList.ForEachMN(false, [&](const CDeterministicMNCPtr& dmn) {
            CTxDestination collateralDest;
            Coin coin;
            if (clientModel->node().getUnspentOutput(dmn->collateralOutpoint, coin) && ExtractDestination(coin.out.scriptPubKey, collateralDest)) {
                mapCollateralDests.emplace(dmn->proTxHash, collateralDest);
            }
        });
    }

    LOCK(cs_dip3list);

    QString strToFilter;
    ui->countLabelDIP3->setText(tr("Updating..."));
    ui->tableWidgetMasternodesDIP3->setSortingEnabled(false);
    ui->tableWidgetMasternodesDIP3->clearContents();
    ui->tableWidgetMasternodesDIP3->setRowCount(0);

    nTimeUpdatedDIP3 = GetTime();

    auto projectedPayees = mnList.GetProjectedMNPayees(mnList.GetValidMNsCount());
    std::map<uint256, int> nextPayments;
    for (size_t i = 0; i < projectedPayees.size(); i++) {
        const auto& dmn = projectedPayees[i];
        nextPayments.emplace(dmn->proTxHash, mnList.GetHeight() + (int)i + 1);
    }

    std::set<COutPoint> setOutpts;
    if (walletModel && ui->checkBoxMyMasternodesOnly->isChecked()) {
        std::vector<COutPoint> vOutpts;
        walletModel->wallet().listProTxCoins(vOutpts);
        for (const auto& outpt : vOutpts) {
            setOutpts.emplace(outpt);
        }
    }

    mnList.ForEachMN(false, [&](const CDeterministicMNCPtr& dmn) {
        if (walletModel && ui->checkBoxMyMasternodesOnly->isChecked()) {
            bool fMyMasternode = setOutpts.count(dmn->collateralOutpoint) ||
                walletModel->wallet().isSpendable(dmn->pdmnState->keyIDOwner) ||
                walletModel->wallet().isSpendable(dmn->pdmnState->keyIDVoting) ||
                walletModel->wallet().isSpendable(dmn->pdmnState->scriptPayout) ||
                walletModel->wallet().isSpendable(dmn->pdmnState->scriptOperatorPayout);
            if (!fMyMasternode) return;
        }
        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        auto addr_key = dmn->pdmnState->addr.GetKey();
        QByteArray addr_ba(reinterpret_cast<const char*>(addr_key.data()), addr_key.size());
        QTableWidgetItem* addressItem = new CMasternodeListWidgetItem<QByteArray>(QString::fromStdString(dmn->pdmnState->addr.ToString()), addr_ba);
        QTableWidgetItem* statusItem = new QTableWidgetItem(mnList.IsMNValid(dmn) ? tr("ENABLED") : (mnList.IsMNPoSeBanned(dmn) ? tr("POSE_BANNED") : tr("UNKNOWN")));
        QTableWidgetItem* PoSeScoreItem = new CMasternodeListWidgetItem<int>(QString::number(dmn->pdmnState->nPoSePenalty), dmn->pdmnState->nPoSePenalty);
        QTableWidgetItem* registeredItem = new CMasternodeListWidgetItem<int>(QString::number(dmn->pdmnState->nRegisteredHeight), dmn->pdmnState->nRegisteredHeight);
        QTableWidgetItem* lastPaidItem = new CMasternodeListWidgetItem<int>(QString::number(dmn->pdmnState->nLastPaidHeight), dmn->pdmnState->nLastPaidHeight);

        QString strNextPayment = "UNKNOWN";
        int nNextPayment = 0;
        if (nextPayments.count(dmn->proTxHash)) {
            nNextPayment = nextPayments[dmn->proTxHash];
            strNextPayment = QString::number(nNextPayment);
        }
        QTableWidgetItem* nextPaymentItem = new CMasternodeListWidgetItem<int>(strNextPayment, nNextPayment);

        CTxDestination payeeDest;
        QString payeeStr = tr("UNKNOWN");
        if (ExtractDestination(dmn->pdmnState->scriptPayout, payeeDest)) {
            payeeStr = QString::fromStdString(EncodeDestination(payeeDest));
        }
        QTableWidgetItem* payeeItem = new QTableWidgetItem(payeeStr);

        QString operatorRewardStr = tr("NONE");
        if (dmn->nOperatorReward) {
            operatorRewardStr = QString::number(dmn->nOperatorReward / 100.0, 'f', 2) + "% ";

            if (dmn->pdmnState->scriptOperatorPayout != CScript()) {
                CTxDestination operatorDest;
                if (ExtractDestination(dmn->pdmnState->scriptOperatorPayout, operatorDest)) {
                    operatorRewardStr += tr("to %1").arg(QString::fromStdString(EncodeDestination(operatorDest)));
                } else {
                    operatorRewardStr += tr("to UNKNOWN");
                }
            } else {
                operatorRewardStr += tr("but not claimed");
            }
        }
        QTableWidgetItem* operatorRewardItem = new CMasternodeListWidgetItem<uint16_t>(operatorRewardStr, dmn->nOperatorReward);

        QString collateralStr = tr("UNKNOWN");
        auto collateralDestIt = mapCollateralDests.find(dmn->proTxHash);
        if (collateralDestIt != mapCollateralDests.end()) {
            collateralStr = QString::fromStdString(EncodeDestination(collateralDestIt->second));
        }
        QTableWidgetItem* collateralItem = new QTableWidgetItem(collateralStr);

        QString ownerStr = QString::fromStdString(EncodeDestination(dmn->pdmnState->keyIDOwner));
        QTableWidgetItem* ownerItem = new QTableWidgetItem(ownerStr);

        QString votingStr = QString::fromStdString(EncodeDestination(dmn->pdmnState->keyIDVoting));
        QTableWidgetItem* votingItem = new QTableWidgetItem(votingStr);

        QTableWidgetItem* proTxHashItem = new QTableWidgetItem(QString::fromStdString(dmn->proTxHash.ToString()));

        if (strCurrentFilterDIP3 != "") {
            strToFilter = addressItem->text() + " " +
                          statusItem->text() + " " +
                          PoSeScoreItem->text() + " " +
                          registeredItem->text() + " " +
                          lastPaidItem->text() + " " +
                          nextPaymentItem->text() + " " +
                          payeeItem->text() + " " +
                          operatorRewardItem->text() + " " +
                          collateralItem->text() + " " +
                          ownerItem->text() + " " +
                          votingItem->text() + " " +
                          proTxHashItem->text();
            if (!strToFilter.contains(strCurrentFilterDIP3)) return;
        }

        ui->tableWidgetMasternodesDIP3->insertRow(0);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_SERVICE, addressItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_STATUS, statusItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_POSE, PoSeScoreItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_REGISTERED, registeredItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_LAST_PAYMENT, lastPaidItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_NEXT_PAYMENT, nextPaymentItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_PAYOUT_ADDRESS, payeeItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_OPERATOR_REWARD, operatorRewardItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_COLLATERAL_ADDRESS, collateralItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_OWNER_ADDRESS, ownerItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_VOTING_ADDRESS, votingItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, COLUMN_PROTX_HASH, proTxHashItem);
    });

    ui->countLabelDIP3->setText(QString::number(ui->tableWidgetMasternodesDIP3->rowCount()));
    ui->tableWidgetMasternodesDIP3->setSortingEnabled(true);
}

void MasternodeList::on_filterLineEditDIP3_textChanged(const QString& strFilterIn)
{
    strCurrentFilterDIP3 = strFilterIn;
    nTimeFilterUpdatedDIP3 = GetTime();
    fFilterUpdatedDIP3 = true;
    ui->countLabelDIP3->setText(tr("Please wait...") + " " + QString::number(MASTERNODELIST_FILTER_COOLDOWN_SECONDS));
}

void MasternodeList::on_checkBoxMyMasternodesOnly_stateChanged(int state)
{
    // no cooldown
    nTimeFilterUpdatedDIP3 = GetTime() - MASTERNODELIST_FILTER_COOLDOWN_SECONDS;
    fFilterUpdatedDIP3 = true;
}

CDeterministicMNCPtr MasternodeList::GetSelectedDIP3MN()
{
    if (!clientModel) {
        return nullptr;
    }

    std::string strProTxHash;
    {
        LOCK(cs_dip3list);

        QItemSelectionModel* selectionModel = ui->tableWidgetMasternodesDIP3->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();

        if (selected.count() == 0) return nullptr;

        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        strProTxHash = ui->tableWidgetMasternodesDIP3->item(nSelectedRow, COLUMN_PROTX_HASH)->text().toStdString();
    }

    uint256 proTxHash;
    proTxHash.SetHex(strProTxHash);

    auto mnList = clientModel->getMasternodeList();
    return mnList.GetMN(proTxHash);
}

void MasternodeList::extraInfoDIP3_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    UniValue json(UniValue::VOBJ);
    dmn->ToJson(json);

    // Title of popup window
    QString strWindowtitle = tr("Additional information for DIP3 Masternode %1").arg(QString::fromStdString(dmn->proTxHash.ToString()));
    QString strText = QString::fromStdString(json.write(2));

    QMessageBox::information(this, strWindowtitle, strText);
}

void MasternodeList::copyProTxHash_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    QApplication::clipboard()->setText(QString::fromStdString(dmn->proTxHash.ToString()));
}

void MasternodeList::copyCollateralOutpoint_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    QApplication::clipboard()->setText(QString::fromStdString(dmn->collateralOutpoint.ToStringShort()));
}

void MasternodeList::on_pushButtonMasternodeAdd_clicked()
{
    QMessageBox msgPrompt(this);
    msgPrompt.setWindowTitle(tr("Deploy Masternode"));
    msgPrompt.setTextFormat(Qt::RichText);
    msgPrompt.setText(tr("Send Provider Registration Transaction?<br><a href='https://documentchain.org/support/masternodes/'>Please note the instructions</a>"));
    msgPrompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgPrompt.setDefaultButton(QMessageBox::Yes);
    msgPrompt.setCheckBox(new QCheckBox(tr("Create Collateral Transaction")));
    if (msgPrompt.exec() != QMessageBox::Yes) 
        return;
    bool createCollateralTx = msgPrompt.checkBox()->isChecked();

    bool ok;
    QString strIP = QInputDialog::getText(this, tr("Deploy Masternode"), tr("Masternode IP:Port"), QLineEdit::Normal, "", &ok);
    if (!ok) return;
    CService mnaddr;
    if (!(strIP.contains(":") && Lookup(strIP.toStdString().c_str(), mnaddr, 0, false))) {
        QMessageBox::critical(this, tr("Deploy Masternode"), tr("%1 is not a valid IP:Port.").arg(strIP));
        return;
    }

    /* using RPCConsole is easy and prevents duplicate code */
    try {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());
        if (!ctx.isValid()) 
            return;

        auto node = interfaces::MakeNode();
        std::string result;
        std::string filtered;
        QString cmd;
        QJsonDocument jdoc;
        QJsonObject jobj;
        QJsonArray jarr;
        QString addrPayout;
        QString addrOwner;
        QString addrFee;
        CBLSSecretKey blskey;
        QString blsSecret;
        QString blsPublic;
        QString txCollateral;
        int idxCollateral = -1;
        int operatorReward = 0;

        // manual mode, collateral tx was already created
        if (!createCollateralTx) {
            txCollateral = QInputDialog::getText(this, tr("Deploy Masternode"), tr("Collateral tx id"), QLineEdit::Normal, "", &ok);
            if (!ok || txCollateral.isEmpty()) return;

            idxCollateral = -1;
            for (int i = 0; i < 100; i++) {
                cmd = QString("gettxout %1 %2").arg(txCollateral).arg(i);
                RPCConsole::RPCExecuteCommandLine(*node, result, cmd.toStdString(), &filtered);
                jdoc = QJsonDocument::fromJson(QByteArray::fromStdString(result));
                if (jdoc.isNull()) {
                    break;
                }
                jobj = jdoc.object();
                if (jobj.value("value").toInt() == 5000) {
                    idxCollateral = i;
                    jarr = jobj.value("scriptPubKey").toObject().value("addresses").toArray();
                    if (jarr.size() != 1) {
                        QMessageBox::critical(this, "Unexpected", QString("%1 addresses in collateral output.").arg(jarr.size()));
                        return;
                    }
                    addrPayout = jarr.at(0).toString();
                    break;
                }
            }
            if (idxCollateral < 0) {
                QMessageBox::critical(this, tr("Deploy Masternode"), tr("No collateral transaction found."));
                return;
            }
            operatorReward = QInputDialog::getInt(this, tr("Deploy Masternode"), tr("Operator Reward [0-100%]"), 0, 0, 100, 1, &ok);
            if (!ok) return;

            addrOwner = QInputDialog::getText(this, tr("Deploy Masternode"), tr("Owner Address (optional)"), QLineEdit::Normal, "", &ok);
            if (!ok) return;
            if (addrOwner.isEmpty()) {
                RPCConsole::RPCExecuteCommandLine(*node, result, "getnewaddress \"MN " + strIP.toStdString() + " (owner)\"", &filtered);
                addrOwner = QString::fromStdString(result);
                QMessageBox::information(this, "Deploy Masternode", tr("Owner Address %1 created").arg(addrOwner));
             }

            addrFee = QInputDialog::getText(this, tr("Deploy Masternode"), tr("Fee Address"), QLineEdit::Normal, addrPayout, &ok);
            if (!ok) return;

            blsSecret = QInputDialog::getText(this, tr("Deploy Masternode"), tr("BLS Secret (optional)"), QLineEdit::Normal, "", &ok);
            if (!ok) return;
            if (blsSecret.isEmpty()) {
                blskey.MakeNewKey();
                blsSecret = QString::fromStdString(blskey.ToString());
                blsPublic = QString::fromStdString(blskey.GetPublicKey().ToString());
            }
            else {
                auto binKey = ParseHex(blsSecret.toStdString());
                blskey.SetByteVector(binKey);
                if (!blskey.IsValid()) {
                    QMessageBox::critical(this, tr("Deploy Masternode"), tr("Invalid BLS secret key"));
                    return;
                }
                blsPublic = QString::fromStdString(blskey.GetPublicKey().ToString());
            }

            // protx register "collateralHash" collateralIndex "ipAndPort" "ownerAddress" "operatorPubKey" "votingAddress" operatorReward "payoutAddress" "feeSourceAddress"
            cmd = QString("protx register %1 %2 %3 %4 %5 %4 %8 %6 %7")
                          .arg(txCollateral)
                          .arg(idxCollateral)
                          .arg(strIP)
                          .arg(addrOwner)
                          .arg(blsPublic)
                          .arg(addrPayout)
                          .arg(addrFee)
                          .arg(operatorReward);
        }

        // automatic mode
        else {
            QString addrFund = QInputDialog::getText(this, tr("Deploy Masternode"), tr("Fund address to debit"), QLineEdit::Normal, "", &ok);
            if (!ok) return;
            RPCConsole::RPCExecuteCommandLine(*node, result, "listaddressbalances 5000.1", &filtered);
            jdoc = QJsonDocument::fromJson(QByteArray::fromStdString(result));
            if (!jdoc.object().contains(addrFund)) {
                QMessageBox::critical(this, tr("Deploy Masternode"), tr("The fund address requires a balance of at least 5,000.01 DMS"));
                return;
            }

            RPCConsole::RPCExecuteCommandLine(*node, result, "getnewaddress \"MN " + strIP.toStdString() + " (payout)\"", &filtered);
            addrPayout = QString::fromStdString(result);
            RPCConsole::RPCExecuteCommandLine(*node, result, "getnewaddress \"MN " + strIP.toStdString() + " (owner)\"", &filtered);
            addrOwner = QString::fromStdString(result);
            blskey.MakeNewKey();
            blsSecret = QString::fromStdString(blskey.ToString());
            blsPublic = QString::fromStdString(blskey.GetPublicKey().ToString());

            // protx register_fund "collateralAddress" "ipAndPort" "ownerAddress" "operatorPubKey" "votingAddress" operatorReward "payoutAddress" "fundAddress"
            cmd = QString("protx register_fund %1 %2 %3 %4 %3 0 %1 %5")
                          .arg(addrPayout)
                          .arg(strIP)
                          .arg(addrOwner)
                          .arg(blsPublic)
                          .arg(addrFund);
        }

        if (QMessageBox::question(this, tr("Deploy Masternode"), tr("Send protx") + "?\n" + cmd,
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) != QMessageBox::Yes) return;

        RPCConsole::RPCExecuteCommandLine(*node, result, cmd.toStdString(), &filtered);
        QString proregtx = QString::fromStdString(result);

        if (createCollateralTx) {
            txCollateral = proregtx;
            for (int i = 0; i < 2; i++) {
                cmd = QString("gettxout %1 %2").arg(txCollateral).arg(i);
                RPCConsole::RPCExecuteCommandLine(*node, result, cmd.toStdString(), &filtered);
                jdoc = QJsonDocument::fromJson(QByteArray::fromStdString(result));
                if (jdoc.object().value("value").toInt() == 5000) {
                    idxCollateral = i;
                    break;
                }
            }
        }

        QSettings mnk(GUIUtil::boostPathToQString(GetDataDir() / "masternodek.conf"), QSettings::IniFormat);
        QString strOutpoint = QString("%1-%2").arg(txCollateral).arg(idxCollateral);
        mnk.beginGroup("blssecret");
        mnk.setValue(strOutpoint, blsSecret);
        mnk.endGroup();

        /* The update_service transaction is usually sent by the operator and 
           is only possible once the registration transaction has been mined */
        if (operatorReward > 0) {
            QInputDialog::getMultiLineText(this, tr("Deploy Masternode"),
                "Set Operator Reward Address",
                QString("With the following protx you or the operator can set the operator reward address. ")
              + QString("Please wait until the registration transaction just sent is mined.\n\n")
              + QString("protx update_service %1 %2 %3 \"enter-operator-reward-address\" %4")
                        .arg(proregtx)
                        .arg(strIP)
                        .arg(blsSecret)
                        .arg(addrFee));
        }

        QMessageBox msgPrompt(this);
        msgPrompt.setWindowTitle(tr("Deploy Masternode"));
        msgPrompt.setText(tr("Done. As soon as the provider register transaction, your masternode is ready."));
        msgPrompt.setStandardButtons(QMessageBox::Ok);
        msgPrompt.setCheckBox(new QCheckBox(tr("Show dms.conf suggestion")));
        msgPrompt.exec();
        bool showDmsconf = msgPrompt.checkBox()->isChecked();

        if (showDmsconf) {
            QInputDialog::getMultiLineText(this, tr("Deploy Masternode"),
                "Suggestion for dms.conf on masternode server",
                QString("ssh %1 -l (username)\n").arg(strIP.split(':')[0])
              + QString("nano .dmscore/dms.conf\n")
              + QString("dms.conf:\n\n")
              + QString("rpcuser=dmsrpcuser\n")
              + QString("rpcpassword=mySeCrEtPw-TODO\n")
              + QString("rpcallowip=127.0.0.1\n")
              + QString("rpcport=41320\n")
              + QString("server=1\n")
              + QString("listen=1\n")
              + QString("daemon=1\n")
              + QString("maxconnections=125\n")
              + QString("masternodeblsprivkey=%1\n").arg(blsSecret)
              + QString("externalip=%1").arg(strIP));
        }
    }
    catch (const UniValue& objError) {
        std::string msgErr = find_value(objError, "message").get_str();
        QMessageBox::critical(this, tr("Deploy Masternode"), QString::fromStdString(msgErr));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Deploy Masternode"), QString::fromStdString(e.what()));
    }
    catch (...) {
        QMessageBox::critical(this, tr("Deploy Masternode"), "Unknown error");
    }
}

void MasternodeList::sendProtxUpdateService_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    bool ok;
    bool confExists = true;
    QString strOutpoint = QString::fromStdString(dmn->collateralOutpoint.ToStringShort());
    QString strIP = QString::fromStdString(dmn->pdmnState->addr.ToString());
    QString strProTx = QString::fromStdString(dmn->proTxHash.ToString());
    QString strBlsSecret = "";

    QSettings mnk(GUIUtil::boostPathToQString(GetDataDir() / "masternodek.conf"), QSettings::IniFormat);
    mnk.beginGroup("blssecret");
    strBlsSecret = mnk.value(strOutpoint).toString();

    if (strBlsSecret.isEmpty()) {
        confExists = false;
        // read "dip3-masternodes.ini" from v0.13
        QSettings mnini(GUIUtil::boostPathToQString(GetDataDir() / "dip3-masternodes.ini"), QSettings::IniFormat);
        QStringList groups = mnini.childGroups();
        for (int i = 0; i < groups.size(); ++i) {
            mnini.beginGroup(groups.at(i));
            if (mnini.value("ipAndPort").toString() == strIP) {
                strBlsSecret = mnini.value("operatorPrivKey").toString();
                break;
            }
            mnini.endGroup();
        }

        // enter BLS secret if not found
        if (strBlsSecret.isEmpty()) {
            strBlsSecret = QInputDialog::getText(this, tr("Update Service"), tr("BLS Secret"), QLineEdit::Normal, strBlsSecret, &ok);
            if (!ok) {
                return;
            }
        }
    }

    strIP = QInputDialog::getText(this, tr("Update Service"), tr("IP:Port"), QLineEdit::Normal, strIP, &ok);
    if (!ok) {
        return;
    }

    CService mnaddr;
    ok = false;
    if (Lookup(strIP.toStdString().c_str(), mnaddr, 0, false)) {
      //ok = g_connman->FindNode(mnaddr);   TODO : POSE_BAN node
        if (!ok) {
            LogPrintf("DMSDEBUG FindNode=false\n");
            // TODO : can fail on testnet with non-default port
            g_connman->OpenMasternodeConnection(CAddress(mnaddr, NODE_NETWORK));
            ok = (g_connman->IsConnected(CAddress(mnaddr, NODE_NETWORK), CConnman::AllNodes));
        }
    }
    if (!ok) {
        if (QMessageBox::warning(this, tr("Update Service"), tr("Couldn't connect to masternode %1").arg(strIP),
            QMessageBox::Ok | QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) return;
    }

    std::string result;
    std::string cmd;
    std::string filtered;
    cmd = strprintf("protx update_service %s %s %s", 
        strProTx.toStdString(), strIP.toStdString(), strBlsSecret.toStdString());
    if (QMessageBox::question(this, tr("Update Service"), tr("Send protx") + "?\n" + QString::fromStdString(cmd),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) != QMessageBox::Yes) return;

    try {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());
        if (!ctx.isValid()) 
            return;

        auto node = interfaces::MakeNode();
        RPCConsole::RPCExecuteCommandLine(*node, result, cmd, &filtered);

        QMessageBox::information(this, tr("Update Service"),
            tr("Done.") + QString("\n%1").arg(QString::fromStdString(result)));

        if (!confExists) {
            mnk.setValue(strOutpoint, strBlsSecret);
        }
    }
    catch (...) {
        QMessageBox::critical(this, "Error", "Error sending\n" + QString::fromStdString(cmd));
    }
}

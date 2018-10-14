// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
/*
{  
   "status":{  
      "code":0,
      "name":"OK"
   },
   "them":[  
      {  
         "id":"a1da7f927134c10ec6bec60000dd8f19",
         "public_keys":{  
            ]
         },
         "cryptocurrency_addresses":{  
            "bitcoin":[  
               {  
                  "address":"34iDgKC7FbDs1EqrZ2tYnvgb65bUHbdCoG",
                  "sig_id":"33fdeac92720bd9a0123ed15b0439cb4e56f57cf578f20080aceb9536d472b130f"
               }
            ]
         },
         "sigs":{  
            "last":{  
            }
         }
      }
   ]
}      

*/

#include <qt/openuridialog.h>
#include <qt/forms/ui_openuridialog.h>

#include <qt/guiutil.h>
#include <qt/walletmodel.h>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QVariantList>
#include <QTextStream>

#include <key_io.h>
#include <validation.h> // For strMessageMagic
#include <wallet/wallet.h>
#include <QUrl>

OpenURIDialog::OpenURIDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenURIDialog)
{
    ui->setupUi(this);
    ui->uriEdit->setPlaceholderText("bitcoin: or keybase username");
}

OpenURIDialog::~OpenURIDialog()
{
    delete ui;
}


QString OpenURIDialog::getURI()
{
    // start code
    QString keybase_username = ui->uriEdit->text();
    //https://keybase.io/_/api/1.0/user/lookup.json?usernames=vahagn&fields=cryptocurrency_addresses
    QString _url = QString("https://keybase.io/_/api/1.0/user/lookup.json?usernames=%1&fields=cryptocurrency_addresses").arg(keybase_username);
    if (keybase_username.length()>0)
    {
        
        QNetworkAccessManager * manager = new QNetworkAccessManager();
        QNetworkRequest request;
        
        request.setUrl(QUrl(_url));
        manager->get(request);
        
        QObject::connect(manager, &QNetworkAccessManager::finished,
            this, [=](QNetworkReply *reply) {
             QString response = reply->readAll();
            
            

                QJsonDocument json_doc = QJsonDocument::fromJson(response.toUtf8());
                QJsonObject root_obj = json_doc.object();
                QJsonValue _them = root_obj.value("them");

                if(_them.isArray()){
                    QJsonArray _them_arr = _them.toArray();
                    for(int i = 0; i < _them_arr.count(); i++){
                                QString _addr = _them_arr.at(i).toObject()
                                                .value("cryptocurrency_addresses")
                                                .toObject()
                                                .value("bitcoin")
                                                .toArray()
                                                .at(0).toObject()
                                                .value("address")
                                                .toString();
                                
                                if(!_addr.isNull()){
                                    
                                SendCoinsRecipient rcp;
                                if(GUIUtil::parseBitcoinURI("bitcoin:"+_addr, &rcp))
                                {
                                    if(IsValidDestination(DecodeDestination(_addr.toStdString()))) //check address
                                    {
                                        ui->uriEdit->setText("bitcoin:"+_addr);
                                        QDialog::accept();
                                    }
                                } else {
                                    ui->uriEdit->setValid(false);
                                }
                                }
                                
                            }
                    
                }    
            }
        );
    }
    // end code
    
     return ui->uriEdit->text();
}

void OpenURIDialog::accept()
{
    SendCoinsRecipient rcp;
    if(GUIUtil::parseBitcoinURI(getURI(), &rcp))
    {
        /* Only accept value URIs */
        QDialog::accept();
    } else {
        ui->uriEdit->setValid(false);
    }
}

void OpenURIDialog::on_selectFileButton_clicked()
{
    QString filename = GUIUtil::getOpenFileName(this, tr("Select payment request file to open"), "", "", nullptr);
    if(filename.isEmpty())
        return;
    QUrl fileUri = QUrl::fromLocalFile(filename);
    ui->uriEdit->setText("bitcoin:?r=" + QUrl::toPercentEncoding(fileUri.toString()));
}

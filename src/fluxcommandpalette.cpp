#include "fluxcommandpalette.h"
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

FluxCommandPalette::FluxCommandPalette(QWidget* parent):QDialog(parent){setWindowTitle("Command Palette");setModal(true);resize(640,460);auto*lay=new QVBoxLayout(this);m_search=new QLineEdit; m_search->setPlaceholderText("Search commands…");m_list=new QListWidget;lay->addWidget(m_search);lay->addWidget(m_list,1);connect(m_search,&QLineEdit::textChanged,this,[this]{rebuild();});connect(m_list,&QListWidget::itemActivated,this,[this](QListWidgetItem*i){const int idx=i?i->data(Qt::UserRole).toInt():-1;if(idx>=0&&idx<m_commands.size()){if(m_commands[idx].invoke)m_commands[idx].invoke();accept();}});m_search->setFocus();}
void FluxCommandPalette::setCommands(const QVector<FluxCommand>&commands){m_commands=commands;rebuild();}
void FluxCommandPalette::rebuild(){if(!m_list)return;m_list->clear();const QString q=m_search?m_search->text().trimmed():QString();for(int i=0;i<m_commands.size();++i){const auto&c=m_commands[i];if(!q.isEmpty()&&!c.title.contains(q,Qt::CaseInsensitive)&&!c.id.contains(q,Qt::CaseInsensitive)&&!c.shortcut.contains(q,Qt::CaseInsensitive))continue;auto*item=new QListWidgetItem(c.title+(c.shortcut.isEmpty()?QString():QStringLiteral("    ")+c.shortcut),m_list);item->setData(Qt::UserRole,i);}}

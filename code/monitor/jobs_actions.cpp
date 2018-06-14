#include "jobs_actions.h"

void jobs_actions::init(){

    actions();

}

void jobs_actions::actions(){

	// signal del drop a la lista
	//connect( jobsList, &QTreeWidget::itemDropped, this, submitDrop );
	//------------------------------------

	connect( jobsList, &QTreeWidget::itemDoubleClicked, this, &jobs_actions::jobModify );
	connect( jobsList, &QTreeWidget::customContextMenuRequested, this, &jobs_actions::job_popup );

	// Job Acciones
	connect( deleteAction, &QAction::triggered, this, &jobs_actions::itemDelete );

	deleteAction->setIcon( QIcon("../../icons/delete.png") );
	deleteAction->setShortcut( QString("Del"));

	connect( jobSuspendAction, &QAction::triggered, this,[this](){
		QString ask = "Sure you want to suspend the job?";
		QString tile = "Job Suspend";
		string action = "suspend";
		jobMessage( &jobs_actions::jobAction, action, ask,  tile, this );
	});

	connect( jobRestartAction, &QAction::triggered, this,[this](){
		QString ask = "Sure you want to restart the job?";
		QString tile = "Job Restart";
		string action = "restart";
		jobMessage( &jobs_actions::jobAction, action, ask,  tile, this );
	});

	jobSuspendAction->setIcon(QIcon("../../icons/pause.png"));

	connect( jobResumeAction, &QAction::triggered, this, [this](){
		jobAction("resume");
	});
	jobResumeAction->setIcon(QIcon("../../icons/play3.png"));

	connect( jobUnlockServersAction, &QAction::triggered, this, [this](){
		jobAction("unlock");		
	});

	connect( jobLogAction, &QAction::triggered, this, &jobs_actions::jobShowLog);
	jobLogAction->setShortcut( QString("L") );

	connect( jobModifyAction, &QAction::triggered, this, &jobs_actions::jobModify);
	jobModifyAction->setShortcut( QString("M") );

	//-----------------------------------------------------------------------
}

void jobs_actions::job_popup(){

	auto selected = jobsList->selectedItems();
	if ( not selected.empty() ){

		QMenu *menu = new QMenu( monitor );

		menu->addAction( jobSuspendAction );			
		menu->addAction( jobResumeAction );
		menu->addAction( deleteAction );
		menu->addAction( jobRestartAction );

		menu->addSeparator();
		menu->addAction( jobLogAction );
		menu->addAction( jobUnlockServersAction );

		menu->addSeparator();

		menu->addAction( jobShowRenderFolderAction );
		menu->addAction( jobModifyAction );

		menu->popup( QCursor::pos() );
	}
}

void jobs_actions::jobShowLog(){

	auto selected = jobsList->selectedItems();
	if ( not selected.empty() ){

	    // log plaintText
		plainLog = new QPlainTextEdit();
		plainLog->setObjectName( "style1" );
		plainLog->setReadOnly( true );
		plainLog->setCenterOnScroll( true );
		QVBoxLayout *hbox = new QVBoxLayout();
		hbox->addWidget( plainLog );
		QWidget *widget = new QWidget();
		widget->setLayout( hbox );
		log_dock->setWidget( widget );
		//-------------------------------

		string job_name = selected[0]->text(0).toStdString();

		// encuentra job seleccionado en la jobs recividos
		json job;
		for ( auto _job : shared->jobs ) if ( _job["name"] == job_name ) job = _job;
		//--------------------------------------

		for ( auto task : job["task"] ){
			log_server = task["server"];
			log_server = split( log_server, ":" )[0];

			if ( not ( log_server == "...") ){

				if ( log_thread->isRunning() )
					log_thread->terminate();

				jobLogUpdate();
				log_thread = qthread( &jobs_actions::jobLogUpdate, this );		
				break;

			}
			else {

				if ( log_thread->isRunning() )
					log_thread->terminate();

				plainLog->setPlainText( "The jobs has not yet rendered" );
				
			}

		}

		log_dock->show();

	}
}



void jobs_actions::jobLogUpdate(){

	string result;
	// si el server esta en la lista de server copia el log a result
	for ( auto server : shared->servers ){
		if (  server["name"] == log_server ){ 
			result = server["log"];
			break;
		}
	} //---------------------------------------------

	plainLog->setPlainText( QString::fromStdString(result) );
}

void jobs_actions::jobModify(){
	debug("jobs_actions::jobModify.");

	uiJobOptions->show();

	// Recive los servers del jobs que estan en el manager
	auto selected = jobsList->selectedItems();
	firstJobItem = selected[0];

	if ( not selected.empty() ){

		string job_name = selected.takeLast()->text(0).toStdString();

		json pks = {{ job_name, "options", "read" }};
		pks = { pks, "jobOptions" };
		pks = tcpClient( managerHost, 7000, pks, 3 );

		//-----------------------------------------

		vector <string> serverExist; 
		for ( auto s : pks[0] ) serverExist.push_back(s);

		vector <string> serverGroupExist; 
		for ( auto sg : pks[1] ) serverGroupExist.push_back(sg);
		
		int priority = pks[2]; 
		string comment = pks[3];
		int instances = pks[4];
		int task_size = pks[5];
		string _job_name = pks[6];
		int first_frame = pks[7];
		int last_frame = pks[8];


		uiJobOptions->priority->setText( QString::number( priority ) );
		uiJobOptions->firstFrame->setText( QString::number( first_frame ) );
		uiJobOptions->lastFrame->setText( QString::number( last_frame ) );
		uiJobOptions->jobName->setText( QString::fromStdString( _job_name ) );
		uiJobOptions->taskSize->setText( QString::number( task_size ) );
		uiJobOptions->comment->setText( QString::fromStdString( comment ) );
		uiJobOptions->instances->setText( QString::number( instances ) );

		string jobServerAsignName = job_name;

		uiJobOptions->serverAsign->setSelectionMode( QAbstractItemView::ExtendedSelection ); 
		uiJobOptions->serverAsign->setIndentation(0);     
		uiJobOptions->serverAsign->setColumnWidth(0,100);
		uiJobOptions->serverAsign->setColumnWidth(1,50);

		uiJobOptions->serverGroupAsign->setSelectionMode( QAbstractItemView::ExtendedSelection ); 
		uiJobOptions->serverGroupAsign->setIndentation(0);     
		uiJobOptions->serverGroupAsign->setColumnWidth(0,100);
		uiJobOptions->serverGroupAsign->setColumnWidth(1,50);

		uiJobOptions->serverAsign->clear();
		for (int i = 0; i < serverList->topLevelItemCount(); ++i){

			auto _item = serverList->topLevelItem(i); 
			QString name = _item->text(0);

			QTreeWidgetItem *item = new QTreeWidgetItem();

			item->setText(0,name);

			item->setCheckState( 0, Qt::Unchecked );

			if ( in_vector( name.toStdString(), serverExist ) ){
				item->setCheckState(0, Qt::Checked );
			}

			uiJobOptions->serverAsign->addTopLevelItem(item);
		}

		uiJobOptions->serverGroupAsign->clear();
		for (int i = 0; i < groupList->topLevelItemCount(); ++i){

			auto _item = groupList->topLevelItem(i); 
			QString name = _item->text(2);

			QTreeWidgetItem *item = new QTreeWidgetItem();

			item->setText(0,name);

			item->setCheckState( 0, Qt::Unchecked );
			if ( in_vector( name.toStdString(), serverGroupExist ) ){
				item->setCheckState(0, Qt::Checked );
			}
			uiJobOptions->serverGroupAsign->addTopLevelItem( item );
		}

		connect( uiJobOptions->dialog, &QDialogButtonBox::accepted, this, &jobs_actions::jobOptionsOk );	
		connect( uiJobOptions->dialog, &QDialogButtonBox::rejected, [this](){
			uiJobOptions->hide();
		});

	}
}

void jobs_actions::jobOptionsOk(){
	//Obtiene servideros chekeados para el job seleccionado
	json machines;

	for (int i = 0; i < uiJobOptions->serverAsign->topLevelItemCount(); ++i){
		auto item = uiJobOptions->serverAsign->topLevelItem(i); 
		string name = item->text(0).toStdString();

		if ( item->checkState(0) ){
			machines.push_back(name);
		}
	}
	//--------------------------------------------

	//Obtiene grupos chekeados para el job seleccionado
	json group;
	for (int i = 0; i < uiJobOptions->serverGroupAsign->topLevelItemCount(); ++i){
		auto item = uiJobOptions->serverGroupAsign->topLevelItem(i); 
		string name = item->text(0).toStdString();

		if ( item->checkState(0) ){
			group.push_back(name);
		}
	}

	//--------------------------------------------

	int priority = uiJobOptions->priority->text().toInt();
	int first_frame = uiJobOptions->firstFrame->text().toInt();
	int last_frame = uiJobOptions->lastFrame->text().toInt();
	int task_size = uiJobOptions->taskSize->text().toInt();
	string comment = uiJobOptions->comment->text().toStdString();
	string _job_name = uiJobOptions->jobName->text().toStdString();
	
	int instances = uiJobOptions->instances->text().toInt();
	if ( instances > 16 ){
		instances = 16;
	}

	json pks;
	auto selected = jobsList->selectedItems();
	selected.push_back( firstJobItem );

	for ( auto item : selected ){

		string job_name = item->text(0).toStdString();
		json options = { machines, group, priority, comment, instances, first_frame, last_frame, task_size, _job_name };
		pks.push_back( { job_name, options, "write" } );

	}
	pks = { pks, "jobOptions" };

	tcpClient( managerHost, 7000, pks, 3 );

	uiJobOptions->hide();
}

void jobs_actions::jobMessage( void ( jobs_actions::*funtion )( string ), string action, 
						 QString ask, QString tile, jobs_actions *_class ){

	auto selected = jobsList->selectedItems();

	if ( not selected.empty() ){

		QMessageBox::StandardButton reply;
		reply = QMessageBox::question( monitor, tile, ask, QMessageBox::Yes | QMessageBox::No );
		if (reply == QMessageBox::Yes) {
			( _class->*funtion )( action );
		}

	}

}

void jobs_actions::jobDeleteStart( string action ){
	debug("jobs_actions::jobDeleteStart.");

	auto root = jobsList->invisibleRootItem();
	auto selected = jobsList->selectedItems();

	json pks;

	for ( auto item : selected ){

		string job_name = item->text(0).toStdString();
		os::remove( "../../log/trayIcon/" + job_name );

		pks.push_back( { job_name, action } );

		root->removeChild( item );

	}

	pks = { pks, "jobAction" };

	tcpClient( managerHost, 7000, pks, 3 );
}

void jobs_actions::jobAction( string action ){
	shared->stopUpdate = true;

	auto selected = jobsList->selectedItems();

	json pks;
	for ( auto item : selected ){
		string job_name = item->text(0).toStdString();
		pks.push_back( { job_name, action } ); 
	}
	pks = { pks, "jobAction" };

	tcpClient( managerHost, 7000, pks, 3 );	
}

void jobs_actions::itemDelete(){

	shared->stopUpdate = true;

	QString ask = "Sure you want to delete the job?";
	QString tile = "Job Delete";
	string action = "delete";
	jobMessage( &jobs_actions::jobDeleteStart, action, ask,  tile, this );

}
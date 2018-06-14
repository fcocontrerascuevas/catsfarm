#include "manager.h"




void manager::update_all(){
	while(1){ 

		update_jobs();
		container_save();
		update_server();
		update_group();
		
		sleep(1);
	}

}

void manager::container_save(){

	static int sec;
	sec++;

	// guarda informacion a json cada 10 seg
	if ( sec > 10 ){
		jwrite( "../../etc/info.json", struct_to_json() ); 
		sec = 0;
	} 

	


}

json manager::update_server_thread( json recv ){
	debug("manager::update_server_thread: update.");

	if ( not recv.empty() ){  
		string name = recv[0];
		string host = recv[1];
		int cpu = recv[2];
		int ram = recv[3];
		int temp = recv[4];
		string system = recv[5];
		bool vbox = recv[6];
		int ram_total = recv[7];
		float ram_used = recv[8];
		int cpu_cores = recv[9];
		string log = recv[10];
		


		int response_time = time(0);

		if ( not is_struct( servers, name ) ){

			vector <inst_struct*> instances;

			for (int i = 0; i < 16; ++i){
			    inst_struct *ins = new inst_struct;
	            ins->index = i;
	            ins->status = 0;
	            ins->reset = 0;
	            ins->job_task = "...";
			    instances.push_back(ins);
			}

	        server_struct *server = new server_struct;

	        server->name = name;
	        server->status = "idle";
	        server->host = host;
	        server->system = system;
	        server->cpu = cpu;
	        server->cpu_cores = cpu_cores;
	        server->ram = ram;
	        server->ram_used = ram_used;
	        server->ram_total = ram_total;
	        server->temp = temp;
	        server->vbox = vbox;
	        server->response_time = response_time;
	        server->instances = instances;
	        server->max_instances = 1;
	        server->sshUser = "root";
	        server->sshPass = "vfx";
	        server->schedule = "0-0";
	        server->schedule_state_0 = 1;
	        server->schedule_state_1 = 1;
	        server->log = log;
	        servers.push_back( server );

	    }

		else{
	        auto server = find_struct( servers, name );

	        server->ram_used = ram_used;
	        server->cpu = cpu;
	        server->ram = ram;
	        server->temp = temp;
	        server->host = host;
	        server->vbox = vbox;
	        server->log = log;
	        server->response_time = response_time;

	    }


	}
	
	return preferences;
}

void manager::update_server(){
	debug("manager::update_server: update.");

	int t1 = time(0);

	json serverList;

	for ( auto server : servers ){
		serverList.push_back( server->name );

		// set status server
		int t2 = server->response_time;
		int response_time = t1 - t2;

		// si el porcentaje de la ram es mayor a 90 %, el tiempo de espera es mas para que no quede aucente le servidor
		int response;
		if ( server->ram > 90 ){ response = 120; }
		else { response = 30; }
		//--------------------------------------------------------------------------------------------------

		if ( response_time > response ){  // si no responde en 10 segundo queda ausente
			server->status = "absent";
		}
		else{ server->status = "idle"; }

        for (int i = 0; i < server->max_instances; ++i){			
			if ( server->instances[i]->status == 3 )
				threading( [=](){
		            sleep(7);
		            if ( server->instances[i]->status == 3 ){
			            server->instances[i]->status = 0;
			            server->instances[i]->job_task = "...";
                    }
				});                 
        }
		//--------------------------------

		//schedule server
        if ( not iTime( server->schedule ) ){
			if ( server->schedule_state_0 ){
				serverSetState( server, false );
				server->schedule_state_0 = false;
				server->schedule_state_1 = true;
            }
        }

		else{
			if ( server->schedule_state_1 ){

				serverSetState( server, true );
				server->schedule_state_1 = false;
				server->schedule_state_0 = true;
			}
		}

		//-----------------------------

    }

	preferences["servers"] = serverList;
}

bool manager::iTime( string schedule ){

    int start = 0, end = 0;
	try{
		auto range = split( schedule, "-" );
        start = stoi( range[0] );
        end = stoi( range[1] ) - 1;
	}
	catch(exception& e){}

	// si no esta en el rango de tiempo inTime es 0
	int hora =  stoi( currentDateTime(6) );
	if ( start > end ){
		if ( ( hora >= start ) or ( hora <=end ) ){ return true; }
		else{ return false; }
	}
	else{
		if ( ( start <= hora ) <= end ){ return true; }
		else{ return false; }
	}
	//----------------------------------------------

}

void manager::update_group(){
	debug("manager::update_group: update.");
	// Obtiene todos los grupos que estan activos
	vector <string> groups_used;
	for ( auto job : jobs ){
		if ( job->active_task ){
			for ( string sg : job->server_group ){
				groups_used.push_back(sg);
			}
	    }
	}
	//------------------------------------------------
	json grouplist;

	
    for ( auto group : groups ){
		grouplist.push_back( group->name );
		int totaMachine = 0, activeMachine = 0;

        for ( auto _server : group->server ){

			if ( is_struct( servers, _server->name ) ){      
				auto server = find_struct( servers, _server->name );

				totaMachine++;						
				if ( server->status == "absent" ){ _server->status = false; }
				else{ 							
					_server->status = true;
					activeMachine++;
				}
            }
			else{
				erase_by_name( group->server, _server->name );
			}
        }
        bool server_status;
		if ( in_vector( group->name, groups_used ) ){ server_status = true;}
		else{ server_status = false; }

		group->status = server_status;
		group->totaMachine = totaMachine;
		group->activeMachine = activeMachine;

    }
	
	preferences[ "groups" ] = grouplist;
	
}

void manager::update_jobs(){
	debug("manager::update_jobs: update.");

	for ( auto job : jobs ){

		int waiting = job->tasks-( job->suspended_task + job->progres + job->active_task );

		job->waiting_task = waiting;

		if ( job->failed_task ){
			threading([=](){
		        job->status = "Failed";
		        sleep(7);
		        job->status = "Queue";
			});

			job->failed_task = 0;
        }
		else{
			if ( ( job->status == "Suspended" ) or ( job->status == "Completed" ) or ( job->status == "Failed" ) or ( job->status =="Concatenate" ) ){}
			else if( job->active_task ){ job->status = "Rendering..."; }
			else{ job->status = "Queue"; }
        }

		if ( job->active_task ){
			bool last_active = job->timer_last_active;
			job->timer_last_active = true;

			// si es que en la vuelta anterior del proceso fue active entonces suma el tiempo
			float timer1, timer2;
			if ( last_active ){
				timer1 = stof( job->timer );
				if ( job->timer == "..." ){
					timer1 = time(0);
				}
				job->timer = to_string( time(0) );
			}
			else{
				timer1 = time(0);
				job->timer = to_string( time(0) );
            }

			timer1 = time(0) - timer1;
			
			
			if ( job->timer2 == "..." ) timer2 = 0;
			else timer2 = stof( job->timer2 );

			
			timer2 = timer1 + timer2;
			job->timer2 = to_string( timer2 );
			//--------------------------------------
			float progres = job->progres;
			//-----------------------------------------------------				
			float eto = job->estimated_time_second;
			//-----------------------------------------------------
            float estimated_time;
			if ( progres ){					
				if ( not ( progres == job->old_p ) ){
					estimated_time =  ( ( job->tasks * timer2 ) / progres ) - timer2;
					job->old_p = progres;
				}

				else{
					if ( not ( eto < 1 ) ){
						estimated_time = eto - 1;
					}
					else{
						estimated_time = 0;
					}
			    }

			}
			else{
				estimated_time = 0;
			}
			job->estimated_time_second = estimated_time;
			job->estimated_time = secToTime( estimated_time );
			job->total_render_time = secToTime( timer2 );
        }
		else{
			job->timer_last_active = false;
			job->estimated_time = "...";
        }

    }
}
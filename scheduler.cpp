//
//  scheduler.cpp
//  Scheduler
//
//  Created by Jessie on 13/03/2018.
//  Copyright © 2018 Jessie. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <vector>
#include <list>
#include <limits>
#include <algorithm>

using namespace std;

vector<int> randvals;
static int ofs;
typedef enum {CREATED, RUN_TO_BLOCK, TRANS_TO_READY,READY_TO_RUN, BLOCK_TO_READY,RUN_TO_READY, DONE} State;

//Process
class process{
    public:

    int at ; //Arrival time
    int tc;  //Total CPU Time
    int cb ;  //CPU BURST Time
    int io ;  // IO Burst Time
    int pid ; //process id
    int cw ;  //CPU waiting time(time in Ready state)
    int ft  ; //finishing time
    int tt ;  //turnaround time(finishing time - arrival time)
    int it ;  //time in blocked state
    int runtime ; //the execution time a process in cpu till now
   // int begin_t; //begin processing
    int s_pri;//static priority
    int d_pri;//dynamic priority
    int remain_burst_t;  //remaining cpu burst time
    //int ready_t;
    
    void new_process(int processId , int arr_t , int total_t , int cpu_burst , int io_burst, int prio ){
        pid = processId;
        at = arr_t;
        tc = total_t;
        cb = cpu_burst;
        io = io_burst;
        s_pri = prio;
        d_pri = s_pri-1;
        //begin_t = -1;
        tt = 0;
        it =0;
        cw = 0;
        remain_burst_t = -1;
        runtime = 0;
        
        
    }
};

//Event
class event{
    public:
    int related_processid; // related process id
    int created_t; //time created
    int timestamp;
    State state;
};



//des
class simulator{
    
public:
    vector<event> eventqueue;
    bool isEmpty(){
        return eventqueue.size()==0;
    }
    
    void add_event(event e){
        int i=0;
       while(i<eventqueue.size()&&e.timestamp >= eventqueue[i].timestamp){
          i++;
    }
        eventqueue.insert(eventqueue.begin()+i,e);
    }
    
    event get_event(){
            event e = eventqueue.front();
            eventqueue.erase(eventqueue.begin());
        return e;

    }
    event create_event(State sta,int processID, int currenttime, int duration){
        event e;
        e.state = sta;
        e.timestamp = currenttime + duration;
        e.created_t = currenttime;
        e.related_processid = processID;
        return e;
    }
    int get_next_time() {
        if(isEmpty()) return -1;
        return eventqueue.front().timestamp;
    }
    
    
};


//Scheduler
class scheduler{
    
    
    public:
    vector<process *> readyqueue;
    virtual void add_process(process * p,bool quant)=0;
    virtual process  * get_next_process()=0;
    virtual bool isEmpty()=0;
};

//Scheduler first come first serve
class FCFS : public scheduler  {
    public:
    void add_process(process * p, bool quant){
        readyqueue.push_back(p);
    }
    process * get_next_process(){
        
        process * p;
       
            p = readyqueue.front();
            readyqueue.erase(readyqueue.begin());
                return p;
        }
    
    
    
    bool isEmpty(){
        return readyqueue.size()==0;
    }
};

//Scheduler last come first serve
class LCFS: public scheduler {
    public:
    void add_process(process *p, bool quant){
        readyqueue.push_back(p);
    }
    process *get_next_process(){
        process *p;
            p = readyqueue.back();
        readyqueue.pop_back();

        return p;
    }
    bool isEmpty(){
        return readyqueue.size()==0;
    }
};

//scheduler shortest remaining time job first
class SJF : public scheduler {
    
    
    public:
    void add_process(process *p, bool quant){
        
        int i = 0;
        while (i < readyqueue.size()&&(p->tc- p->runtime) >=(readyqueue[i]->tc-readyqueue[i]->runtime)){
            i++;
            }
        readyqueue.insert(readyqueue.begin()+i,p);
    }
    
    process * get_next_process()
    {
        process *p;
            p = readyqueue.front();
            readyqueue.erase(readyqueue.begin());
        return p;
    }
    bool isEmpty(){
        return readyqueue.size()==0;
    }
    
};

//scheduler RR

class RR : public scheduler{
    
    public:
    void add_process(process *p, bool quant){
        readyqueue.push_back(p);
    }
    
    process *get_next_process(){
        process *p;
      
            p = readyqueue.front();
            readyqueue.erase(readyqueue.begin());
        return p;
    }
    bool isEmpty(){
        return readyqueue.size()==0;
    }
    
};

//priority scheduling
class PRIO : public scheduler {
    public:
    vector<process *>active;
    vector<process *>expire;

    void add_process(process *p, bool quant){
        if(p->d_pri==-1)  {
           expire.push_back(p) ;
                    }
        else{
            active.push_back(p);
            }
  }
    
    process *get_next_process(){
        if(active.size()==0){
            active.swap(expire);
        }
        process *p;
        int tmp = active[0]->d_pri;
        int index = 0;
        for(int i=1;i<active.size();i++){
            if(active[i]->d_pri>tmp){
                tmp =active[i]->d_pri;
                index=i;
            }
        }
        p=active[index];
        active.erase(active.begin()+index);
        return p;
    }
    bool isEmpty(){
        return active.size()==0&&expire.size()==0;
    }
};
 

//Read rfile
void read_rfile(string rfilename){
    ifstream rfile;
    rfile.open(rfilename.c_str());
    string num;
    while(rfile>>num) {
        randvals.push_back(atoi(num.c_str()));
    }
    rfile.close();
}

//Get the random number
int myrandom(int burs){
    if (ofs>=randvals.size()){
        ofs = 0;
    }
    int random = 1 + (randvals[++ofs] % burs);
    return random;
}

//Generate random io burst and cpu burst
int getRandom(event e, int ofs, process *p){
    int bur = 0;
    if (e.state == READY_TO_RUN){
        bur = myrandom(p->cb);
    }
    else if (e.state == RUN_TO_BLOCK){
        bur = myrandom(p->io);
    }
    return bur;
}

//Read the input file and create process list
vector<process> plist;
vector<process> readFile(string filename){
    int PID = 0;
    int AT, TC, CB, IO;
    ifstream infile;
    infile.open(filename.c_str());
    string line;
    while (getline(infile, line)){
        stringstream word(line);
        word >> AT >> TC >> CB >> IO;
        int Prio = myrandom(4);
        process p;
        p.new_process(PID, AT, TC, CB, IO,Prio);
        PID ++;
        plist.push_back(p);
    }
    infile.close();
    return plist;
}

void output(string t,int quan, int curr_time, int cpu_total, int io_total,int turnaround_total, int cw_total ){
    cout << t;
    if (t == "RR" || t == "PRIO"){
        cout << " "<< quan << endl;
    }
    else{cout<<endl;}

    for(int i = 0; i < plist.size(); i++){
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               i,
               plist[i].at,
               plist[i].tc,
               plist[i].cb,
               plist[i].io,
               plist[i].s_pri,
               plist[i].ft,
               plist[i].tt,
               plist[i].it,
               plist[i].cw);
    
    }
    double cpu_untilization = double(cpu_total * 100/double(curr_time));
    double io_utilization = double(io_total * 100/double(curr_time));
    double avg_turnaround = double(turnaround_total / double(plist.size()));
    double avg_cpu_wait = double(cw_total / double (plist.size()));
    double throughput = double(plist.size()*100/double(curr_time));
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
           curr_time,
           cpu_untilization,
           io_utilization,
           avg_turnaround,
           avg_cpu_wait,
           throughput
           );
}


int main(int argc,  char * argv[]) {
    
    
    scheduler* sche;
    simulator des;
    int quantum = 10000; //default value of quantum is 10000;
  
    //console command;
    string sche_type_command ;
    string sche_type;
    
    int c;
    opterr = 0;
    while ( (c = getopt(argc, argv, "s:")) != -1 )
    switch (c) {
        case 's':
        sche_type_command = string(optarg);
        break;
        case '?':
        if (optopt == 's')
        fprintf (stderr, "Option -%c requires an argument. \n", optopt);
        else if (isprint (optopt))
        fprintf (stderr, "Unknown option '-%c'.\n", optopt);
        default:
        abort ();
    }
    
   
    
    switch (sche_type_command[0]) {
        case 'F':
        sche_type ="FCFS";
        sche = new FCFS();
        break;
        
        case 'L':
        sche_type = "LCFS";
        sche = new LCFS();
        break;
        
        case 'S':
        sche_type = "SJF";
        sche = new SJF();
        break;
    
        case'R':
        quantum = std::atoi(sche_type_command.substr(1).c_str());
        sche_type = "RR";
        sche = new RR();
        break;
    
        case 'P':
        quantum = std::atoi(sche_type_command.substr(1).c_str());
        sche_type = "PRIO";
        sche = new PRIO();
        break;
    
    }
    
    string filename = string (argv[2]);
    string rfilename = string (argv[3]);
    
    ofs = 0;
    read_rfile(rfilename);
    readFile(filename);
    
    
    //Put initial events for processes’s arrival into the event queue
    for (int i=0; i < plist.size(); i++) {
        event e = des.create_event(TRANS_TO_READY, i, plist[i].at, 0);
        des.eventqueue.push_back(e);
    }
    
    process* cpu_process = 0;//initialize cpu status to empty
    process* block_process = 0;//initialize io status to empty
    int current_t =0; //current time
    int cpuburst;
    int ioburst;
    int io_complete=0; // the time io finish processing all the processes
    int io_sum = 0;  //total io time
    int turnaround_sum = 0;
    int cw_sum = 0;
    int cpu_sum = 0;
    bool call_scheduler = false;
    
        while (!des.isEmpty()){
           event evt = des.get_event();
            current_t = evt.timestamp;
            int i = evt.related_processid;
            
            //created to ready; block to ready
                if(evt.state== TRANS_TO_READY){
                    if(block_process==&plist[i]){
                        block_process=0;
                    }
                    sche -> add_process(&plist[i],false);
                    call_scheduler = true;
                   // plist[i].ready_t = current_t;
                    
                }
            
            
                else if(evt.state==READY_TO_RUN){
                    
                call_scheduler = false;
                cpu_process = &plist[i];
                if(plist[i].remain_burst_t>0){
                    cpuburst = plist[i].remain_burst_t;
                }
                else {
                    cpuburst = getRandom(evt, ofs, &plist[i]);
                }
                if(cpuburst >=plist[i].tc - plist[i].runtime){
                    cpuburst = plist[i].tc - plist[i].runtime;
                }
                    
                //if (plist[i].begin_t == -1){
               //     plist[i].begin_t = current_t;
                //}
                    
                //RUNNING_TO_READY
                if (cpuburst > quantum ){
                    plist[i].runtime += quantum;
                    plist[i].remain_burst_t = cpuburst - quantum;
                    des.add_event(des.create_event(RUN_TO_READY, i, current_t, quantum));
                }
                //DONE
                else if(cpuburst + plist[i].runtime >=plist[i].tc){
                      plist[i].runtime += cpuburst;
                      des.add_event(des.create_event(DONE, i, current_t, cpuburst));
                }
                //RUNNING_TO_BLOCK
                else if(cpuburst+ plist[i].runtime < plist[i].tc){
                    plist[i].runtime += cpuburst;
                    plist[i].remain_burst_t = -1;
                    des.add_event(des.create_event(RUN_TO_BLOCK, i, current_t, cpuburst));
                }
            }
            
                //run to ready
                else if(evt.state== RUN_TO_READY){
                cpu_process =0;
                    if(sche_type=="PRIO"){
                        plist[i].d_pri--;
                    }
                sche -> add_process(&plist[i],true);
                    if(plist[i].d_pri==-1){
                        plist[i].d_pri=plist[i].s_pri-1;
                    }
                call_scheduler = true;
                }
            
                
                else if(evt.state== RUN_TO_BLOCK){

                    cpu_process = 0;
                    if(sche_type=="PRIO"){
                        plist[i].d_pri=plist[i].s_pri-1;
                    }
                    
                ioburst = getRandom(evt, ofs, &plist[i]);
                plist[i].it += ioburst;
               
               
                des.add_event(des.create_event(TRANS_TO_READY, i, current_t, ioburst));
                plist[i].d_pri =plist[i].s_pri-1;
                if (block_process == 0){
                    io_sum += ioburst;
                    io_complete = ioburst + current_t;
                    block_process = &plist[i];
                }
                
                else{
                    
                    if(current_t + ioburst > io_complete){
                        io_sum +=ioburst+current_t - io_complete;
                        io_complete = ioburst + current_t;
                        block_process = &plist[i];
                        
                    }
                }
                call_scheduler = true;
                }
            
                
           else if(evt.state== DONE) {
                plist[i].ft = current_t;
                plist[i].tt = plist[i].ft - plist[i].at;
                plist[i].cw = plist[i].ft - plist[i].at -plist[i].tc-plist[i].it;
               
                cpu_process = 0;
                call_scheduler=true;
           }
            
            
           if(call_scheduler==true){
                   if(des.get_next_time()==current_t)
                    {continue;}
                   //process the events with the same stamp in the order they are created
                   call_scheduler = false;
                   if(cpu_process==0){
                   if(sche->isEmpty()){continue;}
                    cpu_process = sche->get_next_process();
                   
                   des.add_event(des.create_event(READY_TO_RUN, cpu_process->pid, current_t, 0));
                   cpu_process=0;
                   
               }
            }
        }
    
    for(int i=0;i<plist.size();i++){
        turnaround_sum += plist[i].tt;
        cw_sum += plist[i].cw;
        cpu_sum += plist[i].tc;
    }
    output(sche_type, quantum,  current_t,  cpu_sum,  io_sum, turnaround_sum,  cw_sum);
    
    return 0;
}










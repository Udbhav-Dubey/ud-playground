#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <fstream>
#include <cstdlib>
class Subject{
public:
    std:: string name;
    int TUT;
    std ::vector<int> Lecture;
    int LAB;
    int lecture_total;
    int lecture_left;
    int tut_total;
    int tut_left;
    int lab_total;
    int lab_left;
    
};
class Data_block{
private:
std::vector<int> days=std::vector<int>(5,0);
std::vector<std::string>weekdays={"monday","tuesday","wednesday","thursday","friday"};
int sub_count;
public:
    void createFile(){
        system("clear");
        std::cout << "YOu want to create a File\n";
        std::cout << "now answer the following question\n";
        std::ofstream workd("work_days.txt",std::ios::app);
        if (!workd){
            std::cout << "error in file creation\n";
            exit(1);
        }
        for (int i=0;i<5;i++){
            std::cout << "How many working " << weekdays[i] << " you have : ";
            int x;
            std::cin>>days[i];
            workd<<weekdays[i] << " : " << days[i] << "\n";
        }
        workd.close();
        std::cout << "How many subjects you have this sem : " ;
        int sub_count;
        std::cin>>sub_count;
        std::vector <Subject> subjects(sub_count);
        system("clear");
        std::cout << "now we will get data for subject \nFor Monday type  :  0\nFor Tuesday type :  1\nFor Wednesday Type :2\nFor Thursday type : 3\nFor Friday Type : 4\n";
        std::cout << "if No type of that class write -1 in days for example if no tut in some subject and asked about tut write -1 in day\n";
            for (int i=0;i<sub_count;i++){
            std::cout << "Name of subject : " ;
            std::string sub_name;
            std::cin>>sub_name;
            subjects[i].name=sub_name;
wrong_tut:
            std::cout << "Tut day : " ;
            int tut_day;
            std::cin>>tut_day;
            if (tut_day>4 || tut_day<-1){
                std::cout << "wrong type , try again only -1 to 4 ";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                goto wrong_tut;
            }
            subjects[i].TUT=tut_day;
wrong_lab:
            std::cout << "Lab day : ";
            int lab_day;
            std::cin>>lab_day;
            if (lab_day>4||lab_day<-1){
                std::cout << "wrong type, try again only -1 to 4 ";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                goto wrong_lab;
            }
            subjects[i].LAB=lab_day;
        
/*wrong_lec_num:
            std::cout << "how many lect you have in the week : \n";
            int num_lec;
            std::cin>>num_lec;
            if (num_lec<0||num_lec>5){
                goto wrong_lec_num;
            }
            subjects[i].Lecture.resize(num_lec); */
            subjects[i].Lecture.resize(5);
            std::cout << "now enter the days of lecture like : on that day hit type of number of classes that day \n if class present 1 , if not press -1 for example \nif tuesday have 2 classes type 2\nif 1 classes type 1 \nif no class-1 :\n";
wrong_lec:
                for (int k=0;k<5;k++){
                    int num_lec;
                    std::cout << weekdays[k] << " : ";
                    std::cin>>num_lec;
                    if (num_lec<-1||num_lec>5){
                    std::cout << "cmmon type correct number : \n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    goto wrong_lec;
                         }
                    subjects[i].Lecture[k]=num_lec;
              }
        }
        std::ofstream of("classes.txt",std::ios::app);
        if (!of){
            std::cout << "error in txt creation\n";
            exit(1);
        }
        for (int i=0;i<sub_count;i++){
            of << subjects[i].name << "\n";
            of << "TUT : " << subjects[i].TUT << "\n";
            of << "Lab : " << subjects[i].LAB << "\n";
            of << "Lecture : " ;
            for (int j=0;j<5;j++){
                of << subjects[i].Lecture[j] << " ";
            }
            of << "\n";
        }
        of.close();
        std::ofstream off("classes_log.txt",std::ios::app);
        if (!off){
            std::cout << "error in log creation\n";
            exit(1);
        }
        system("clear");
        std::cout << "answer the following : \n";
        for (int i=0;i<sub_count;i++){
            std::cout << "number of lecture left of " << subjects[i].name << " ";
            std::cin>>subjects[i].lecture_left;
            std::cout << "number of lab left of " << subjects[i].name << " ";
            std::cin>>subjects[i].lab_left;
            std::cout << "number of tut left of " << subjects[i].name << " ";
            std::cin>>subjects[i].tut_left;
        }
        for (int i=0;i<sub_count;i++){
            off<< subjects[i].name << "\n";
            if (subjects[i].lecture_total>=0){
            off<< "Total Lecture : " << subjects[i].lecture_total << "\n";
            off<< "Lecture Left : " << subjects[i].lecture_left << "\n";
            }
            if (subjects[i].lab_total>=0){
            off<< "Total Lab : " << subjects[i].lab_total<< "\n";
            off<< "Lab Left : " << subjects[i].lecture_left<<"\n";
            }
            if (subjects[i].tut_total>=0){
            off<< "Totat tut : " << subjects[i].tut_total<<"\n";
            off<< "Tut left : " << subjects[i].tut_left<<"\n";
            }
        }
        off.close();
        std::cout << "type :menu to go back to menu\n";
        std::string menuu;
        std::cin>>menuu; // idhar getline shayad better rahe 
        if (menuu==":menu"){ 
            return ;
        }
        else {
            std::cout << "cmmon man \n";
        }
    }
    void getData(){
        system("clear");
        std::ifstream in ("classes.txt");
        if (!in){
            //nodat=1;
            std::cout << "no file found for classes\npress 1 to create data \npress 2 to exit\n";
            int n;
            std::cin>>n;
            if (n==1){
                createFile();
            }
            else if (n==2) {return ;}
            else {std::cout << "cmmon buddy how hard it is too press 1 or 2 \n";}
        }
        std::ifstream inn("classes_log.txt");
        if (!inn){
            std::cout << "no file found for log file lets create one \n";
            createFile();
        }
        std::string line;
        sub_count=0;
            while(std::getline(in,line)){
                if (line.empty()){continue;}
                sub_count++;                
            }
            sub_count /=4;
            in.close();
      //  std::cout << sub_count << "\n";
            std::ifstream in1("classes.txt");    
            std::vector<Subject>subjects(sub_count);
            for (int i=0;i<sub_count;i++){
                std::string line1,line2,line3,line4,tut_str,prac_str,lec_str;
                std::getline(in1,line1);
                subjects[i].name=line1;
                std::getline(in1,line2,':');
                std::getline(in1,tut_str);
                subjects[i].TUT=std::stoi(tut_str);
                std::getline(in1,line3,':');
                std::getline(in1,prac_str);
                subjects[i].LAB=std::stoi(prac_str);
                std::getline(in1,line4,':');
                std::getline(in1,lec_str);
                std::stringstream ss(lec_str);
                int x,u=0;
                subjects[i].Lecture.resize(5);
                while(ss>>x&&u<5){
                    subjects[i].Lecture[u]=x;
                    u++;
                }
            }
back_list:            
            system("clear");
            std::cout << "press the number against subject to log that absent: \n";
            for (int i=0;i<sub_count;i++){
            std::cout << i << " : " << subjects[i].name << "\n";
           }
        int subj_index;
wrong_sub_index:
        if (!(std::cin>>subj_index)){
            std::cin.clear();
            std::cin.ignore(10000,'\n');
            std::cout << "enter a number .\n";
        }
        else if (subj_index<-1||subj_index>sub_count){
            std::cout << "wrong number kid\n"; 
        goto wrong_sub_index;
        }
        {
            system("clear");
        std::cout << "enter the number you want to log as absent \n";
        int b=0,lec_limit,tut_limit,lab_limit;
        for (int i=0;i<5;i++){
            if (subjects[subj_index].Lecture[i]>0){
                std::cout << b++ << " : " << subjects[subj_index].name << " - Lecture " << " - " << weekdays[i] << "\n" ;
            }
        }
        lec_limit=b;
        if (subjects[subj_index].TUT!=-1){
        std::cout << b++ << " : "<<subjects[subj_index].name << " - TUT " << "\n"; 
        }
        tut_limit=b;
        if (subjects[subj_index].LAB!=-1){
        std::cout << b++ << " : "<<subjects[subj_index].name << " - LAB " << "\n";
        }
        lab_limit=b;
limits:        
        int take_input;
        std::cin>>take_input;
        if (take_input>=b||take_input<0){
            std::cout << "Stay in your limits Soldier \n";
            goto limits;
        }
        else if (take_input<lec_limit){
            subjects[subj_index].lecture_left++;
        }
        else if (take_input<tut_limit){
            subjects[subj_index].tut_left++;
        }
        else {
            subjects[subj_index].lab_left++;
        }
        }
        std::ofstream offt("classes_log.txt.tmp",std::ios::app);
        if (!offt){
            std::cout << "Failed to create temp file\n";
            exit(1);
        }
        for (int i=0;i<sub_count;i++){
            offt<< subjects[i].name << "\n";
            if (subjects[i].lecture_total>=0){
            offt<< "Total Lecture : " << subjects[i].lecture_total << "\n";
            offt<< "Lecture Left : " << subjects[i].lecture_left << "\n";
            }
            if (subjects[i].lab_total>=0){
            offt<< "Total Lab : " << subjects[i].lab_total<< "\n";
            offt<< "Lab Left : " << subjects[i].lecture_left<<"\n";
            }
            if (subjects[i].tut_total>=0){
            offt<< "Totat tut : " << subjects[i].tut_total<<"\n";
            offt<< "Tut left : " << subjects[i].tut_left<<"\n";
            }
        }
        offt.close();
        rename("classes_log.txt.tmp","classes_log.txt");
again:
        std::cout << "type :back to go back to subject list\n";
        std::cout << "type :menu to go back to menu\n";
        std::string menuu;
        std::cin>>menuu; // idhar getline shayad better rahe 
        if (menuu==":menu"){ 
            return ;
        }
        else if (menuu==":back"){
            goto back_list ;
        }
        else {
            std::cout << "cmmon man \n";
        }      
    }
};
class UI{
private:
    std::string header_string= R"(
                    Are you getting Detained Son                              
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣤⣤⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣾⠿⠋⠉⠉⢿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠴⠀⠀⠀⠀⠀⠀⠄⠀⠀⠀⠀⠀⠀⠄⠀⠄⠀⡀⢀⠀⠀⠀⠀⠀⣠⣴⡿⠛⠁⠀⠀⠀⠀⠀⢿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴
⠀⠀⠀⠀⡀⠄⠀⠀⠀⠀⠀⠀⠂⠠⠀⢂⠀⠀⠀⠁⠀⠀⠀⠀⠀⠄⠀⠊⠀⠀⣠⣾⡿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠸⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣾⡿⠟
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⡿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣾⡿⠛⠉⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣶⣶⣤⣤⣤⣀⣀⠀⠀⠀⣤⣶⣿⠟⠋⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⠀⠀⠀⣠⣿⣿⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⡤⠤⠿⡻⠛⠉⠉⠛⠛⢿⠿⣿⣷⣿⣿⠟⠁⠀⠀⠀⠀⠀⠀⡼
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠇⠹⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡴⠋⠀⣠⠎⠀⠀⠀⠀⢀⡔⠃⠀⢀⡞⠉⠁⠀⠀⠀⠀⠀⠀⢀⡜⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡞⠀⠀⣰⠃⠀⠀⠀⠀⡰⠋⠀⠀⣠⠊⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣁⢀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⣿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⣰⠃⠀⠀⠀⠀⡜⠁⠀⠀⡰⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢀⣴
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⡟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⣦⡴⠃⠀⠀⠀⠀⢸⠀⠀⠀⡰⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣅
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⢿⣿⣿⠋⠀⠀⠀⠀⠀⠀⢀⣠⣶⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠳⠶⠚⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈
⠀⠀⠀⠀⠀⠀⠀⣴⡿⠿⢿⣷⣦⣄⠀⣿⠃⠀⠈⠁⠀⠀⠀⠀⠀⠀⣰⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⢀⣤⣤⣼⣿⠀⠀⠀⣿⣿⣿⣿⣿⣄⡀⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣴⣾⣿⣶⣤⡀⠀⠀⠀⠀⠀⠀
⠀⢀⣴⣿⠛⠉⠉⠉⠀⠀⠀⠉⠙⢛⠛⠿⣿⣿⣦⣄⠀⠀⠀⠀⠀⢿⣿⣿⣿⣿⣿⡟⠀⠀⠀⠐⠀⠀⠀⠀⠀⠀⠂⠠⡀⠀⠀⠀⠀⠀⠀⣾⣿⣿⣿⣿⣿⣿⣿⡄⠀⠀⠀⠀⠀
⣴⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠳⢤⡈⠙⠿⣿⣿⣶⣄⡀⠀⠈⠛⠻⠿⠟⠋⠀⡠⢡⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀
⣿⡇⠀⠀⠀⣠⡾⣛⣿⣶⡀⠀⠀⠀⠀⠀⠉⠓⠤⡈⠉⠻⢿⣿⣶⣄⡀⠀⠀⠀⠀⠐⠀⠸⣄⠀⠀⠀⢠⡄⠀⠀⠀⠀⢀⡀⠀⠡⠀⠀⠀⢿⣿⣿⣿⣿⣿⣿⣿⡟⠀⠀⠀⠀⠀
⣿⡇⠀⠀⢰⡟⢰⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠁⠂⢄⡑⠢⢀⠉⠻⢿⣿⣦⣄⡀⠀⠀⠀⠀⠈⠓⠒⠚⠁⠙⢄⣀⣀⣠⠞⠁⠀⠀⡀⠀⠀⠈⠻⢿⣿⣿⣿⠿⠛⠀⠀⠀⠀⠀⠀
⣿⡇⠀⠀⠘⣷⡘⣿⣿⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠑⠠⢁⠂⠀⠈⠛⠿⣿⣶⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⣿⡇⠀⠀⠀⠈⠛⠿⠟⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀⠀⠀⢹⠛⠻⣿⣶⣤⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡐⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⣿⠉⣿⣿⣶⣤⡀⣀⣤⣤⣄⣀⠀⠀⠊⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⣿⡇⠀⠀⠀⠀⠀⣿⣖⠀⠀⣈⠀⠀⠀⠈⠙⠲⢤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⣿⠀⣿⠀⢸⠛⢿⣿⡿⠛⠿⢿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⣿⡇⠀⠀⠳⣦⡀⠀⠀⢀⠔⠀⠀⠀⠀⠀⠀⠀⢸⠈⠛⠳⣤⣀⠀⠀⠀⠀⠀⢸⡀⠀⣿⠀⣿⠀⢸⠀⠈⣏⠛⣶⣄⡀⠈⢿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⣿⡇⢀⠀⠀⠀⠉⠉⠉⠁⠀⠀⣠⣤⣤⣄⡀⠀⢸⠀⠀⠀⡇⠈⡟⠲⢤⣀⠀⠸⣷⣄⣿⠀⣿⠀⢸⠀⠀⣿⠀⣿⠀⠉⠓⠺⣿⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⢿⣿⣬⠑⠒⢤⣄⣀⣀⣤⡤⢦⣅⠉⠛⢯⣽⣳⣾⣄⡁⠀⡇⠀⡇⠀⠀⠈⠙⠲⢬⡙⠻⣿⣿⣦⣼⡀⠀⣿⠀⣿⠀⠀⠀⠀⠈⢿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠈⠛⠿⣿⣶⣤⣀⡉⠛⢦⣄⡀⠈⠙⠶⣤⣈⠙⠻⢿⣿⣷⣧⣀⡇⠀⠀⠀⠀⠀⠀⠈⠙⠲⢌⡙⠿⣿⣿⣿⣀⣾⠀⠀⠀⠀⠀⢸⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠉⠛⠿⣿⣷⣶⣬⣙⡳⠄⠀⠀⠉⠙⠶⣤⣈⠙⠳⣯⣻⡶⣤⣀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠲⢬⣛⠿⣿⠀⠀⠀⠀⠀⠈⣿⡇⠀⠀⠀⠀⠀⠀⠰⣄⠀⠀⠀⠀⢀⡶⠶
⠀⠀⠀⠀⠀⠀⠀⠀⠙⠛⠿⣿⣿⣿⣶⣤⣄⡀⠀⠀⠙⠷⢦⣤⡈⠙⠶⣽⣓⣦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠲⢤⣀⠀⠀⠀⠀⣿⡇⠀⠒⢀⠀⠀⠀⢀⠈⠳⢤⣀⣀⣼⠇⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠛⣿⣿⡿⠿⠿⠿⠶⣶⣦⣈⡙⠓⢦⣄⣉⣿⣬⠿⠷⠦⣤⣤⣤⣄⣀⠀⠀⠀⠀⠈⠙⠲⣤⣠⣿⣿⠀⠀⠀⠑⠂⠐⠋⠀⠀⠀⠉⠉⠁⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⡏⠀⠀⠀⠀⠀⠀⠉⢻⣿⡶⠋⠉⠁⠀⠀⠀⠀⠀⠀⠈⠉⠙⠻⣿⣦⣄⠀⠀⠀⠀⣿⣿⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠻⣷⣤⡀⢀⣿⡏⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⢿⣾⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
            
                Are you getting Detained Son 
    )";
public:
    void intro(){
        system("clear");
        std::cout << header_string;
        std::cout<< "\n\nPress Enter To Continue\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        std::cin.ignore();
    }
    void menue(){
        system("clear");
        std::cout << "\n1 : log absent  " ;
        std::cout << "\n2 : classes you can leave  ";
        std::cout << "\n3 : edit classes ";
        std::cout << "\n4 : exit\n";      
    }
    void menue_ask(){
        int x;
        Data_block db;
        while(true){
        menue();
        std::cin>>x;
        bool flag=false;
            switch(x){
            case 1 : db.getData();
                     break;
            case 2 : break;
            case 3 : break;
            case 4 : flag=true;
                     break;
            default: std::cout << "cmmon buddy how hard it is to press 1-4\n";
            }
            if (flag==1){
                break;
            }
    }
    }
};
int main (){
    UI ui;
    ui.intro();
    //ui.menue();
    ui.menue_ask();
    return 0;
}

#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sched.h>
#include <string>
#include <wait.h>
#include <cstring>
#include <sys/mount.h>
#include <fstream>
#include <signal.h>
#include <filesystem>
#define STACK_SIZE 8192

typedef struct clone_args{
    char* host_name;
    char* filesystem_directory;
    int num_processes;
    char** args;
}clone_args;

int child(void* args){
    auto* args1 = (clone_args*) args;
    sethostname(args1->host_name,strlen(args1->host_name)); // setting the new host name
    chroot(args1->filesystem_directory); // setting the new directory

    chdir("/");
    mkdir("/sys/fs",0755);
    mkdir("/sys/fs/cgroup",0755);
    mkdir("/sys/fs/cgroup/pids",0755);


    //file open and write to it
    std::string cgroup="/sys/fs/cgroup/pids/cgroup.procs";
    std::ofstream file;
    file.open("/sys/fs/cgroup/pids/cgroup.procs");
    chmod("/sys/fs/cgroup/pids/cgroup.procs",0755);
    file << "1";
    file.close();

    std::ofstream myfile1("/sys/fs/cgroup/pids/pids.max");
    chmod("/sys/fs/cgroup/pids/pids.max",0755);
    myfile1 << args1->num_processes;
    myfile1.close();
    std::ofstream myfile12("/sys/fs/cgroup/pids/notify_on_release");
    chmod("/sys/fs/cgroup/pids/notify_on_release",0755);

    myfile12<<"1";
    myfile12.close();
    mount("proc", "/proc", "proc", 0, 0);

    int ex= execvp(args1->args[0],args1->args);
    if(ex == -1){
        std::cerr << "system error: execvp failed" << std::endl;
        return -1;
    }

    return 0;
}

char* get_dir(char* dir1,char*dir2){
    char* new_dir = new char[strlen(dir1) + strlen(dir2) + 1];
    strcat(new_dir,dir1);
    strcat(new_dir,dir2);
    return new_dir;
}


int main(int argc, char* argv[]) {
    char* stack = new char[8192];
    size_t args_size = argc - 2;
    clone_args c_args;
    c_args.args = new char*[args_size];
    c_args.host_name = argv[1];
    c_args.filesystem_directory = argv[2];
    char* file = argv[2];
    c_args.num_processes = (int) strtol(argv[3], nullptr,10);
    size_t ind = 0;
    for (size_t i = 4; i < argc; ++i) { // get the path and rest of the args

        c_args.args[ind] =(char*) argv[i];
        ind++;
    }
    c_args.args[args_size-1] = nullptr;

    int child_pid = clone(child,stack+STACK_SIZE, CLONE_NEWUTS| CLONE_NEWPID| CLONE_NEWNS| SIGCHLD,&c_args);
    if(child_pid <0){
        exit(1);
    }
    wait(nullptr);

    char* directory = get_dir(argv[2],(char*)"sys/fs/cgroup/pids/notify_on_release");
    remove(directory);
    delete[] directory;

    char* directory1 = get_dir(argv[2],(char*)"sys/fs/cgroup/pids/pids.max");
    remove(directory1);
    delete[] directory1;

    char* directory2 = get_dir(argv[2],(char*)"sys/fs/cgroup/pids/cgroup.procs");
    remove(directory2);
    delete[] directory2;

    char* directory3 = get_dir(argv[2],(char*)"sys/fs/cgroup/pids");
    rmdir(directory3);
    delete[] directory3;

    char* directory4 = get_dir(argv[2],(char*)"sys/fs/cgroup");
    rmdir(directory4);
    delete[] directory4;

    char* directory5 = get_dir(argv[2],(char*)"sys/fs");
    rmdir(directory5);
    delete[] directory5;

    char* directory6 = get_dir(file,(char*)"/proc");

    umount(directory6);
    delete[] directory6;
    return 0;
}

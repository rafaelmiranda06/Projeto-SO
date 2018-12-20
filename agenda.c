#include <time.h>
#include"apiagenda.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>

int main(){
	
	int n = 0;
	int atc = open("agendaservidor", O_WRONLY);
	int cta = open("servidoragenda",O_RDONLY);
        
	if(atc==-1){
		perror("Erro ao abrir o fifo para escrever para a agenda");
		return -1;
	}

	if(cta ==-1){
		perror("Erro ao abrir o fifo para ler da agenda");
		return -1;
	}

	char agenda[1024];
	char central[1024];
	
	if(fork()==0){//Para ler
		close(atc);
		while((n=read(cta,central,1024))>0){
			write(1,central,n);
		}
	}else{//Para escrever
		close(cta);
		while((n=read(0, agenda,1024))>0){	
			write(atc,agenda,n);
		}
	}
	return 0;
}
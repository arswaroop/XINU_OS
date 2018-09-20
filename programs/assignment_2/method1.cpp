#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <typeinfo>
#include <vector>

pthread_mutex_t lock;
int flag = 0;


/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  char filename[100];
  int start_line;
  int end_line;
  char word[100];
  std::vector <std::string> sent;
} thread_data_t;
 
/* thread function */
void *thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  std::string line;
  std::fstream file;

  file.open(data->filename);
 
  if(file.good()){
    file.seekg(data->start_line);
    while(file and file.tellg() < data->end_line){
        //pthread_mutex_lock(&lock);
        std::getline(file,line);    
         const char *line_arr = line.c_str();    
        if (strstr(line_arr,data->word)){
            data->sent.push_back(line_arr);
            //printf("%s \n",line_arr);
        }
        //pthread_mutex_unlock(&lock);
    }
  }
  pthread_exit(NULL);
}
 
int main(int argc, char **argv) {
  int i, rc;
  int n;
  char word[100];
  char file_name[100];

  std::string line;
  std::fstream file;

 if (argc < 3){
      printf("Please enter number of threads, searching term and filename to initiate pargrep\n");
    return EXIT_FAILURE;
  }
  else if (argc==3){
    strcpy(word,argv[1]);
    strcpy(file_name,argv[2]);
    n = 1;
  }  
  else{
    n = atoi(argv[1]);
    strcpy(word,argv[2]);
        strcpy(file_name,argv[3]);
  }
  pthread_t thr[n];
  file.open(file_name);

  if(!file.good())
  {
	printf("Please provide a valid input file\n");
	return 1;
  }
  file.seekg(0,file.end);
  int len = file.tellg();
 /* create a thread_data_t argument array */
  thread_data_t thr_data[n];

  int start_line = 0;
  // Adjust line pointer
  for (i = 0; i < n; ++i) 
  {
	  thr_data[i].start_line = start_line;
	  int end_line   = (i<n-1)?(len*(i+1)/n)+1:len;
	  if(i == n-1)
	  {
	  	thr_data[i].end_line = end_line-1;
	  	//printf("start: %d end : %d \n",start_line,end_line-1);
		break;
	  }
	  file.seekg(end_line);
	  std::getline(file,line);
	  end_line = file.tellg();
	  thr_data[i].end_line = end_line-1;
	  //printf("start: %d end : %d \n",start_line,end_line-1);
	  //file.seekg(end_line);
	  //std::getline(file,line);
	  //printf("start: %s\n",line.c_str());
	  //file.seekg(end_line-10);
	  //std::getline(file,line);
	  //printf("end: %s\n",line.c_str());
	  start_line = end_line;
  }
 /* create threads */
  if (pthread_mutex_init(&lock, NULL) != 0)
   {
    printf("\n mutex init has failed\n");
        return 1;
   }
  for (i = 0; i < n; ++i) {
    //thr_data[i].filename = file_name;
    //thr_data[i].start_line = len*i/n;
    //thr_data[i].end_line   = (i<n-1)?(len*(i+1)/n)+1:len;
    strcpy(thr_data[i].filename, file_name);
    //thr_data[i].file = &file;
    strcpy(thr_data[i].word,word);
    if ((rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  
  for (i = 0; i <n ; ++i) {
    pthread_join(thr[i], NULL);

  }
  for(i =0; i< n;i++)
  {
    //printf("printing op for thread i = :%d \n",i);
     for (std::vector<std::string>::iterator it = thr_data[i].sent.begin() ; it != thr_data[i].sent.end(); ++it)
    {
        std::cout<< *it<<std::endl;
    }
  }
  pthread_exit(NULL);
  return EXIT_SUCCESS;
}



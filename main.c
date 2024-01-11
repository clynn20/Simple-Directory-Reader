#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define PREFIX "movies_"
#define SUFFIX ".csv"
#define INT_MAX 2147483647 

struct movie {
    char* title;
    int year;
    char* lang[5];
    float rating_val;
    struct movie* next;
};

// find the largest file in the current directory with PREFIX "movie_" and is of type ".csv"
char* largest_file_name(){
    char* file_name = malloc(256*sizeof(char));
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat;
    off_t size = 0;
    while((aDir = readdir(currDir)) != NULL){
        if((strncmp(PREFIX, aDir->d_name, strlen(PREFIX))==0) && (strcmp(aDir->d_name + strlen(aDir->d_name) - strlen(SUFFIX), ".csv")==0)){
            stat(aDir->d_name, &dirStat);
            if(dirStat.st_size > size){
                size = dirStat.st_size;
                memset(file_name, '\0',256*sizeof(char));
                strcpy(file_name, aDir->d_name);
            }
        }
    }
    closedir(currDir);
    return file_name;
}

// find the smallest file in the current directory with PREFIX "movie_" and is of type ".csv"
char* smallest_file_name(){
    char* file_name = malloc(256*sizeof(char));
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat;
    off_t size = INT_MAX;
    while((aDir = readdir(currDir)) != NULL){
        if((strncmp(PREFIX, aDir->d_name, strlen(PREFIX))==0) && (strcmp(aDir->d_name + strlen(aDir->d_name) - strlen(SUFFIX), ".csv")==0)){
            stat(aDir->d_name, &dirStat);
            if(dirStat.st_size < size){
                size = dirStat.st_size;
                memset(file_name, '\0',256*sizeof(char));
                strcpy(file_name, aDir->d_name);
            }
        }
    }
    closedir(currDir);
    return file_name;
}

// check whether a certain file exists in the current directory
char* ask_and_check_file_name(){
    char* file_name = malloc(256*sizeof(char));
    printf("Enter the complete file name: ");
    scanf("%s", file_name);
    DIR* currDir = opendir(".");
    struct dirent* aDir;
    int exist = 0;
    while((aDir = readdir(currDir)) != NULL){
        if(strncmp(file_name, aDir->d_name, strlen(aDir->d_name))==0){
            memset(file_name, '\0', 256*sizeof(char));
            strcpy(file_name, aDir->d_name);
            exist = 1;
        }
    }
    closedir(currDir);
    if(exist!=1){
        printf("The %s was not found. Try again!\n", file_name);
    }
    return (exist==1)? file_name : NULL;
}

// create a movie
struct movie* create_movie(char* read_line){
    struct movie* new_movie = malloc(sizeof(struct movie));
    char* save_ptr;
    char* save_ptr_lang;

    // first token is title
    char* token = strtok_r(read_line, ",", &save_ptr);
    new_movie->title = calloc(strlen(token)+1, sizeof(char));
    strcpy(new_movie->title, token);

    // second token is year
    token = strtok_r(NULL, ",", &save_ptr);
    new_movie->year = atoi(token);

    // the next token is language
    token = strtok_r(NULL, ",", &save_ptr);
    // parse the language array
    char* lang_token = strtok_r(token, "[]", &save_ptr_lang);
    lang_token = strtok_r(lang_token, ",", &save_ptr_lang);
    for(int i=0; i<5; i++){
        new_movie->lang[i] = NULL;
    }
    int i = 0;
    while(lang_token){
        new_movie->lang[i] = calloc(strlen(lang_token)+1, sizeof(char));
        strcpy(new_movie->lang[i], lang_token);
        lang_token = strtok_r(NULL, ",", &save_ptr_lang);
        i++;
    }
    // the last token is rating value
    token = strtok_r(NULL, "\n", &save_ptr);
    new_movie->rating_val = strtod(token, NULL);
    new_movie->next = NULL;
    return new_movie;
}

// create the movie list
struct movie* process_file(char* file_name){
    FILE* read_file = fopen(file_name, "r");
    if(read_file == NULL){
        printf("Could not open the file named %s\n", file_name);
        exit(1);
    }
    char* read_line = NULL;
    size_t len = 0;
    ssize_t nread;
    struct movie* head = NULL;
    struct movie* tail = NULL;
    getline(&read_line, &len, read_file);
    while((nread = getline(&read_line, &len, read_file))!=-1){
        struct movie* new_movie = create_movie(read_line);
        if(head==NULL){
            head = new_movie;
            tail = new_movie;
        }
        else{
            tail->next = new_movie;
            tail = new_movie;
        }
    }
    free(read_line);
    fclose(read_file);
    return head;
}

// create a new directory
char* create_dir(){
    srand((unsigned)time(NULL));
    int num = rand() % 100000;
    char* dir_name = malloc(256*sizeof(char));
    sprintf(dir_name, "myfile.movies.%d", num);
    mkdir(dir_name, 0750);
    printf("Created directory with name %s\n", dir_name);
    return dir_name;
}

// create the txt file under a given file 
void create_txt_file(char* dir_name, struct movie* list){
    char* file_path = malloc(256*sizeof(char));
    DIR* new_dir = opendir(dir_name);
    if(new_dir == NULL){
        printf("Could not open %s\n", dir_name);
        exit(1);
    }
    while(list!=NULL){
        memset(file_path, '\0', 256*sizeof(char));
        sprintf(file_path, "%s/%d.txt", dir_name, list->year);
        FILE* write_file = fopen(file_path, "a");
        chmod(file_path,0640);
        fprintf(write_file, "%s\n", list->title);
        fclose(write_file);
        list = list->next;
    }
    free(file_path);
    closedir(new_dir);
}

void free_mem(struct movie* list){
    while(list!=NULL){
        struct movie* temp = list->next;
        free(list->title);
        for(int i=0; i<5; i++){
            free(list->lang[i]);
        }
        free(list);
        list = temp;
    }
}

void operation_menu(){
    int option = 0;
    int exit = 0;
    char* file_name;
    char* dir_name;
    struct movie* movie_list;
    while(exit!=1){
        do{
            printf("Which file do you want to process?\n");
            printf("Enter 1 to pick the largest file.\n");
            printf("Enter 2 to pick the smallest file.\n");
            printf("Enter 3 to specify the name of a file.\n");
            printf("Enter a choice from 1 to 3: ");
            scanf("%d", &option);
            if(option!=1 && option!=2 && option!=3){
                printf("Invalid choice, please try again!\n");
            }
        }while(option!=1 && option!=2 && option!=3);
        switch(option){
            case 1:
                file_name = largest_file_name();
                printf("Now processing the chosen file named %s\n", file_name);
                movie_list = process_file(file_name);
                dir_name = create_dir();
                create_txt_file(dir_name, movie_list);
                free_mem(movie_list);
                free(file_name);
                free(dir_name);
                exit = 1;
                break;

            case 2:
                file_name = smallest_file_name();
                printf("Now processing the chose file named %s\n", file_name);
                movie_list = process_file(file_name);
                dir_name = create_dir();
                create_txt_file(dir_name, movie_list);
                free_mem(movie_list);
                free(file_name);
                free(dir_name);
                exit = 1;
                break;

            case 3:
                file_name = ask_and_check_file_name();
                if(file_name != NULL){
                    printf("Now processing the chosen file named %s\n", file_name);
                    movie_list = process_file(file_name);
                    dir_name = create_dir();
                    create_txt_file(dir_name, movie_list);
                    free_mem(movie_list);
                    free(file_name);
                    free(dir_name);
                    exit = 1;
                }
                else if(file_name == NULL){
                    exit = 0;
                }
                break;
        }
    }
}

int main(){
    int choice = 0;
    do{
        printf("1. Select file to process\n");
        printf("2. Exit the program\n");
        printf("Enter a choice 1 or 2: ");
        scanf("%d", &choice);
        if(choice != 1 && choice != 2){
            printf("Invalid choice, please try again!\n");
        }
        switch(choice){
            case 1:
                    operation_menu();
                    break;
            case 2:
                    exit(0);
                    break;
        }

    }while(choice!=2);
    return 0;
}
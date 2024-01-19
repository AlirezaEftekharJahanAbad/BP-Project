#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// functions prototypes
int init(void);
int doesFolderExist(char *);
void MainSettings(void);
void globalNameConfig(const char * New_Value);
void globalEmailConfig(const char * New_Value);
void projectsGlobalConfig();


// constant values
const char settingsFolder[]="C:\\Users\\admin\\Desktop\\settings";
const char settingsFilePath[]="C:\\Users\\admin\\Desktop\\settings\\settings.txt";
const char projectsFilePath[]="C:\\Users\\admin\\Desktop\\settings\\projects.txt";

// global variables
FILE * file;
char workingDirectory[1024];



int main(int argc, char const *argv[])
{

    getcwd(workingDirectory,sizeof(workingDirectory));
    MainSettings();

    if (strcmp(argv[0], "vgit") == 0 && argc == 1)
    {
        printf("\tvgit version 1.0.0");
    }
    else if(strcmp(argv[0],"vgit")==0 &&strcmp(argv[1],"config")==0 && strcmp(argv[2],"--global")==0){
        if (strcmp(argv[3],"user.name")==0)
        {
            globalNameConfig(argv[4]);
            projectsGlobalConfig();
        }
        else if(strcmp(argv[3],"user.email")==0){
            globalEmailConfig(argv[4]);
            projectsGlobalConfig();
        }
        
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "init") == 0)
    {
        init();
    }

    return 0;
}

// global config
void globalNameConfig(const char * New_Value){

    char prevSettings[2][1024];

    file=fopen(settingsFilePath,"r");
    fgets(prevSettings[0],1024,file);
    fgets(prevSettings[1],1024,file);
    fclose(file);

    file=fopen(settingsFilePath,"w");
    fprintf(file,"UserName : %s\n",New_Value);
    fputs(prevSettings[1],file);
    fclose(file);
}

void globalEmailConfig(const char * New_Value){

    char prevSettings[2][1024];

    file=fopen(settingsFilePath,"r");
    fgets(prevSettings[0],1024,file);
    fgets(prevSettings[1],1024,file);
    fclose(file);

    file=fopen(settingsFilePath,"w");
    fputs(prevSettings[0],file);
    fprintf(file,"UserEmail : %s\n",New_Value);
    fclose(file);
}

void projectsGlobalConfig(){
    
    int repoCount=0,ch=0;
    char projects[1024][1024];
    
    file=fopen(projectsFilePath,"r");
    do{
        ch=fgetc(file);
        if (ch == '\n')
        {
            repoCount++;
        }
    }while(ch!=EOF);
    fclose(file);
    repoCount++;

    file=fopen(projectsFilePath,"r");
    for(int i=0;i<repoCount;i++){
        fgets(projects[i],1024,file);
        projects[i][strlen(projects[i])]='\0';        
    }
    fclose(file);
    

    for (int i = 0; i < repoCount; i++)
    {

        char tempRepoSettingsPath[1024];
        sprintf(tempRepoSettingsPath,"%s\\settings.txt",projects[i]);

        char Data[2][1024];
        file=fopen(settingsFilePath,"r");
        fgets(Data[0],1024,file);
        fgets(Data[1],1024,file);
        fclose(file);


        file=fopen(tempRepoSettingsPath,"w");
        fputs(Data[0],file);
        fputs(Data[1],file);
        fclose(file);   
    }
}

// vgit main settings
void MainSettings(){
    struct stat st;

    if (stat(settingsFolder, &st) == 0 && S_ISDIR(st.st_mode))
        {
            // if "settings" folder exists do nothing
        }
    else{
        
        mkdir(settingsFolder);

        // creating settings.txt
        chdir(settingsFolder);
        file =fopen(settingsFilePath,"w");
        fprintf(file,"");
        fclose(file);
        
        file =fopen(projectsFilePath,"w");
        fprintf(file,"");
        fclose(file);
        chdir(workingDirectory);
    }
}


// init function implementation
int init()
{

    char currentDir[1024];
    getcwd(currentDir, 1024);

    int vgitExistence = doesFolderExist(".vgit");

    if (vgitExistence == 1)
    {
        return 0;
    }
    else
    {
        chdir(currentDir);
        mkdir(".vgit");
        system("attrib +H .vgit");
        printf("Initialized empty Git repository successfully :)");
    }

        char repoFullAddrress[1024];
        sprintf(repoFullAddrress,"%s\\%s",currentDir,".vgit");

        char reposettingsAddrress[1024];
        sprintf(reposettingsAddrress,"%s\\%s",repoFullAddrress,"settings.txt");


        // creating settings.txt for repository
        file=fopen(reposettingsAddrress,"w");
        fprintf(file,"");
        fclose(file);


        // save repo Full Address
        file =fopen(projectsFilePath,"a");
        fputs(repoFullAddrress,file);
        fclose(file);
}

int doesFolderExist(char *folderName)
{
    struct stat st;

    char currentDir[1024];
    getcwd(currentDir, sizeof(currentDir));

    char folderPath[1024];
    while (1)
    {
        // Check if the folder exists in the current directory
        sprintf(folderPath, "%s\\%s", currentDir, folderName);
        if (stat(folderPath, &st) == 0 && S_ISDIR(st.st_mode))
        {
            printf("cannot create directory '.vgit': File exists\n");
            return 1;
        }

        // Move to the parent directory
        chdir("..");

        getcwd(currentDir, sizeof(currentDir));

        // Check if we've reached the root directory
        if (strcmp(currentDir, "C:\\") == 0 || strcmp(currentDir, "D:\\") == 0 || strcmp(currentDir, "F:\\"))
        {
            sprintf(folderPath, "%s\\%s", currentDir, folderName);
            if (stat(folderPath, &st) == 0 && S_ISDIR(st.st_mode))
            {
                printf("cannot create directory '.vgit': Folder exists\n");
                return 1;
            }
            return 0;
        }
    }
}

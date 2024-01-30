#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// functions prototypes
int init(void); // init function
int doesFolderExist(char *); // check folder existence for init function error handling
void MainSettings(void); // creates settings folder for vgit
void globalNameConfig(const char * New_Value); // save global username in main settings
void globalEmailConfig(const char * New_Value); // save global useremail in main settings
void globalAliasConfig(const char *,const char *);
void projectsGlobalConfig();  // save username and useremail in every repo settings 
void projectLocalNameConfig(char *,const char *); // save username in repo settings
void projectLocalEmailConfig(char *,const char *); // save useremail in repo settings
void projectLocalAliasConfig(char *,const char *,const char *); // save alias in repo settings
int lineCounter(const char *); // counts file's line number
void aliasChecker(char *,const char *);



// constant values
const char settingsFolder[]="C:\\Users\\admin\\Desktop\\settings";
const char vgitMainSettings[]="C:\\Users\\admin\\Desktop\\settings\\settings.txt";
const char vgitMainProjects[]="C:\\Users\\admin\\Desktop\\settings\\projects.txt";
const char vgitMainAlias[]="C:\\Users\\admin\\Desktop\\settings\\alias.txt";

// global variables
FILE * file;
char workingDirectory[1024];



int main(int argc, char const *argv[])
{

    getcwd(workingDirectory,sizeof(workingDirectory));
    MainSettings();


    if (strcmp(argv[0], "vgit") == 0 && argc == 1)
    {
        printf("(: vgit version 1.0.0 :)");
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "init") == 0)
    {
        init();
    }
    else if(strcmp(argv[0],"vgit")==0 && argc == 2){
        aliasChecker(workingDirectory,argv[1]);
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
        else if(strncmp(argv[3],"alias",5)==0){
            globalAliasConfig(argv[3],argv[4]);
        }    
    }
    else if(strcmp(argv[0],"vgit")==0 &&strcmp(argv[1],"config")==0 )
    {
        if (strcmp(argv[2],"user.name")==0){
            projectLocalNameConfig(workingDirectory,argv[3]);
        }
        else if(strcmp(argv[2],"user.email")==0){
            projectLocalEmailConfig(workingDirectory,argv[3]);
        }
        else if(strncmp(argv[2],"alias",5)==0){
            projectLocalAliasConfig(workingDirectory,argv[2],argv[3]);
        }
    }
    

    return 0;
}

// global config
void globalNameConfig(const char * NewValue){

    char prevSettings[2][1024];

    file=fopen(vgitMainSettings,"r");
    fgets(prevSettings[0],1024,file);
    fgets(prevSettings[1],1024,file);
    fclose(file);

    file=fopen(vgitMainSettings,"w");
    fprintf(file,"UserName : %s\n",NewValue);
    fputs(prevSettings[1],file);
    fclose(file);
}

void globalEmailConfig(const char * NewValue){

    char prevSettings[2][1024];

    file=fopen(vgitMainSettings,"r");
    fgets(prevSettings[0],1024,file);
    fgets(prevSettings[1],1024,file);
    fclose(file);

    file=fopen(vgitMainSettings,"w");
    fputs(prevSettings[0],file);
    fprintf(file,"UserEmail : %s\n",NewValue);
    fclose(file);
}

void globalAliasConfig(const char * aliasName,const char * command){
    
    file=fopen(vgitMainAlias,"a");
    fprintf(file,"%s : %s\n",aliasName+6,command);
    fclose(file);


    int repoCount;
    char projects[1024][1024];
    
    repoCount=lineCounter(vgitMainProjects);

    file=fopen(vgitMainProjects,"r");
    for(int i=0;i<repoCount;i++){
        fgets(projects[i],1024,file);
        projects[i][strlen(projects[i])-1]='\0';        
    }
    fclose(file);

    for (int i = 0; i < repoCount; i++)
    {

        char tempRepoAliasPath[1024];
        sprintf(tempRepoAliasPath,"%s\\alias.txt",projects[i]);

        file=fopen(tempRepoAliasPath,"a");
        fprintf(file,"%s : %s\n",aliasName+6,command);
        fclose(file);
   
    }

    
}


void projectsGlobalConfig(){
    
    int repoCount;
    char projects[1024][1024];
    
    
    repoCount=lineCounter(vgitMainProjects);

    file=fopen(vgitMainProjects,"r");
    for(int i=0;i<repoCount;i++){
        fgets(projects[i],1024,file);
        projects[i][strlen(projects[i])-1]='\0';        
    }
    fclose(file);
    

    for (int i = 0; i < repoCount; i++)
    {

        char tempRepoSettingsPath[1024];
        sprintf(tempRepoSettingsPath,"%s\\settings.txt",projects[i]);

        char Data[2][1024];
        file=fopen(vgitMainSettings,"r");
        fgets(Data[0],1024,file);
        fgets(Data[1],1024,file);
        fclose(file);

        file=fopen(tempRepoSettingsPath,"w");
        fputs(Data[0],file);
        fputs(Data[1],file);
        fclose(file);   
    }
}

void projectLocalNameConfig(char * workingDirectory,const char * NewValue){
    char settingsPath[1024];
    sprintf(settingsPath,"%s\\.vgit\\settings.txt",workingDirectory);

    char prevSettings[2][1024];

    file=fopen(settingsPath,"r");
    fgets(prevSettings[0],1024,file);
    fgets(prevSettings[1],1024,file);
    fclose(file);

    file=fopen(settingsPath,"w");
    fprintf(file,"UserName : %s\n",NewValue);
    fputs(prevSettings[1],file);
    fclose(file);

}

void projectLocalEmailConfig(char * workingDirectory,const char * NewValue){
    char settingsPath[1024];
    sprintf(settingsPath,"%s\\.vgit\\settings.txt",workingDirectory);

    char prevSettings[2][1024];

    file=fopen(settingsPath,"r");
    fgets(prevSettings[0],1024,file);
    fgets(prevSettings[1],1024,file);
    fclose(file);

    file=fopen(settingsPath,"w");
    fputs(prevSettings[0],file);
    fprintf(file,"UserEmail : %s\n",NewValue);
    fclose(file);
    
}

void projectLocalAliasConfig(char * workingDirectory,const char * aliasName,const char * command){
    char aliasPath[1024];
    sprintf(aliasPath,"%s\\.vgit\\alias.txt",workingDirectory);

    file=fopen(aliasPath,"a");
    fprintf(file,"%s : %s\n",aliasName+6,command);
    fclose(file);

}

// checks if alias exists
void aliasChecker(char * workingDirectory,const char * command){
    char aliasTxtFilePath[1024];
    sprintf(aliasTxtFilePath,"%s\\.vgit\\alias.txt",workingDirectory);

    char aliases[1024][1024];

    int aliasCount;
    aliasCount=lineCounter(aliasTxtFilePath);


    file=fopen(aliasTxtFilePath,"r");
    for (int i=0;i<aliasCount;i++){
        fgets(aliases[i],1024,file);
        aliases[i][strlen(aliases[i])-1]='\0';
    }
    fclose(file);

    int commandLength=strlen(command);

    for (int i = 0; i < aliasCount; i++)
    {
        if (strncmp(aliases[i],command,commandLength)==0)
        {
            char * command=aliases[i]+commandLength+3;
            system(command);
            return;
        }
    }
    printf("Invalid Command :(");

}

// counts file's line number
int lineCounter(const char * fileAddress){
    int lineNumber=0,ch=0;
    
    file=fopen(fileAddress,"r");
    do{
        ch=fgetc(file);
        if (ch == '\n')
        {
            lineNumber++;
        }
    }while(ch!=EOF);
    fclose(file);

    return lineNumber;
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
        file =fopen(vgitMainSettings,"w");
        fprintf(file,"UserName : \n");
        fprintf(file,"UserEmail : \n");
        fclose(file);
        
        // creating projects.txt
        file =fopen(vgitMainProjects,"w");
        fprintf(file,"");
        fclose(file);

        // creating alias.txt
        file=fopen(vgitMainAlias,"w");
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
        
        char repoAliasAddrress[1024];
        sprintf(repoAliasAddrress,"%s\\%s",repoFullAddrress,"alias.txt");


        // creating settings.txt for repository
        file=fopen(reposettingsAddrress,"w");
        fprintf(file,"UserName : \n");
        fprintf(file,"UserEmail : \n");
        fclose(file);


        // save repo Full Address
        file =fopen(vgitMainProjects,"a");
        fprintf(file,"%s\n",repoFullAddrress);
        fclose(file);

        // creating alias.txt for repository
        file=fopen(repoAliasAddrress,"w");
        fprintf(file,"");
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include <dirent.h>
#include <stdbool.h>
#include <time.h>

// functions prototypes
int init(void);              // init function
int doesFolderExist(char *); // check folder existence for init function error handling

void MainSettings(void); // creates settings folder for vgit

void globalNameConfig(const char *New_Value);  // save global username in main settings
void globalEmailConfig(const char *New_Value); // save global useremail in main settings
void globalAliasConfig(const char *, const char *);
void projectsGlobalConfig();                                      // save username and useremail in every repo settings
void projectLocalNameConfig(char *, const char *);                // save username in repo settings
void projectLocalEmailConfig(char *, const char *);               // save useremail in repo settings
void projectLocalAliasConfig(char *, const char *, const char *); // save alias in repo settings
int lineCounter(const char *);                                    // counts file's line number
void aliasChecker(char *, const char *);

int runAdd(char *, char const *, char const **);
int addToStaging(char *, char const *, char const **);
void listFilesRecursively(const char *, char[][1024], int *, char const **);
int match(const char *, char *);

int runReset(char *, char const *, char const **);
int removeFromStaging(char *, char const *, char const **);
int undo(char *);

void status(char *, char const **);

int runCommit(int, char const **);
int incLastCommitId();
int checkFileDirectoryExists(char *);
int trackFile(char *filepath);
bool isTracked(char *filepath);

void setShortcutMessage(int, const char **);
void replaceShortcutMessage(int, const char **);
void removeShortcutMessage(int, const char **);

int runLog(int, const char **);
// Define a structure to hold file information
struct FileMetadata
{
    char name[256];
    time_t modified_time;
};
// Comparison function for sorting file metadata by modified time
int compare(const void *a, const void *b)
{
    struct FileMetadata *fileA = (struct FileMetadata *)a;
    struct FileMetadata *fileB = (struct FileMetadata *)b;
    return difftime(fileB->modified_time, fileA->modified_time);
}

// constant values
const char settingsFolder[] = "C:\\Users\\admin\\Desktop\\settings";
const char vgitMainSettings[] = "C:\\Users\\admin\\Desktop\\settings\\settings.txt";
const char vgitMainProjects[] = "C:\\Users\\admin\\Desktop\\settings\\projects.txt";
const char vgitMainAlias[] = "C:\\Users\\admin\\Desktop\\settings\\alias.txt";

// global variables
FILE *file;
char workingDirectory[1024];

int main(int argc, char const *argv[])
{

    getcwd(workingDirectory, sizeof(workingDirectory));
    MainSettings();

    if (strcmp(argv[0], "vgit") == 0 && argc == 1)
    {
        printf("(: vgit version 1.0.0 :)");
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "init") == 0)
    {
        init();
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "config") == 0 && strcmp(argv[2], "--global") == 0)
    {
        if (strcmp(argv[3], "user.name") == 0)
        {
            globalNameConfig(argv[4]);
            projectsGlobalConfig();
        }
        else if (strcmp(argv[3], "user.email") == 0)
        {
            globalEmailConfig(argv[4]);
            projectsGlobalConfig();
        }
        else if (strncmp(argv[3], "alias", 5) == 0)
        {
            globalAliasConfig(argv[3], argv[4]);
        }
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "config") == 0)
    {
        if (strcmp(argv[2], "user.name") == 0)
        {
            projectLocalNameConfig(workingDirectory, argv[3]);
        }
        else if (strcmp(argv[2], "user.email") == 0)
        {
            projectLocalEmailConfig(workingDirectory, argv[3]);
        }
        else if (strncmp(argv[2], "alias", 5) == 0)
        {
            projectLocalAliasConfig(workingDirectory, argv[2], argv[3]);
        }
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "add") == 0)
    {
        if (strcmp(argv[2], "-f") != 0)
        {
            runAdd(workingDirectory, argv[2], argv);
        }
        else if (strcmp(argv[2], "-f") == 0)
        {
            for (int i = 3; i < argc; i++)
            {
                runAdd(workingDirectory, argv[i], argv);
            }
        }
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "reset") == 0)
    {
        if (strcmp(argv[2], "-f") != 0 && strcmp(argv[2], "-undo") != 0)
        {
            runReset(workingDirectory, argv[2], argv);
        }
        else if (strcmp(argv[2], "-f") == 0)
        {
            for (int i = 3; i < argc; i++)
            {
                runReset(workingDirectory, argv[i], argv);
            }
        }
        else if (strcmp(argv[2], "-undo") == 0)
        {
            undo(workingDirectory);
        }
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "status") == 0)
    {
        status(workingDirectory, argv);
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "commit") == 0)
    {
        runCommit(argc, argv);
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "set") == 0 && strcmp(argv[2], "-m") == 0 && strcmp(argv[4], "-s") == 0)
    {
        setShortcutMessage(argc, argv);
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "replace") == 0 && strcmp(argv[2], "-m") == 0 && strcmp(argv[4], "-s") == 0)
    {
        replaceShortcutMessage(argc, argv);
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "remove") == 0 && strcmp(argv[2], "-s") == 0)
    {
        removeShortcutMessage(argc, argv);
    }
    else if (strcmp(argv[0], "vgit") == 0 && strcmp(argv[1], "log") == 0)
    {
        runLog(argc, argv);
    }

    else if (strcmp(argv[0], "vgit") == 0 && argc == 2)
    {
        aliasChecker(workingDirectory, argv[1]);
    }

    return 0;
}

// global config
void globalNameConfig(const char *NewValue)
{

    char prevSettings[2][1024];

    file = fopen(vgitMainSettings, "r");
    fgets(prevSettings[0], 1024, file);
    fgets(prevSettings[1], 1024, file);
    fclose(file);

    file = fopen(vgitMainSettings, "w");
    fprintf(file, "UserName : %s\n", NewValue);
    fputs(prevSettings[1], file);
    fclose(file);
}

void globalEmailConfig(const char *NewValue)
{

    char prevSettings[2][1024];

    file = fopen(vgitMainSettings, "r");
    fgets(prevSettings[0], 1024, file);
    fgets(prevSettings[1], 1024, file);
    fclose(file);

    file = fopen(vgitMainSettings, "w");
    fputs(prevSettings[0], file);
    fprintf(file, "UserEmail : %s\n", NewValue);
    fclose(file);
}

void globalAliasConfig(const char *aliasName, const char *command)
{

    file = fopen(vgitMainAlias, "a");
    fprintf(file, "%s : %s\n", aliasName + 6, command);
    fclose(file);

    int repoCount;
    char projects[1024][1024];

    repoCount = lineCounter(vgitMainProjects);

    file = fopen(vgitMainProjects, "r");
    for (int i = 0; i < repoCount; i++)
    {
        fgets(projects[i], 1024, file);
        projects[i][strlen(projects[i]) - 1] = '\0';
    }
    fclose(file);

    for (int i = 0; i < repoCount; i++)
    {

        char tempRepoAliasPath[1024];
        sprintf(tempRepoAliasPath, "%s\\alias.txt", projects[i]);

        file = fopen(tempRepoAliasPath, "a");
        fprintf(file, "%s : %s\n", aliasName + 6, command);
        fclose(file);
    }
}

void projectsGlobalConfig()
{

    int repoCount;
    char projects[1024][1024];

    repoCount = lineCounter(vgitMainProjects);

    file = fopen(vgitMainProjects, "r");
    for (int i = 0; i < repoCount; i++)
    {
        fgets(projects[i], 1024, file);
        projects[i][strlen(projects[i]) - 1] = '\0';
    }
    fclose(file);

    for (int i = 0; i < repoCount; i++)
    {

        char tempRepoSettingsPath[1024];
        sprintf(tempRepoSettingsPath, "%s\\settings.txt", projects[i]);

        char Data[2][1024];
        file = fopen(vgitMainSettings, "r");
        fgets(Data[0], 1024, file);
        fgets(Data[1], 1024, file);
        fclose(file);

        file = fopen(tempRepoSettingsPath, "w");
        fputs(Data[0], file);
        fputs(Data[1], file);
        fclose(file);
    }
}

void projectLocalNameConfig(char *workingDirectory, const char *NewValue)
{
    char settingsPath[1024];
    sprintf(settingsPath, "%s\\.vgit\\settings.txt", workingDirectory);

    char prevSettings[2][1024];

    file = fopen(settingsPath, "r");
    fgets(prevSettings[0], 1024, file);
    fgets(prevSettings[1], 1024, file);
    fclose(file);

    file = fopen(settingsPath, "w");
    fprintf(file, "UserName : %s\n", NewValue);
    fputs(prevSettings[1], file);
    fclose(file);
}

void projectLocalEmailConfig(char *workingDirectory, const char *NewValue)
{
    char settingsPath[1024];
    sprintf(settingsPath, "%s\\.vgit\\settings.txt", workingDirectory);

    char prevSettings[2][1024];

    file = fopen(settingsPath, "r");
    fgets(prevSettings[0], 1024, file);
    fgets(prevSettings[1], 1024, file);
    fclose(file);

    file = fopen(settingsPath, "w");
    fputs(prevSettings[0], file);
    fprintf(file, "UserEmail : %s\n", NewValue);
    fclose(file);
}

void projectLocalAliasConfig(char *workingDirectory, const char *aliasName, const char *command)
{
    char aliasPath[1024];
    sprintf(aliasPath, "%s\\.vgit\\alias.txt", workingDirectory);

    file = fopen(aliasPath, "a");
    fprintf(file, "%s : %s\n", aliasName + 6, command);
    fclose(file);
}

// checks if alias exists
void aliasChecker(char *workingDirectory, const char *command)
{
    char aliasTxtFilePath[1024];
    sprintf(aliasTxtFilePath, "%s\\.vgit\\alias.txt", workingDirectory);

    char aliases[1024][1024];

    int aliasCount;
    aliasCount = lineCounter(aliasTxtFilePath);

    file = fopen(aliasTxtFilePath, "r");
    for (int i = 0; i < aliasCount; i++)
    {
        fgets(aliases[i], 1024, file);
        aliases[i][strlen(aliases[i]) - 1] = '\0';
    }
    fclose(file);

    int commandLength = strlen(command);

    for (int i = 0; i < aliasCount; i++)
    {
        if (strncmp(aliases[i], command, commandLength) == 0)
        {
            char *command = aliases[i] + commandLength + 3;
            system(command);
            return;
        }
    }
    printf("Invalid Command :(");
}

// counts file's line number
int lineCounter(const char *fileAddress)
{
    int lineNumber = 0, ch = 0;

    file = fopen(fileAddress, "r");
    do
    {
        ch = fgetc(file);
        if (ch == '\n')
        {
            lineNumber++;
        }
    } while (ch != EOF);
    fclose(file);

    return lineNumber;
}

// vgit main settings
void MainSettings()
{
    struct stat st;

    if (stat(settingsFolder, &st) == 0 && S_ISDIR(st.st_mode))
    {
        // if "settings" folder exists do nothing
    }
    else
    {

        mkdir(settingsFolder);

        // creating settings.txt
        chdir(settingsFolder);
        file = fopen(vgitMainSettings, "w");
        fprintf(file, "UserName : \n");
        fprintf(file, "UserEmail : \n");
        fclose(file);

        // creating projects.txt
        file = fopen(vgitMainProjects, "w");
        fprintf(file, "");
        fclose(file);

        // creating alias.txt
        file = fopen(vgitMainAlias, "w");
        fprintf(file, "");
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
    sprintf(repoFullAddrress, "%s\\%s", currentDir, ".vgit");

    char reposettingsAddrress[1024];
    sprintf(reposettingsAddrress, "%s\\%s", repoFullAddrress, "settings.txt");

    char repoAliasAddrress[1024];
    sprintf(repoAliasAddrress, "%s\\%s", repoFullAddrress, "alias.txt");

    char repoStagingAddress[1024];
    sprintf(repoStagingAddress, "%s\\%s", repoFullAddrress, "staging.txt");

    char undoControllerAddress[1024];
    sprintf(undoControllerAddress, "%s\\%s", repoFullAddrress, "undoController.txt");

    char tracksAddress[1024];
    sprintf(tracksAddress, "%s\\%s", repoFullAddrress, "tracks.txt");

    char shortcutMessageAddress[1024];
    sprintf(shortcutMessageAddress, "%s\\%s", repoFullAddrress, "shortcutMessage.txt");

    char filesAddress[1024];
    sprintf(filesAddress, "%s\\%s", repoFullAddrress, "files");

    char commitsAddress[1024];
    sprintf(commitsAddress, "%s\\%s", repoFullAddrress, "commits");

    // creating settings.txt for repository
    file = fopen(reposettingsAddrress, "w");
    fprintf(file, "UserName : \n");
    fprintf(file, "UserEmail : \n");
    fprintf(file, "last commit ID : 0\n");
    fclose(file);

    // save repo Full Address
    file = fopen(vgitMainProjects, "a");
    fprintf(file, "%s\n", repoFullAddrress);
    fclose(file);

    // creating alias.txt for repository
    file = fopen(repoAliasAddrress, "w");
    fclose(file);

    // creating staging.txt for repository
    file = fopen(repoStagingAddress, "w");
    fclose(file);

    file = fopen(undoControllerAddress, "w");
    fclose(file);

    file = fopen(tracksAddress, "w");
    fclose(file);

    file = fopen(shortcutMessageAddress, "w");
    fclose(file);

    mkdir(filesAddress);

    mkdir(commitsAddress);
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

int runAdd(char *workingDirectory, char const *path, char const **argv)
{

    // TODO: handle command in non-root directories
    if (path == 0)
    {
        printf("please specify a file or a dirctory");
        return 1;
    }

    return addToStaging(workingDirectory, path, argv);
}

int addToStaging(char *workingDirectory, char const *path, char const **argv)
{

    char absolutePath[1024];
    sprintf(absolutePath, "%s\\%s", workingDirectory, path);

    char undoControllerPath[1024];
    sprintf(undoControllerPath, "%s\\.vgit\\undoController.txt", workingDirectory);

    struct stat st;

    if (stat(absolutePath, &st) == 0)
    {
        char repoStagingAddress[1024];
        sprintf(repoStagingAddress, "%s\\.vgit\\staging.txt", workingDirectory);

        // if path declare a file not a directory
        if (S_ISREG(st.st_mode))
        {

            char fileAddress[1024];
            sprintf(fileAddress, "%s\\.vgit\\files\\%s", workingDirectory, path);

            FILE *fileToReadFrom = fopen(absolutePath, "r");
            FILE *fileToSaveInFiles = fopen(fileAddress, "w");

            char line[1024];
            while (fgets(line, sizeof(line), fileToReadFrom) != NULL)
            {
                fputs(line, fileToSaveInFiles);
            }

            fclose(fileToReadFrom);
            fclose(fileToSaveInFiles);

            int dataNumber = lineCounter(repoStagingAddress);

            char Data[dataNumber][1024];

            file = fopen(repoStagingAddress, "r");
            for (int i = 0; i < dataNumber; i++)
            {
                fgets(Data[i], sizeof(Data[i]), file);
                Data[i][strlen(Data[i]) - 1] = '\0';
            }
            fclose(file);

            file = fopen(repoStagingAddress, "a");

            int flag = 1;
            for (int i = 0; i < dataNumber; i++)
            {
                if (strcmp(path, Data[i]) == 0)
                {
                    flag = 0;
                    break;
                }
            }
            if (flag)
            {
                fprintf(file, "%s\n", path);
            }
            fclose(file);

            file = fopen(undoControllerPath, "w");
            fprintf(file, "%s\n", path);
            fclose(file);
        }

        // if path declare a directory not a file
        else if (S_ISDIR(st.st_mode))
        {
            char filesToStage[1024][1024];
            int *index = malloc(1 * sizeof(int));
            *index = 0;
            listFilesRecursively(absolutePath, filesToStage, index, argv);

            int dataNumber = lineCounter(repoStagingAddress);

            char Data[dataNumber][1024];

            file = fopen(repoStagingAddress, "r");
            for (int i = 0; i < dataNumber; i++)
            {
                fgets(Data[i], sizeof(Data[i]), file);
                Data[i][strlen(Data[i]) - 1] = '\0';
            }
            fclose(file);

            file = fopen(repoStagingAddress, "a");

            for (int j = 0; j < *index; j++)
            {
                int flag = 1;
                for (int i = 0; i < dataNumber; i++)
                {
                    if (strcmp(filesToStage[j], Data[i]) == 0)
                    {
                        flag = 0;
                        break;
                    }
                }
                if (flag)
                {
                    fprintf(file, "%s\n", filesToStage[j]);
                }
            }
            fclose(file);

            file = fopen(undoControllerPath, "w");
            for (int j = 0; j < *index; j++)
            {
                fprintf(file, "%s\n", filesToStage[j]);
            }
            fclose(file);
        }
    }
    else
    {

        DIR *dir = opendir(".");
        struct dirent *entry;

        int flag = 1;
        char realPath[1024];

        while ((entry = readdir(dir)) != NULL)
        {
            flag = match(path, entry->d_name);
            if (!flag)
            {
                strcpy(realPath, entry->d_name);
                return addToStaging(workingDirectory, realPath, argv);
                break;
            }
        }

        if (flag)
        {
            printf("fatal: pathspec '%s' did not match any files\n", path);
        }
        closedir(dir);
    }

    return 0;
}

void listFilesRecursively(const char *basePath, char filesToStage[][1024], int *index, char const **argv)
{

    char searchPath[MAX_PATH];
    WIN32_FIND_DATA findData;

    // Construct the search path
    snprintf(searchPath, MAX_PATH, "%s\\*", basePath);

    // Start searching for files and directories
    HANDLE hFind = FindFirstFile(searchPath, &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // Skip '.' and '..'
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
            {

                // If it's a directory, recursively list files in it
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    char subDir[MAX_PATH];
                    snprintf(subDir, MAX_PATH, "%s\\%s", basePath, findData.cFileName);
                    listFilesRecursively(subDir, filesToStage, index, argv);
                }
                else
                {
                    strcpy(filesToStage[*index], findData.cFileName);
                    (*index)++;

                    if (strcmp(argv[1], "add") == 0)
                    {
                        char fileToReadFromAddress[1024];
                        memset(fileToReadFromAddress, '\0', sizeof(fileToReadFromAddress));
                        strncpy(fileToReadFromAddress, searchPath, strlen(searchPath) - 1);
                        strcat(fileToReadFromAddress, findData.cFileName);

                        char fileAddress[1024];
                        sprintf(fileAddress, "%s\\.vgit\\files\\%s", workingDirectory, findData.cFileName);

                        FILE *fileToReadFrom = fopen(fileToReadFromAddress, "r");
                        FILE *fileToSaveInFiles = fopen(fileAddress, "w");

                        char line[1024];
                        while (fgets(line, sizeof(line), fileToReadFrom) != NULL)
                        {
                            fputs(line, fileToSaveInFiles);
                        }

                        fclose(fileToReadFrom);
                        fclose(fileToSaveInFiles);
                    }
                }
            }
        } while (FindNextFile(hFind, &findData) != 0);

        // Close the handle when done
        FindClose(hFind);
    }
}

int match(const char *wildcard, char *string)
{
    // If both strings are empty, they match
    if (*wildcard == '\0' && *string == '\0')
        return 0;

    // If the wildcard string contains '*' as the first character,
    // try to match the string with wildcard excluding the first character
    if (*wildcard == '*')
        return match(wildcard + 1, string) || (*string && match(wildcard, string + 1));

    // If the wildcard string contains '?' as the first character
    // or the characters at the current positions in both strings match,
    // move to the next character in both strings.
    if (*wildcard == '?' || *wildcard == *string)
        return match(wildcard + 1, string + 1);

    // If none of the above conditions are met, the strings don't match.
    return 1;
}

int runReset(char *workingDirectory, char const *path, char const **argv)
{

    // TODO: handle command in non-root directories
    if (path == 0)
    {
        printf("please specify a file or a dirctory");
        return 1;
    }

    return removeFromStaging(workingDirectory, path, argv);
}

int removeFromStaging(char *workingDirectory, char const *path, char const **argv)
{

    char absolutePath[1024];
    sprintf(absolutePath, "%s\\%s", workingDirectory, path);

    struct stat st;

    if (stat(absolutePath, &st) == 0)
    {
        char repoStagingAddress[1024];
        sprintf(repoStagingAddress, "%s\\.vgit\\staging.txt", workingDirectory);

        // if path declare a file not a directory
        if (S_ISREG(st.st_mode))
        {
            int dataNumber = lineCounter(repoStagingAddress);

            char Data[dataNumber][1024];

            int index = -1;

            file = fopen(repoStagingAddress, "r");
            for (int i = 0; i < dataNumber; i++)
            {
                fgets(Data[i], sizeof(Data[i]), file);
                Data[i][strlen(Data[i]) - 1] = '\0';

                if (strcmp(Data[i], path) == 0)
                {
                    index = i;
                }
            }
            fclose(file);

            file = fopen(repoStagingAddress, "w");

            for (int i = 0; i < dataNumber; i++)
            {
                if (i == index)
                {
                    continue;
                }
                fprintf(file, "%s\n", Data[i]);
            }
        }

        // if path declare a directory not a file
        else if (S_ISDIR(st.st_mode))
        {
            char filesToUnstage[1024][1024];
            int *index = malloc(1 * sizeof(int));
            *index = 0;
            listFilesRecursively(absolutePath, filesToUnstage, index, argv);

            int dataNumber = lineCounter(repoStagingAddress);

            char Data[dataNumber][1024];

            file = fopen(repoStagingAddress, "r");
            for (int i = 0; i < dataNumber; i++)
            {
                fgets(Data[i], sizeof(Data[i]), file);
                Data[i][strlen(Data[i]) - 1] = '\0';
            }
            fclose(file);

            file = fopen(repoStagingAddress, "w");

            for (int i = 0; i < dataNumber; i++)
            {
                int flag = 1;
                for (int j = 0; j < *index; j++)
                {
                    if (strcmp(Data[i], filesToUnstage[j]) == 0)
                    {
                        flag = 0;
                        break;
                    }
                }
                if (flag)
                {
                    fprintf(file, "%s\n", Data[i]);
                }
            }
            fclose(file);
        }
    }
    else
    {

        DIR *dir = opendir(".");
        struct dirent *entry;

        int flag = 1;
        char realPath[1024];

        while ((entry = readdir(dir)) != NULL)
        {
            flag = match(path, entry->d_name);
            if (!flag)
            {
                strcpy(realPath, entry->d_name);
                return addToStaging(workingDirectory, realPath, argv);
                break;
            }
        }

        if (flag)
        {
            printf("fatal: pathspec '%s' did not match any files\n", path);
        }
        closedir(dir);
    }

    return 0;
}

int undo(char *workingDirectory)
{

    char repoundoControllerAddress[1024];
    sprintf(repoundoControllerAddress, "%s\\.vgit\\undoController.txt", workingDirectory);

    char repoStagingAddress[1024];
    sprintf(repoStagingAddress, "%s\\.vgit\\staging.txt", workingDirectory);

    int filesToUnstageNum = lineCounter(repoundoControllerAddress);

    int stagedFilesNum = lineCounter(repoStagingAddress);

    char filesToUnstage[filesToUnstageNum][1024];
    char stagedFiles[stagedFilesNum][1024];

    file = fopen(repoundoControllerAddress, "r");
    for (int i = 0; i < filesToUnstageNum; i++)
    {
        fgets(filesToUnstage[i], sizeof(filesToUnstage[i]), file);
        filesToUnstage[i][strlen(filesToUnstage[i]) - 1] = '\0';
    }
    fclose(file);

    file = fopen(repoStagingAddress, "r");
    for (int i = 0; i < stagedFilesNum; i++)
    {
        fgets(stagedFiles[i], sizeof(stagedFiles[i]), file);
        stagedFiles[i][strlen(stagedFiles[i]) - 1] = '\0';
    }
    fclose(file);

    file = fopen(repoStagingAddress, "w");
    for (int i = 0; i < stagedFilesNum; i++)
    {
        int flag = 1;
        for (int j = 0; j < filesToUnstageNum; j++)
        {
            if (strcmp(stagedFiles[i], filesToUnstage[j]) == 0)
            {
                flag = 0;
                break;
            }
        }
        if (flag)
        {
            fprintf(file, "%s\n", stagedFiles[i]);
        }
    }
    fclose(file);
}

void status(char *workingDirectory, char const **argv)

{

    char repoStagingAddress[1024];
    sprintf(repoStagingAddress, "%s\\.vgit\\staging.txt", workingDirectory);

    int stagedFilesNum = lineCounter(repoStagingAddress);

    char stagedFiles[stagedFilesNum][1024];

    file = fopen(repoStagingAddress, "r");

    for (int i = 0; i < stagedFilesNum; i++)
    {
        fgets(stagedFiles[i], sizeof(stagedFiles[i]), file);
        stagedFiles[i][strlen(stagedFiles[i]) - 1] = '\0';
    }
    fclose(file);

    char repoundoControllerAddress[1024];
    sprintf(repoundoControllerAddress, "%s\\.vgit\\undoController.txt", workingDirectory);

    int filesDeletedFromStage = lineCounter(repoundoControllerAddress);

    char filesDeleted[filesDeletedFromStage][1024];

    file = fopen(repoundoControllerAddress, "r");

    for (int i = 0; i < filesDeletedFromStage; i++)
    {
        fgets(filesDeleted[i], sizeof(filesDeleted[i]), file);
        filesDeleted[i][strlen(filesDeleted[i]) - 1] = '\0';
    }
    fclose(file);

    char AllFiles[1024][1024];
    int *index = (int *)malloc(1 * sizeof(int));
    *index = 0;

    char tempDirectory[1024];
    getcwd(tempDirectory, sizeof(tempDirectory));

    listFilesRecursively(tempDirectory, AllFiles, index, argv);

    // for (int i = 0; i < *index; i++)
    // {
    //     printf("%s\n",AllFiles[i]);
    // }

    for (int i = 0; i < stagedFilesNum; i++)
    {
        printf("%s +M\n", stagedFiles[i]);
    }

    printf("------------------------------\n");

    char *configFiles[1024] = {"alias.txt", "settings.txt", "staging.txt", "undoController.txt"};

    for (int i = 0; i < *index; i++)
    {
        int flag = 1;
        for (int j = 0; j < 4; j++)
        {
            if (strcmp(AllFiles[i], configFiles[j]) == 0)
            {
                flag = 0;
                break;
            }
        }
        if (!flag)
        {
            strcpy(AllFiles[i], "\0");
        }
    }

    for (int i = 0; i < *index; i++)
    {
        int flag = 1;
        for (int j = 0; j < stagedFilesNum; j++)
        {
            if (strcmp(AllFiles[i], stagedFiles[j]) == 0)
            {
                flag = 0;
                break;
            }
        }
        if (flag && strcmp(AllFiles[i], "\0") != 0)
        {
            printf("%s -A\n", AllFiles[i]);
        }
    }

    printf("------------------------------\n");

    for (int i = 0; i < filesDeletedFromStage; i++)
    {
        printf("%s -D\n", filesDeleted[i]);
    }
}

int runCommit(int argc, char const **argv)
{
    if (argc < 4)
    {
        printf("please use the correct format");
        return 1;
    }

    char stagedFilesAddress[1024] = ".vgit\\staging.txt";

    int stagedFilesNumber = lineCounter(stagedFilesAddress);
    if (stagedFilesNumber == 0)
    {
        printf("nothing added to commit");
        return 0;
    }

    if (strlen(argv[3]) > 72)
    {
        printf("message length exceeded maximum allowed length");
        return 1;
    }

    char message[72];

    if (strcmp(argv[2], "-m") == 0)
    {
        strcpy(message, argv[3]);
    }
    else if (strcmp(argv[2], "-s") == 0)
    {
        int shortcutMessagesNumber = lineCounter(".vgit\\shortcutMessage.txt");
        int flag = 0;

        if (shortcutMessagesNumber == 0)
        {
            printf("Shorctut name is invalid");
            return 1;
        }
        else
        {
            file = fopen(".vgit\\shortcutMessage.txt", "r");
            char line[1024];
            for (int i = 0; i < shortcutMessagesNumber; i++)
            {
                fgets(line, sizeof(line), file);
                line[strlen(line) - 1] = '\0';

                if (strncmp(argv[3], line, strlen(argv[3])) == 0)
                {
                    strcpy(message, line + (strlen(argv[3]) + 3));
                    fclose(file);
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
            {
                printf("Shorctut name is invalid");
                fclose(file);
                return 1;
            }
        }
    }

    int commitId = incLastCommitId();
    if (commitId == -1)
        return 1;

    char commitFolder[1024];
    sprintf(commitFolder, ".vgit\\commits\\%d", commitId);
    mkdir(commitFolder);

    int filesNumber = 0;

    FILE *file = fopen(".vgit\\staging.txt", "r");
    if (file == NULL)
        return 1;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n')
        {
            line[length - 1] = '\0';
        }

        char fileAddress[1024];
        sprintf(fileAddress, ".vgit\\files\\%s", line);

        char commitedFileAddress[1024];
        sprintf(commitedFileAddress, "%s\\%s", commitFolder, line);

        FILE *fileToCommit = fopen(fileAddress, "r");
        FILE *commitedFile = fopen(commitedFileAddress, "w");

        char tmpLine[1024];
        while (fgets(tmpLine, sizeof(tmpLine), fileToCommit) != NULL)
        {
            fputs(tmpLine, commitedFile);
        }
        printf("commit %s\n", line);
        filesNumber++;

        trackFile(line);
    }
    fclose(file);

    // save commit info
    char commitInfo[1024];
    strcpy(commitInfo, commitFolder);
    strcat(commitInfo, "\\commitInfo.txt");

    time_t now;
    time(&now);
    char userName[128];

    file = fopen(".vgit\\settings.txt", "r");
    fgets(userName, sizeof(userName), file);
    fclose(file);

    file = fopen(commitInfo, "w");
    fprintf(file, "commit Time : %s", ctime(&now));
    fprintf(file, "commit Message : %s\n", message);
    fputs(userName, file);
    fprintf(file, "commit Id : %d\n", commitId);
    fprintf(file, "branch : \n");
    fprintf(file, "files Number : %d\n", filesNumber);
    fclose(file);

    // free staging
    file = fopen(".vgit\\staging.txt", "w");
    if (file == NULL)
        return 1;
    fclose(file);

    fprintf(stdout, "commit successfully with commit ID %d\n", commitId);
    fprintf(stdout, "commit Message : %s", message);

    printf("\n%s", ctime(&now));

    return 0;
}

// returns new commit_ID
int incLastCommitId()
{
    FILE *file = fopen(".vgit\\lastCommitId.txt", "r");
    if (file == NULL)
        return -1;

    FILE *tmpFile = fopen(".vgit\\tmpLastCommitId.txt", "w");
    if (tmpFile == NULL)
        return -1;

    int lastCommitId;

    fscanf(file, "last commit ID : %d\n", &lastCommitId);
    lastCommitId++;
    fprintf(tmpFile, "last commit ID: %d\n", lastCommitId);

    fclose(file);
    fclose(tmpFile);

    remove(".vgit\\lastCommitId.txt");
    rename(".vgit\\tmpLastCommitId.txt", ".vgit\\lastCommitId.txt");
    return lastCommitId;
}

int trackFile(char *filepath)
{
    if (isTracked(filepath))
        return 0;

    FILE *file = fopen(".vgit\\tracks.txt", "a");
    if (file == NULL)
        return 1;
    fprintf(file, "%s\n", filepath);
    return 0;
}

bool isTracked(char *filepath)
{
    FILE *file = fopen(".vgit\\tracks.txt", "r");
    if (file == NULL)
        return false;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n')
        {
            line[length - 1] = '\0';
        }

        if (strcmp(line, filepath) == 0)
            return true;
    }
    fclose(file);

    return false;
}

void setShortcutMessage(int argc, const char **argv)
{

    file = fopen(".vgit\\shortcutMessage.txt", "a");
    fprintf(file, "%s : %s\n", argv[5], argv[3]);
    fclose(file);
}

void replaceShortcutMessage(int argc, const char **argv)
{

    int shortcutMessagesNumber = lineCounter(".vgit\\shortcutMessage.txt");
    char shortcutMessages[shortcutMessagesNumber][1024];

    file = fopen(".vgit\\shortcutMessage.txt", "r");
    for (int i = 0; i < shortcutMessagesNumber; i++)
    {
        fgets(shortcutMessages[i], sizeof(shortcutMessages[i]), file);
    }
    fclose(file);

    int flag = 0;
    file = fopen(".vgit\\shortcutMessage.txt", "w");
    for (int i = 0; i < shortcutMessagesNumber; i++)
    {
        if (strncmp(argv[5], shortcutMessages[i], strlen(argv[5])) == 0)
        {
            fprintf(file, "%s : %s\n", argv[5], argv[3]);
            flag = 1;
        }
        else
        {
            fputs(shortcutMessages[i], file);
        }
    }
    if (flag == 0)
    {
        printf("shortcut name is invalid");
    }
    fclose(file);
}

void removeShortcutMessage(int argc, const char **argv)
{
    if (argc < 4)
    {
        printf("please use the correct format");
        return;
    }

    int shortcutMessagesNumber = lineCounter(".vgit\\shortcutMessage.txt");
    char shortcutMessages[shortcutMessagesNumber][1024];

    file = fopen(".vgit\\shortcutMessage.txt", "r");
    for (int i = 0; i < shortcutMessagesNumber; i++)
    {
        fgets(shortcutMessages[i], sizeof(shortcutMessages[i]), file);
    }
    fclose(file);

    int flag = 0;
    file = fopen(".vgit\\shortcutMessage.txt", "w");
    for (int i = 0; i < shortcutMessagesNumber; i++)
    {
        if (strncmp(argv[3], shortcutMessages[i], strlen(argv[3])) == 0)
        {
            flag = 1;
        }
        else
        {
            fputs(shortcutMessages[i], file);
        }
    }
    if (flag == 0)
    {
        printf("shortcut name is invalid");
    }
    fclose(file);
}

int runLog(int argc, const char **argv)
{

    struct stat fileStat;
    struct dirent *entry;
    DIR *directory;

    char dirName[128] = ".vgit\\commits";

    directory = opendir(dirName);
    if (directory == NULL)
    {
        printf("Unable to read directory");
        return 1;
    }

    struct FileMetadata files[1000]; // Assuming a maximum of 1000 files
    int filesNum = 0;

    // Read each entry in the directory
    while ((entry = readdir(directory)) != NULL)
    {
        char path[256];
        snprintf(path, sizeof(path), "%s\\%s", dirName, entry->d_name);
        if (stat(path, &fileStat) == 0 && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            strcpy(files[filesNum].name, path);
            files[filesNum].modified_time = fileStat.st_mtime;
            filesNum++;
        }
    }
    closedir(directory);

    // Sort the files based on modified time
    qsort(files, filesNum, sizeof(struct FileMetadata), compare);

    if (argc == 2)
    {
        for (int i = 0; i < filesNum; ++i, fputs("\n--------------------\n", stdout))
        {
            char path[1024];
            sprintf(path, "%s\\commitInfo.txt", files[i].name);

            file = fopen(path, "r");
            char line[1024];
            while (fgets(line, sizeof(line), file) != NULL)
            {
                fputs(line, stdout);
            }
        }
    }
    else
    {
        if (strcmp(argv[2], "-n") == 0)
        {
            int Number;
            sscanf(argv[3], "%d", &Number);

            for (int i = 0; i < Number; ++i, fputs("\n--------------------\n", stdout))
            {
                char path[1024];
                sprintf(path, "%s\\commitInfo.txt", files[i].name);

                file = fopen(path, "r");
                char line[1024];
                while (fgets(line, sizeof(line), file) != NULL)
                {
                    fputs(line, stdout);
                }
            }
        }
        else if (strcmp(argv[2], "-branch") == 0)
        {

            char branchName[128];
            strcpy(branchName, argv[3]);

            file = fopen(".vgit\\branches.txt", "r");
            int flag = 0;
            char line[1024];
            while (fgets(line, sizeof(line), file) != NULL)
            {
                if (strncmp(line, branchName, strlen(branchName)) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
            {
                printf("Branch <%s> doesn't exist", branchName);
                return 1;
            }

            for (int i = 0; i < filesNum; ++i)
            {
                char path[1024];
                sprintf(path, "%s\\commitInfo.txt", files[i].name);

                file = fopen(path, "r");
                char data[6][1024];
                int index = 0;
                while (fgets(data[index], sizeof(data[index]), file) != NULL)
                {
                    index++;
                }

                char stringToCompare[1024];
                sprintf(stringToCompare, "branch : %s\n", branchName);

                if (strcmp(data[4], stringToCompare) == 0)
                {
                    for (int j = 0; j <= index; j++)
                    {
                        fputs(data[j], stdout);
                    }
                    fputs("\n--------------------\n", stdout);
                }
            }
        }
        else if (strcmp(argv[2], "-author") == 0)
        {
            char authorName[128];
            strcpy(authorName, argv[3]);

            for (int i = 0; i < filesNum; ++i)
            {
                char path[1024];
                sprintf(path, "%s\\commitInfo.txt", files[i].name);

                file = fopen(path, "r");
                char data[6][1024];
                int index = 0;
                while (fgets(data[index], sizeof(data[index]), file) != NULL)
                {
                    index++;
                }

                char stringToCompare[1024];
                sprintf(stringToCompare, "UserName : %s\n", authorName);

                if (strcmp(data[2], stringToCompare) == 0)
                {
                    for (int j = 0; j <= index; j++)
                    {
                        fputs(data[j], stdout);
                    }
                    fputs("\n--------------------\n", stdout);
                }
            }
        }
        else if (strcmp(argv[2], "-since") == 0)
        {
            char strTime[128];
            strcpy(strTime, argv[3]);

            struct tm structTime;

            if (sscanf(strTime, "%d-%d-%d %d:%d:%d",
                       &structTime.tm_year, &structTime.tm_mon, &structTime.tm_mday,
                       &structTime.tm_hour, &structTime.tm_min, &structTime.tm_sec) != 6)
            {
                printf("Error parsing time");
                return 1;
            }

            // Adjust struct tm fields
            structTime.tm_year -= 1900; // Years since 1900
            structTime.tm_mon -= 1;     // Months start from 0
            // No need for adjustment for day of the month (tm_mday)

            time_t timeValue;

            timeValue = mktime(&structTime);
            if (timeValue == -1)
            {
                perror("Error converting time");
                return 1;
            }

            for (int i = 0; i < filesNum; ++i)
            {
                char path[1024];
                sprintf(path, "%s\\commitInfo.txt", files[i].name);

                file = fopen(path, "r");
                char data[6][1024];
                int index = 0;
                while (fgets(data[index], sizeof(data[index]), file) != NULL)
                {
                    index++;
                }

                if (difftime(files[i].modified_time, timeValue) > 0)
                {
                    for (int j = 0; j <= index; j++)
                    {
                        fputs(data[j], stdout);
                    }
                    fputs("\n--------------------\n", stdout);
                }
            }
        }
        else if (strcmp(argv[2], "-before") == 0)
        {
            char strTime[128];
            strcpy(strTime, argv[3]);

            struct tm structTime;

            if (sscanf(strTime, "%d-%d-%d %d:%d:%d",
                       &structTime.tm_year, &structTime.tm_mon, &structTime.tm_mday,
                       &structTime.tm_hour, &structTime.tm_min, &structTime.tm_sec) != 6)
            {
                printf("Error parsing time");
                return 1;
            }

            // Adjust struct tm fields
            structTime.tm_year -= 1900; // Years since 1900
            structTime.tm_mon -= 1;     // Months start from 0
            // No need for adjustment for day of the month (tm_mday)

            time_t timeValue;

            timeValue = mktime(&structTime);
            if (timeValue == -1)
            {
                perror("Error converting time");
                return 1;
            }

            for (int i = 0; i < filesNum; ++i)
            {
                char path[1024];
                sprintf(path, "%s\\commitInfo.txt", files[i].name);

                file = fopen(path, "r");
                char data[6][1024];
                int index = 0;
                while (fgets(data[index], sizeof(data[index]), file) != NULL)
                {
                    index++;
                }

                if (difftime(files[i].modified_time, timeValue) < 0)
                {
                    for (int j = 0; j <= index; j++)
                    {
                        fputs(data[j], stdout);
                    }
                    fputs("\n--------------------\n", stdout);
                }
            }
        }
        else if (strcmp(argv[2], "-search") == 0)
        {
            char word[128];
            strcpy(word, argv[3]);

            for (int i = 0; i < filesNum; ++i)
            {
                char path[1024];
                sprintf(path, "%s\\commitInfo.txt", files[i].name);

                file = fopen(path, "r");
                char data[6][1024];
                int index = 0;
                while (fgets(data[index], sizeof(data[index]), file) != NULL)
                {
                    index++;
                }

                if (strstr(data[1], word) != NULL)
                {
                    for (int j = 0; j <= index; j++)
                    {
                        fputs(data[j], stdout);
                    }
                    fputs("\n--------------------\n", stdout);
                }
            }
        }
    }
}
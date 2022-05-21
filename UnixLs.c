#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

int printLongFileOption(struct stat fileStat, char *name, char *presentFile);
void myLs(char *directory);
void myRecursiveLs(char *directory);
int isSymbolicLink(char *directory);

int flagR = 0;
int flagL = 0;
int flagI = 0;
int baseDirFlag = 0;
int flagDir = 0;

void myLs(char *directory)
{
    struct dirent *d;
    DIR *navigator = opendir(directory);

    if (navigator == NULL)
    {
        printf("\nDirectory is not reachable/invalid. Please check again.");
        exit(0);
    }

    while ((d = readdir(navigator)) != NULL)
    {
        struct stat fileStat;

        char presentFile[PATH_MAX];
        sprintf(presentFile, "%s/%s", directory, d->d_name);
        lstat(presentFile, &fileStat);

        //I guess this removed the . stuff at the end LOL
        if (d->d_name[0] == '.')
        {
            continue;
        }

        if (flagI == 1)
        {
            //Kept the padding as 17 on Windows and 7 on Ubuntu
            //printf("%8ju ", fileStat.st_ino);
            printf("%7ju ", fileStat.st_ino);
        }

        if (flagL == 1)
        {
            printLongFileOption(fileStat, d->d_name, presentFile);
        }
        else
        {
            if (S_ISDIR(fileStat.st_mode) == 1)
            {
                printf("\033[1;34m");
                printf(" %s ", d->d_name);
            }
            else if (S_ISLNK(fileStat.st_mode) == 1)
            {
                printf("\033[1;36m");
                printf(" %s ", d->d_name);
            }
            else if (fileStat.st_mode & S_IXUSR)
            {
                printf("\033[0;32m");
                printf(" %s ", d->d_name);
            }
            else
            {
                printf("\033[0;37m");
                printf(" %s ", d->d_name);
            }
            printf("\033[0;37m");
        }
        printf("\n");
    }
    closedir(navigator);
}

void myRecursiveLs(char *directory)
{
    DIR *navigator;
    struct dirent *d;

    navigator = opendir(directory);

    if (navigator == NULL)
    {
        printf("\nDirectory is not reachable/invalid. Please check again.\n");
        exit(0);
    }

    if(baseDirFlag == 0){
        printf("\n%s:\n", directory);
        myLs(directory);
        baseDirFlag = 1;
    }

    while ((d = readdir(navigator)) != NULL)
    {
        if (d->d_type == DT_DIR)
        {
            char presentDirectory[1000];
            if (d->d_name[0] == '.')
            {
                continue;
            }
            snprintf(presentDirectory, sizeof(presentDirectory), "%s/%s", directory, d->d_name);
            printf("\n%s:\n", presentDirectory);
            myLs(presentDirectory);
            myRecursiveLs(presentDirectory);
        }
    }
    closedir(navigator);
}

int printLongFileOption(struct stat fileStat, char *name, char *presentFile)
{
    //File permissions
    if((S_ISDIR(fileStat.st_mode)))
        printf("d");
    else if(S_ISLNK(fileStat.st_mode) == 1)
        printf("l");
    else
        printf("-");

    if((fileStat.st_mode & S_IRUSR))
        printf("r");
    else
        printf("-");

    if((fileStat.st_mode & S_IWUSR))
        printf("w");
    else
        printf("-");

    if((fileStat.st_mode & S_IXUSR))
        printf("x");
    else
        printf("-");

    if((fileStat.st_mode & S_IRGRP))
        printf("r");
    else
        printf("-");

    if((fileStat.st_mode & S_IWGRP))
        printf("w");
    else
        printf("-");

    if((fileStat.st_mode & S_IXGRP))
        printf("x");
    else
        printf("-");

    if((fileStat.st_mode & S_IROTH))
        printf("r");
    else
        printf("-");

    if((fileStat.st_mode & S_IWOTH))
        printf("w");
    else
        printf("-");

    if((fileStat.st_mode & S_IXOTH))
        printf("x");
    else
        printf("-");

    //Number of links
    printf(" %d ", fileStat.st_nlink);

    //Username of owner
    struct passwd *pw = NULL;
    pw = getpwuid(fileStat.st_uid);
    if (pw)
    {
        printf("%s", pw->pw_name);
    }
    else
    {
        printf(" OWNERNOT FOUND ");
    }

    //Groupname of owner
    struct group *grp;
    grp = getgrgid(fileStat.st_gid);
    if (grp)
    {
        printf(" %s", grp->gr_name);
    }
    else
    {
        printf("GROUP NOT FOUND ");
    }

    //Size of file in bytes
    printf(" %8jd ", fileStat.st_size);

    //Date and time of last modification
    char dateTime[100];
    strftime(dateTime, sizeof(dateTime), "%b %d %Y %R", localtime(&(fileStat.st_ctime)));
    printf("%s ", dateTime);

    //Filename
    if (S_ISDIR(fileStat.st_mode) == 1)
    {
        printf("\033[1;34m");
        printf("%s", name);
    }
    else if (S_ISLNK(fileStat.st_mode) == 1)
    {
        printf("\033[1;36m");
        printf("%s", name);
        char linkBuf[PATH_MAX];
        ssize_t linkLen = readlink(presentFile, linkBuf, sizeof(linkBuf) - 1);
        if (linkLen != -1)
        {
            printf("\033[0;37m");
            printf(" -> %s", linkBuf);
        }
    }
    else if (fileStat.st_mode & S_IXUSR)
    {
        printf("\033[0;32m");
        printf("%s", name);
    }
    else
    {
        printf("\033[0;37m");
        printf("%s", name);
    }
    printf("\033[0;37m");
}

int main(int argc, char *argv[])
{
    char searchDirectory[100];

    //Set flags correctly
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strchr(argv[i], './') != NULL || strchr(argv[i], '..') != NULL)
            {
                //Ignore if args have a directory along with or that'll mess with flags
                continue;
            }
            if (strchr(argv[i], '-i') != NULL || strchr(argv[i], 'l') != NULL || strchr(argv[i], 'R') != NULL)
            {
                if (strchr(argv[i], '-i') != NULL)
                {
                    //printf("\ni is found");
                    flagI = 1;
                }
                if (strchr(argv[i], '-l') != NULL)
                {
                    //printf("\nl is found");
                    flagL = 1;
                }
                if (strchr(argv[i], '-R') != NULL)
                {
                    //printf("\nR is found");
                    flagR = 1;
                }
            }
        }
    }

    //Set directory of ls
    if (strchr(argv[argc - 1], '.') != NULL && argc > 1)
    {
        flagDir = 1;
        strcpy(searchDirectory, argv[argc - 1]);
    }
    if (flagDir == 0)
    {
        strcpy(searchDirectory, ".");
    }

    //printf("\nSearch directroy is: %s", searchDirectory);
    if (flagR == 0)
    {
        myLs(searchDirectory);
    }
    else
    {
        myRecursiveLs(searchDirectory);
    }
    printf("\n");
}

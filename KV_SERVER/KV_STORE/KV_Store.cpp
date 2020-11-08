#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h> 

#define EMPTY "EMPTY"
#define FILLED "FILLD"
#define DELETED "DELET"
#define KEYLEN 256
#define VALLEN 256
#define OVERHEAD 7 // len 5 metadata and ":" and "\n"
#define FILLER "-"

const char * empty = "EMPTY";
const char * filled = "FILLD";
const char * deleted = "DELET";
const char * filler = "-";
const int ENTRYSIZE = (KEYLEN + VALLEN + OVERHEAD);
const int ENTRIES = 256;
const char *kv_store_name = "kv_hash_db.txt";
char return_str[257];


unsigned long hash(char *str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}


int string_length(char * str)
{
    int i = 0;
    while(str[i]!= '\0')
    {
        i++;
    }
    return i;
}

/*
    insert function with linear probing
*/

int insert(FILE* file_fd, char * key, char * value)
{
    int key_len, val_len;
    key_len = val_len = 0;
    char status[6];
    char key_with_filler[257];
    char value_with_filler[257];
    char temp_str[257];
    if(key == NULL || value == NULL)
        return 0;

    //calculate length of key
    key_len = string_length(key);

    //calculate length of value
    val_len = string_length(value);

    if(key_len > KEYLEN || val_len > VALLEN)
        return 0;
    
    if(key_len<=KEYLEN)
    {
        strcpy(key_with_filler, key);
        for(int i = key_len; i<256; i++)
        {
            key_with_filler[i] = '-';
        }
        key_with_filler[256] = '\0'; 
    }

    if(val_len<=VALLEN)
    {
        strcpy(value_with_filler, value);
        for(int i = val_len; i<256; i++)
        {
            value_with_filler[i] = '-';
        }
        value_with_filler[256] = '\0';
    }

    int index = hash(key);
    index %= ENTRIES;
    if(index<0)
    {
        index += ENTRIES;
    }

    fseek(file_fd, 0, SEEK_SET);
    printf("Index is %d\n", index);

    for(int i = 0; i < ENTRIES; i++)
    {
        fseek(file_fd, ENTRYSIZE*index, SEEK_SET);
        fread(status, 5, 1, file_fd);
        status[5] = '\0';
        printf("Status is %s\n", status);
        if(strcmp(status, empty) == 0 || strcmp(status, deleted) == 0)
        {
            fseek(file_fd, -5, SEEK_CUR);
            fwrite(filled, 5, 1, file_fd);
            fwrite(key_with_filler, 256, 1, file_fd);
            fseek(file_fd, 1, SEEK_CUR);
            fwrite(value_with_filler, 256, 1, file_fd);
            return 1;
        } else if (strcmp(status, filled) == 0)
        {
            fread(temp_str, 256, 1, file_fd);
            temp_str[256] = '\0';
            if(strcmp(temp_str, key_with_filler) == 0)
            {
                printf("Updated Value\n");
                fseek(file_fd, 1, SEEK_CUR);
                fwrite(value_with_filler, 256, 1, file_fd);
                return 1;
            }
        }
        index++;
        index %= 256;
        if(index < 0)
        {
            index += 256;
        }
    }

    return -1;
}


/*
    Delete
*/

int delete_entry(FILE * file_fd, char *key)
{
    if(key == NULL)
        return -1;
    
    int key_len = string_length(key);
    char key_with_filler[257];
    char temp_str[257];
    char status[6];

    if(key_len > KEYLEN)
        return -1;
    
    if(key_len <= KEYLEN)
    {
        strcpy(key_with_filler, key);
        for(int i = key_len; i < KEYLEN; i++)
        {
            key_with_filler[i] = '-';
        }
        key_with_filler[256] = '\0';
    }

    int index = hash(key);
    index %= ENTRIES;
    if(index<0)
    {
        index += ENTRIES;
    }

    printf("Index: %d\n", index);

    for(int i = 0; i < 256; i++)
    {
        fseek(file_fd, ENTRYSIZE*index, SEEK_SET);
        fread(status, 5, 1, file_fd);
        status[5] = '\0';
        printf("Status is %s\n", status);
        if(strcmp(status, empty)==0)
        {
            return -1;
        }
        else if (strcmp(status, filled)==0)
        {
            fread(temp_str, 256, 1, file_fd);
            temp_str[256] = '\0';
            fseek(file_fd, ENTRYSIZE*index, SEEK_SET);
            if(strcmp(temp_str, key_with_filler) == 0)
            {
                printf("Here\n");
                // fseek(file_fd, -5, SEEK_CUR);
                fwrite(deleted, 5, 1, file_fd);
                for (int j = 0; j < KEYLEN; j++)
                {
                    fwrite(filler, 1 , 1, file_fd);
                }
                fseek(file_fd, 1, SEEK_CUR);
                for (int j = 0; j < VALLEN; j++)
                {
                    fwrite(filler, 1 , 1, file_fd);
                }
                return 1;
            }
        }
        index++;
        index %= 256;
        if(index < 0)
        {
            index += 256;
        }        
    }
}

char * search_value(FILE* file_fd, char * key)
{
    if(key == NULL)
        return NULL;
    
    int key_len = string_length(key);
    char key_with_filler[257];
    char temp_str[257];
    char status[6];

    if(key_len > KEYLEN)
        return NULL;
    
    if(key_len <= KEYLEN)
    {
        strcpy(key_with_filler, key);
        for(int i = key_len; i < KEYLEN; i++)
        {
            key_with_filler[i] = '-';
        }
        key_with_filler[256] = '\0';
    }

    int index = hash(key);
    index %= ENTRIES;
    if(index<0)
    {
        index += ENTRIES;
    }

    printf("Index: %d\n", index);

    for(int i = 0; i < 256; i++)
    {
        fseek(file_fd, ENTRYSIZE*index, SEEK_SET);
        fread(status, 5, 1, file_fd);
        status[5] = '\0';
        printf("Status is %s\n", status);
        if(strcmp(status, empty)==0)
        {
            return NULL;
        }
        else if (strcmp(status, filled)==0)
        {
            fread(temp_str, 256, 1, file_fd);
            temp_str[256] = '\0';
            // fseek(file_fd, ENTRYSIZE*index, SEEK_SET);
            if(strcmp(temp_str, key_with_filler) == 0)
            {
                printf("Here\n");
                fseek(file_fd, 1, SEEK_CUR);
                fread(return_str, 256, 1, file_fd);
                return_str[256] = '\0';
                return return_str;
            }
        }
        index++;
        index %= 256;
        if(index < 0)
        {
            index += 256;
        }        
    }

}


/*
  initialise the file 
  check if it already exists 
  or
  create a new file
*/
FILE* initialise_kv_store()
{
    // File pointer to file
    FILE* file_fd;
    // check if file exists or not
    if(access(kv_store_name, W_OK) == 0)
    {
        //file exists and return fd
        printf("File Already Exists\n");
        file_fd =  fopen(kv_store_name, "r+");
        return file_fd;
    } else
    {
        //create the file and add hash entries
        file_fd = fopen(kv_store_name, "w+");
        //create hash entries in table
        for(int i = 0; i < ENTRIES; i++)
        {
            fwrite(empty, 5, 1, file_fd);
            //fprintf(file_fd, "%3d", 0);
            for (int j = 0; j < KEYLEN; j++)
            {
                fwrite(filler, 1 , 1, file_fd);
            }
            fwrite(":", 1, 1, file_fd);
            for (int j = 0; j < VALLEN; j++)
            {
                fwrite(filler, 1 , 1, file_fd);
            }
            fwrite("\n", 1, 1, file_fd);
        }
    }
    
    return file_fd;
}



int main()
{
    FILE* file = initialise_kv_store();
    char* key = "Hello";
    char* val = "YOU";
    if(insert(file, "Abhishek", "Raut") == 1)
    {
        printf("Element Inserted\n");
    }

    printf("Search %s\n", search_value(file, "Abhishek"));

    fclose(file);
    return 0;
}
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
#define OVERHEAD 10
#define FILLER "-"

const char * empty = "EMPTY";
const char * filled = "FILLD";
const char * deleted = "DELET";
const char * filler = "-";
const int ENTRYSIZE = (KEYLEN + VALLEN + OVERHEAD);
const int ENTRIES = 256;
const char *kv_store_name = "kv_hash_db.txt";


unsigned long hash(char *str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	printf("hash=%zu\n", hash);
	return hash;
}



/*
    insert function with linear probing
*/

int insert(FILE* file_fd, char * key, char * value)
{
    int key_len, val_len;
    key_len = val_len = 0;
    char status[5];
    
    while(key[key_len] != '\0')
    {
        key_len++;
    }

    while(value[val_len]!= '\0')
    {
        val_len++;
    }

    int index = hash(key);
    index %= ENTRIES;

    printf("Index is %d\n", index);
    printf("%d\n", ENTRYSIZE*index);
    fseek(file_fd, ENTRYSIZE*index, SEEK_SET);
    fread(status, 5, 1, file_fd);
    printf("%s\n", status);
    if(strcmp(status, empty) == 0)
    {
        fseek(file_fd, -5, SEEK_CUR);
        fwrite(filled, 5, 1, file_fd);
        fseek(file_fd, 3, SEEK_CUR);
        fwrite(key, key_len, 1, file_fd);
        fseek(file_fd, 257-key_len, SEEK_CUR);
        fwrite(value, val_len, 1, file_fd);
    }

    return 1;
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
            fprintf(file_fd, "%3d", 0);
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
    char* val = "World";
    int i = insert(file, key, val);
    int j = insert(file, "Abishek", "Raut");
    fclose(file);
    return 0;
}
#include "projectA.h"
#include "projectB.h"


void HFL_init() 
{
    fprintf(stderr, "Calling HFL_init\n");
}


errCode HFL_create_file(char* filename)
{
    fprintf(stderr, "Attempting to create file with name %s\n", filename);
    return 0;
}


fileDesc HFL_open_file(char* filename)
{
    fprintf(stderr, "Opening file with name %s\n", filename);
    return 0;
}


errCode HFL_close_file(fileDesc fd)
{
    fprintf(stderr, "Attempting to close file %d\n", fd);
    return 0;
}


recordID HFL_insert_rec(fileDesc fd, record* rec)
{
    fprintf(stderr, "Attempting to insert record into file %d\n", fd);
    return 0;
}

errCode HFL_delete_rec(fileDesc fd, recordID rid)
{
    fprintf(stderr, "Attempting to delete record with ID %d from file %d\n", rid, fd);
    return 0;
}


errCode HFL_get_first_rec(fileDesc fd, record** rec)
{
    fprintf(stderr, "Attempting to get first record from file %d\n", fd);
    return 0;
}

errCode HFL_get_next_rec(fileDesc fd, record** rec)
{
    fprintf(stderr, "Attempting to get next record from file %d\n", fd);
    return 0;
}

errCode HFL_get_this_rec(fileDesc fd, recordID rid, record** rec)
{
    fprintf(stderr, "Attempting to get record %d from file %d\n", rid, fd);
    return 0;
}

scanDesc HFL_open_file_scan(fileDesc fd)
{
    fprintf(stderr, "Opening a file scan for file %d\n", fd);
    return 0;
}

errCode HFL_find_next_rec(scanDesc sd, record** rec)
{
    fprintf(stderr, "scanDesc %d is scanning to the next record\n", sd);
    return 0;
}

errCode HFL_close_file_scan(scanDesc sd)
{
    fprintf(stderr, "Closing file scan %d", sd);
    return 0;
}

void HFL_print_error(errCode ec)
{
  fprintf(stderr, "Printing error code %d\n", ec);
}

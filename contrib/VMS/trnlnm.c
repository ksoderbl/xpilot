#include <starlet.h>
#include <lib$routines.h>
#include <ssdef.h>
#include <lnmdef.h>
#include <descrip.h>

typedef struct {
    unsigned short   buffer_length;
    unsigned short   item_code;
    char            *buffer_addr;
    short           *return_len_addr;
    unsigned         terminator;
} item_list_t;

int vms_trnlnm (name, index, translated_name)

  char *name;
  int   index;
  char *translated_name;	/* Trust the user to provide a 256 char str. */
{
    $DESCRIPTOR(table_name, "LNM$DCL_LOGICAL");

    struct dsc$descriptor_s log_name = { 
      strlen (name), DSC$K_DTYPE_T, DSC$K_CLASS_S, name};

    int attr =  LNM$M_CASE_BLIND;
    int         level;
    int         status;
    short       return_length;
    item_list_t item_list;

    item_list.buffer_length = 255;
    item_list.item_code = LNM$_STRING;
    item_list.buffer_addr = translated_name;
    item_list.return_len_addr = &return_length;
    item_list.terminator = 0;

    status = sys$trnlnm(&attr, &table_name, &log_name, 0, &item_list);

    if (status != SS$_NORMAL)
      return -1;

    translated_name[return_length] = '\0';

  return ++level;
}

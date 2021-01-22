

/*=====================================================================
  *�ļ���sgdev_curl.h
 *
  *���������нӿ�
 *
  *���ߣ�����			2020��10��13��20:14:07
  *
  *�޸ļ�¼��

 =====================================================================*/


#ifndef _SGDEV_CURL_H_
#define _SGDEV_CURL_H_

#ifdef __cplusplus
extern "C" {  
#endif

#define DEFAULT_FILE_PATH "/mnt/internal_storage"   

typedef struct http_file {
    char filename[512];
    FILE *stream;
}http_file_s;


int sg_curl_download_file(char* filename, char* filepath);
int sg_curl_upload_file(char* file, char* url);
int sg_file_download(file_info_s file, char* msg);


#ifdef __cplusplus
}
#endif 

#endif





/*=====================================================================
  *文件：sgdev_curl.h
 *
  *描述：下行接口
 *
  *作者：田振超			2020年10月13日20:14:07
  *
  *修改记录：

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



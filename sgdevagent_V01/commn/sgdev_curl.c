
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl/curl.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "ssp_mid.h"

#include "sgdev_struct.h"
#include "sgdev_curl.h"

int sg_sign_download(sign_info_s file);

static void sg_tolower(char* pBuf, int nLen)
{
    for (int i = 0; i < nLen; ++i)
    {
        if (*(pBuf + i) > 0)
            *(pBuf + i) = tolower(*(pBuf + i));
    }
}

//上传
static size_t sg_read_file(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t retcode;
    curl_off_t nread;
    retcode = fread(ptr, size, nmemb, stream);
    nread = (curl_off_t)retcode;
    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
        " bytes from file\n", nread);
    return retcode;
}

static size_t sg_write_file(void *buffer, size_t size, size_t nmemb, void *stream)
{
    http_file_s *out = (http_file_s *)stream;
    if (out && !out->stream) {
        out->stream = fopen(out->filename, "wb");
        if (!out->stream) {
            return -1;
        }
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

//输入filename 带路径的文件名
//输入filepath 下载路径
int sg_curl_download_file(char* filename, char* filepath)
{
    CURL *curl = NULL;
    CURLcode res;

    http_file_s httpfile;
    //sprintf_s(httpfile.filename, DATA_BUF_F512_SIZE, "%s/%s", DEFAULT_FILE_PATH,filename);
    sprintf_s(httpfile.filename, DATA_BUF_F512_SIZE, "%s", filename);
    httpfile.stream = NULL;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl == NULL) {
        return VOS_ERR;
    }
    curl_easy_setopt(curl, CURLOPT_URL, filepath);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sg_write_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpfile);
    // curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);  //ssl
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        printf("curl told us %d\n", res);
        return VOS_ERR;
    }
    if (httpfile.stream) {
        fclose(httpfile.stream);
    }
    curl_global_cleanup();
    return VOS_OK;
}


int sg_curl_upload_file(char* file, char* url)
{
    CURL *curl;
    CURLcode res;
    FILE * hd_src;
    struct stat file_info;
    curl_off_t fsize;

    if (stat(file, &file_info)) {
        printf("Couldn't open '%s': %s\n", file, strerror(errno));
        return VOS_ERR;
    }
    stat(file, &file_info);
    fsize = (curl_off_t)file_info.st_size;
    printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

    hd_src = fopen(file, "rb");
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, sg_read_file);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return VOS_ERR;
        }
        curl_easy_cleanup(curl);
    }
    fclose(hd_src);
    curl_global_cleanup();
    return VOS_OK;
}

int sg_sign_download(sign_info_s file)
{
    char filename[DATA_BUF_F512_SIZE];
    char outmd5[DATA_BUF_F64_SIZE + 1];
    int bRet = VOS_OK;
    int filesize = 0;
    sprintf_s(filename, DATA_BUF_F512_SIZE, "%s/%s", DEFAULT_FILE_PATH, file.name);

    if (sg_curl_download_file(filename, file.url) != VOS_OK) {
        return VOS_ERR;
    }

    if (file.size != 0) {
        filesize = getfilesize(filename) / 1024;  //判断文件大小
        if (abs(filesize - file.size) > 10) {
            bRet = VOS_ERR;
        }
    }
    if (strlen(file.md5) != 0) {
        if (ssp_calculate_md5_of_file(filename, outmd5, DATA_BUF_F64_SIZE) == VOS_OK) {
            sg_tolower(file.md5, strlen(file.md5));
            if (strncmp(outmd5, file.md5, strlen(outmd5)) != 0) {
                printf("%s", "sign md5 check failed \n");
                return VOS_ERR;
            }
        }
    }
    return bRet;
}


int sg_file_download(file_info_s file, char* msg)
{
    char filename[DATA_BUF_F512_SIZE + 1];
    char outmd5[DATA_BUF_F64_SIZE + 1];
    int bRet = VOS_OK;
    int filesize = 0;
    sprintf_s(filename, DATA_BUF_F512_SIZE, "%s/%s", DEFAULT_FILE_PATH, file.name);
    if (strlen(file.url) == 0) { //没有url则无需下载
        return VOS_OK;
    }
    if (sg_curl_download_file(filename, file.url) == VOS_ERR) {
        sprintf_s(msg, DATA_BUF_F64_SIZE, "%s", "download file failed");
        return VOS_ERR;
    }
    filesize = getfilesize(filename) / 1024;  //判断文件大小
    if (abs(filesize - file.size) > 10) {
        sprintf_s(msg, DATA_BUF_F64_SIZE, "%s", "file size is not correct");
        bRet = VOS_ERR;
    }
    //MD5
    if (ssp_calculate_md5_of_file(filename, outmd5, DATA_BUF_F64_SIZE) == VOS_OK) {
        printf("sgdevagent**** : outmd5 = %s.\n", outmd5);
        sg_tolower(file.md5, strlen(file.md5));
        if (strncmp(outmd5, file.md5, strlen(outmd5)) != 0) {
            sprintf_s(msg, DATA_BUF_F64_SIZE, "%s", "md5 check failed");
            return VOS_ERR;
        }
    }
    //需要签名
    if (strlen(file.sign.name) != 0) {
        if (strlen(file.sign.url) == 0) {
            bRet = VOS_OK;
        } else if (sg_sign_download(file.sign) != VOS_OK) {
            sprintf_s(msg, DATA_BUF_F64_SIZE, "%s", "download sign file fialed");
            return VOS_ERR;
        }
    }
    //判断文件类型
    if (strncmp(file.fileType, "ova", strlen(file.fileType)) != 0) {
        sprintf_s(msg, DATA_BUF_F64_SIZE, "%s", "file type is not correct");
        return VOS_ERR;
    }
    return bRet;
}

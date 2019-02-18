#include <malloc.h>
#include <stdlib.h>
#include <cstring>
#include <string>

#include "azure_prov_client/adapters/hsm_client_data.h"

typedef struct HSM_CLIENT_KEY_INFO_TAG
{
    //Sym Key
    std::string symmetrical_key;
    std::string registration_name;

    //X.509
    std::string cert;
    std::string cert_pk;
    std::string common_name;
} HSM_CLIENT_KEY_INFO;

int initialize_hsm_system()
{
    return 0;
}

void deinitialize_hsm_system()
{

}

///////
//tpm
static const HSM_CLIENT_TPM_INTERFACE tpm_interface =
{
    // hsm_client_tpm_create,
    // hsm_client_tpm_destroy,
    // hsm_client_tpm_import_key,
    // hsm_client_tpm_get_endorsement_key,
    // hsm_client_tpm_get_storage_key,
    // hsm_client_tpm_sign_data
};

const HSM_CLIENT_TPM_INTERFACE* hsm_client_tpm_interface(void)
{
    return &tpm_interface;
}

////

////
// X.509
char* custom_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
    printf(">> custom_hsm_get_certificate\n");
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the certificate for the iothub sdk to free
        // this value will be sent unmodified to the tlsio
        // layer to be processed
        HSM_CLIENT_KEY_INFO_TAG* hsm_info = (HSM_CLIENT_KEY_INFO_TAG*)handle;
        size_t len = hsm_info->cert.length();
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating certificate\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->cert.c_str());
        }
    }
    printf(">> returing: \n%s\n", result);
    return result;
}

char* custom_hsm_get_key(HSM_CLIENT_HANDLE handle)
{
    printf(">> custom_hsm_get_key\n");
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the private key for the iothub sdk to free
        // this value will be sent unmodified to the tlsio
        // layer to be processed
        HSM_CLIENT_KEY_INFO_TAG* hsm_info = (HSM_CLIENT_KEY_INFO_TAG*)handle;
        size_t len = hsm_info->cert_pk.length();
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating certificate\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->cert_pk.c_str());
        }
    }
    printf(">> returing: \n%s\n", result);
    return result;
}

char* custom_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{
    printf(">> custom_hsm_get_common_name\n");
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the common name for the iothub sdk to free
        // this value will be sent to dps
        HSM_CLIENT_KEY_INFO_TAG* hsm_info = (HSM_CLIENT_KEY_INFO_TAG*)handle;
        size_t len = hsm_info->common_name.length();
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating certificate\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->common_name.c_str());
        }
    }

    printf(">> returing: \n%s\n", result);
    return result;
}

/////


////
// Sym Key


#include <fstream>
const std::string _storage_file_name = "/var/hsm";

void read_storage(HSM_CLIENT_KEY_INFO& key_info)
{
    std::ifstream file(_storage_file_name);
    if (!file.is_open())
    {
        printf("HSM file is missing\n");
        return;
    }

    std::getline(file, key_info.symmetrical_key);
    std::getline(file, key_info.registration_name);

    printf("Loaded HSM from file: \n"
        "symmetrical_key = %s,\n"
        "registration_name = %s\n", 
        key_info.symmetrical_key.c_str(), key_info.registration_name.c_str());
}

void write_storage(HSM_CLIENT_KEY_INFO& key_info)
{
    std::ofstream file(_storage_file_name, std::ios::out | std::ios::trunc);

    if (!file.is_open())
    {
        printf("Failed to open HSM file for writing\n");
        return;
    }
   
    file << key_info.symmetrical_key << std::endl;
    file << key_info.registration_name << std::endl;
}

std::string load_file(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        printf("HSM file is missing\n");
        return {};
    }

    std::string str((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

    return str;
}

HSM_CLIENT_HANDLE hsm_client_key_create(void)
{
    printf(">> hsm_client_key_create\n");

    HSM_CLIENT_KEY_INFO* result;
    result = new HSM_CLIENT_KEY_INFO();
    if (result == NULL)
    {
        printf("Failure: malloc HSM_CLIENT_KEY_INFO.\n");
    }
    else
    {
        //memset(result, 0, sizeof(HSM_CLIENT_KEY_INFO));

        read_storage(*result);
        result->common_name = "can-cert-1";
        result->cert = load_file("/var/can-cert-1.pem");
        result->cert_pk = load_file("/var/can-cert-1.key");
    }
    return result;
}

void hsm_client_key_destroy(HSM_CLIENT_HANDLE handle)
{
    printf(">> hsm_client_key_destroy\n");
    if (handle != NULL)
    {
        HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;
        delete key_client;
        // HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;
        // if (key_client->symmetrical_key != NULL)
        // {
        //     free(key_client->symmetrical_key);
        //     key_client->symmetrical_key = NULL;
        // }
        // if (key_client->registration_name != NULL)
        // {
        //     free(key_client->registration_name);
        //     key_client->registration_name = NULL;
        // }
        // free(key_client);
    }
}

char* hsm_client_get_symmetric_key(HSM_CLIENT_HANDLE handle)
{
    printf(">> hsm_client_get_symmetric_key\n");
    char* result = 0;
    if (handle == NULL)
    {
        /* Codes_SRS_HSM_CLIENT_RIOT_07_010: [ if handle is NULL, hsm_client_riot_get_certificate shall return NULL. ] */
        //LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;

        result = (char*)malloc(key_client->symmetrical_key.length()+1);
        strcpy(result, key_client->symmetrical_key.c_str());

        printf("key_client->symmetrical_key: %s  result = %s, reslen=%d\n ", key_client->symmetrical_key.c_str(), result, strlen(result));
    }
    return result;
}

char* hsm_client_get_registration_name(HSM_CLIENT_HANDLE handle)
{
    printf(">> hsm_client_get_registration_name\n");
    char* result = 0;
    if (handle == NULL)
    {
        /* Codes_SRS_HSM_CLIENT_RIOT_07_026: [ if handle is NULL, hsm_client_riot_get_common_name shall return NULL. ] */
        printf("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;

        result = (char*)malloc(key_client->registration_name.length()+1);
        strcpy(result, key_client->registration_name.c_str());

        printf("key_client->registration_name: %s  result = %s, reslen=%d\n", key_client->registration_name.c_str(), result, strlen(result));
        
    }
    return result;
}

int hsm_client_set_key_info(HSM_CLIENT_HANDLE handle, const char* reg_name, const char* symm_key)
{
    printf(">> hsm_client_set_key_info\n");

    if (handle == NULL || reg_name == NULL || symm_key == NULL)
    {
        printf("Invalid parameter specified handle: %p, reg_name: %p, symm_key: %p", handle, reg_name, symm_key);
        return -1;
    }

    HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;
    key_client->registration_name = reg_name;
    key_client->symmetrical_key = symm_key;

    printf("registration_name = %s, symmetrical_key = %s\n", 
        key_client->registration_name.c_str(),
         key_client->symmetrical_key.c_str());

    write_storage(*key_client);

    return 0;
    // int result;
    // if (handle == NULL || reg_name == NULL || symm_key == NULL)
    // {
    //     LogError("Invalid parameter specified handle: %p, reg_name: %p, symm_key: %p", handle, reg_name, symm_key);
    //     result = __FAILURE__;
    // }
    // else
    // {
    //     HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;

    //     char* temp_reg_name;
    //     char* temp_key;
    //     if (mallocAndStrcpy_s(&temp_reg_name, reg_name) != 0)
    //     {
    //         LogError("Failure allocating registration name");
    //         result = __FAILURE__;
    //     }
    //     else if (mallocAndStrcpy_s(&temp_key, symm_key) != 0)
    //     {
    //         LogError("Failure allocating symmetric key");
    //         free(temp_reg_name);
    //         result = __FAILURE__;
    //     }
    //     else
    //     {
    //         if (key_client->symmetrical_key != NULL)
    //         {
    //             free(key_client->symmetrical_key);
    //         }
    //         if (key_client->registration_name != NULL)
    //         {
    //             free(key_client->registration_name);
    //         }
    //         key_client->symmetrical_key = temp_key;
    //         key_client->registration_name = temp_reg_name;
    //         result = 0;
    //     }
    // }
    // return result;
}

int hsm_client_set_key_info_void(HSM_CLIENT_HANDLE handle, const char* reg_name, const char* symm_key)
{
    return 0;
}

static const HSM_CLIENT_KEY_INTERFACE key_interface =
{
    hsm_client_key_create,
    hsm_client_key_destroy,
    hsm_client_get_symmetric_key,
    hsm_client_get_registration_name,
    hsm_client_set_key_info_void
};

const HSM_CLIENT_KEY_INTERFACE* hsm_client_key_interface(void)
{
    return &key_interface;
}


static const HSM_CLIENT_X509_INTERFACE x509_interface =
{
    hsm_client_key_create,
    hsm_client_key_destroy,
    custom_hsm_get_certificate,
    custom_hsm_get_key,
    custom_hsm_get_common_name
};

const HSM_CLIENT_X509_INTERFACE* hsm_client_x509_interface(void)
{
    return &x509_interface;
}

///





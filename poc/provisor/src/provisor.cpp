
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "iothub.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/threadapi.h"

#include "azure_prov_client/prov_device_client.h"
#include "azure_prov_client/prov_security_factory.h"

#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"

// This sample is to demostrate iothub reconnection with provisioning and should not
// be confused as production code
#define LOG(stream) std::cout << "\033[33m" << stream << "\033[0m" << std::endl;

DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

static const char* global_prov_uri = "global.azure-devices-provisioning.net";
static const char* id_scope = "0ne00043D3A";

static bool g_registration_complete = false;

static std::string _hub_uri;
static std::string _device_id;

static void registration_status_callback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
    (void)user_context;
    LOG("Provisioning Status:" << ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void register_device_callback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
    (void)user_context;
    if (register_result == PROV_DEVICE_RESULT_OK)
    {
        LOG("Registration Information received from service: " << std::endl <<
        "Hub:       " << iothub_uri << std::endl <<
        "DeviceId:  " << device_id);       
    }
    else
    {
        LOG("Failure registering device: " << ENUM_TO_STRING(PROV_DEVICE_RESULT, register_result));
    }
    g_registration_complete = true;
}



int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <scope_id> s/c" << std::endl;
        return -1;
    }


    SECURE_DEVICE_TYPE hsm_type;
    //hsm_type = SECURE_DEVICE_TYPE_TPM;

    std::string scope_id = argv[1];

    if(std::string(argv[2]) == "c")
    {
        hsm_type = SECURE_DEVICE_TYPE_X509;
        LOG("Using X509 attestation");
    }
    else
    {
        hsm_type = SECURE_DEVICE_TYPE_SYMMETRIC_KEY;
        LOG("Using SymKey attestation");
    }

    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();
    (void)prov_dev_security_init(hsm_type);

    // Set the symmetric key if using they auth type
    //prov_dev_set_symmetric_key_info("imei0123456789", "Lmhse8I8mqe1HEJPrLELq/mayg/4eA6P+OHM3DAt3342CcNhcI9D3tiR5ytchknXMpPTj6qpEJD6hzPHeMB1Sg==");

    PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport;

  
    prov_transport = Prov_Device_MQTT_Protocol;

    printf("Provisioning API Version: %s\r\n", Prov_Device_GetVersionString());

    PROV_DEVICE_RESULT prov_device_result = PROV_DEVICE_RESULT_ERROR;
    PROV_DEVICE_HANDLE prov_device_handle;
    if ((prov_device_handle = Prov_Device_Create(global_prov_uri, scope_id.c_str(), prov_transport)) == NULL)
    {
        LOG("failed calling Prov_Device_Create");
    }
    else
    {
     
        bool traceOn = true;
        Prov_Device_SetOption(prov_device_handle, PROV_OPTION_LOG_TRACE, &traceOn);

        // This option sets the registration ID it overrides the registration ID that is 
        // set within the HSM so be cautious if setting this value
        //Prov_Device_SetOption(prov_device_handle, PROV_REGISTRATION_ID, "[REGISTRATION ID]");

        prov_device_result = Prov_Device_Register_Device(prov_device_handle, register_device_callback, NULL, registration_status_callback, NULL);

        LOG("nRegistering Device...");
        do
        {
            ThreadAPI_Sleep(1000);
        } while (!g_registration_complete);

        Prov_Device_Destroy(prov_device_handle);
    }
    prov_dev_security_deinit();

    // Free all the sdk subsystem
    IoTHub_Deinit();

    

    (void)printf("Press enter key to exit:\r\n");
    (void)getchar();

    return 0;
}

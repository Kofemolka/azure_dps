#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "iothub.h"
#include "azure_prov_client/iothub_security_factory.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothubtransportmqtt.h"

#include "azure_c_shared_utility/threadapi.h"

#include "azure_c_shared_utility/tickcounter.h"

#define MESSAGES_TO_SEND            2
#define TIME_BETWEEN_MESSAGES       2

#define LOG(stream) std::cout << "\033[33m" << stream << "\033[0m" << std::endl;

typedef struct IOTHUB_CLIENT_SAMPLE_INFO_TAG
{
    int stop_running;
    int connected;
} IOTHUB_CLIENT_SAMPLE_INFO;

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    IOTHUB_CLIENT_SAMPLE_INFO* iothub_info = (IOTHUB_CLIENT_SAMPLE_INFO*)user_context;
    if (iothub_info != NULL)
    {
        if (reason == IOTHUB_CLIENT_CONNECTION_OK && result == IOTHUB_CLIENT_CONFIRMATION_OK)
        {
            iothub_info->connected = 1;
        }
        else
        {
            iothub_info->connected = 0;
            LOG("Disconnection from service encountered");
        }
    }
}

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    (void)message;
    IOTHUB_CLIENT_SAMPLE_INFO* iothub_info = (IOTHUB_CLIENT_SAMPLE_INFO*)user_context;
    LOG("Stop message recieved from IoTHub");
    iothub_info->stop_running = 1;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void iothub_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    (void)reason;
    if (user_context == NULL)
    {
        LOG("iothub_connection_status user_context is NULL");
    }
    else
    {
        IOTHUB_CLIENT_SAMPLE_INFO* iothub_info = (IOTHUB_CLIENT_SAMPLE_INFO*)user_context;
        if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
        {
            iothub_info->connected = 1;
        }
        else
        {
            iothub_info->connected = 0;
            iothub_info->stop_running = 1;
        }
    }
}

static void DeviceTwinHandler(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallbac)
{
    LOG("Device Twin received: " << payLoad);
}

int main(int argc, char** argv)
{
    if(argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " s/c <hub_uri> <device_id>" << std::endl;
        return -1;
    }

    IOTHUB_SECURITY_TYPE security_type = std::string(argv[1]) == "s" ? 
        IOTHUB_SECURITY_TYPE_SYMMETRIC_KEY : IOTHUB_SECURITY_TYPE_X509;
    
    std::string hub_uri = argv[2];
    std::string device_id = argv[3];
    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();

    (void)iothub_security_init(security_type);

    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;
    if ((device_ll_handle = IoTHubDeviceClient_LL_CreateFromDeviceAuth(hub_uri.c_str(), device_id.c_str(), MQTT_Protocol)) == NULL)
    //if ((device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(conn_string, MQTT_Protocol)) == NULL)
    {
        LOG("Failure creating device Auth!\r\n");
        //result = __LINE__;
    }
    else
    {
        IOTHUB_CLIENT_SAMPLE_INFO iothub_info;
        TICK_COUNTER_HANDLE tick_counter_handle = tickcounter_create();
        tickcounter_ms_t current_tick;
        tickcounter_ms_t last_send_time = 0;
        size_t msg_count = 0;
        iothub_info.stop_running = 0;
        iothub_info.connected = 0;

        (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, iothub_connection_status, &iothub_info);
        IoTHubDeviceClient_LL_SetDeviceTwinCallback(device_ll_handle, DeviceTwinHandler, nullptr);
        // Set any option that are neccessary.
        // For available options please see the iothub_sdk_options.md documentation
        bool traceOn = true;
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_LOG_TRACE, &traceOn);

        (void)IoTHubDeviceClient_LL_SetMessageCallback(device_ll_handle, receive_msg_callback, &iothub_info);

        (void)printf("Sending 1 messages to IoTHub every %d seconds for %d messages (Send any message to stop)\r\n", TIME_BETWEEN_MESSAGES, MESSAGES_TO_SEND);
        do
        {
            if (iothub_info.connected != 0)
            {
                // Send a message every TIME_BETWEEN_MESSAGES seconds
                (void)tickcounter_get_current_ms(tick_counter_handle, &current_tick);
                if ((current_tick - last_send_time) / 1000 > TIME_BETWEEN_MESSAGES)
                {
                    static char msgText[1024];
                    sprintf_s(msgText, sizeof(msgText), "{ \"message_index\" : \"%zu\" }", msg_count++);

                    IOTHUB_MESSAGE_HANDLE msg_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText));
                    if (msg_handle == NULL)
                    {
                        (void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
                    }
                    else
                    {
                        if (IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, msg_handle, NULL, NULL) != IOTHUB_CLIENT_OK)
                        {
                            (void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
                        }
                        else
                        {
                            (void)tickcounter_get_current_ms(tick_counter_handle, &last_send_time);
                            (void)printf("IoTHubClient_LL_SendEventAsync accepted message [%zu] for transmission to IoT Hub.\r\n", msg_count);

                        }
                        IoTHubMessage_Destroy(msg_handle);
                    }
                }
            }
            IoTHubDeviceClient_LL_DoWork(device_ll_handle);
            ThreadAPI_Sleep(1);
        } while (iothub_info.stop_running == 0 && msg_count < MESSAGES_TO_SEND);

        size_t index = 0;
        for (index = 0; index < 10; index++)
        {
            IoTHubDeviceClient_LL_DoWork(device_ll_handle);
            ThreadAPI_Sleep(1);
        }
        tickcounter_destroy(tick_counter_handle);
        // Clean up the iothub sdk handle
        IoTHubDeviceClient_LL_Destroy(device_ll_handle);        
    }
    
    // Free all the sdk subsystem
    IoTHub_Deinit();

    (void)printf("Press any enter to continue:\r\n");
    (void)getchar();
}
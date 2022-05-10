#include "Device.h"

// Class to represent a Room object
class Room
{
public:
    FirebaseJsonData data;
    FirebaseJsonArray deviceArray;

    Device::Device **deviceList;
    String startAt;
    String endAt;

    String devicesPath;
    String startAtPath;
    String endAtPath;

    Room(FirebaseJson &roomJson)
    {
        devicesPath = "/devices/";
        startAtPath = "/startAt/";
        endAtPath = "/endAt/";

        roomJson.get(data, devicesPath);
        data.getArray(deviceArray);
        deviceList = new Device::Device *[deviceArray.size()];

        for (int i = 0; i < deviceArray.size(); i++)
        {
            deviceArray.get(data, i);

            FirebaseJson deviceJson;
            data.getJSON(deviceJson);

            deviceJson.get(data, "type");

            if (data.to<int>() == Device::DeviceType::Type_LED)
            {
                deviceList[i] = new Device::LED(deviceJson);
            }
            else if (data.to<int>() == Device::DeviceType::Type_HeatSensor)
            {
                deviceList[i] = new Device::HeatSensor(deviceJson);
            }
            else if (data.to<int>() == Device::DeviceType::Type_Fan)
            {
                deviceList[i] = new Device::Fan(deviceJson);
            }
        }
    }

    void update(FirebaseJson &roomJson)
    {
        roomJson.get(data, startAtPath);
        startAt = data.to<String>();

        roomJson.get(data, endAtPath);
        endAt = data.to<String>();

        roomJson.get(data, devicesPath);
        data.getArray(deviceArray);

        for (int i = 0; i < deviceArray.size(); i++)
        {
            deviceArray.get(data, i);
            FirebaseJson deviceJson;
            data.getJSON(deviceJson);

            deviceList[i]->update(deviceJson);
        }
    }

    void turnOnAllDevices(FirebaseJson &roomJson, FirebaseData &fbdo, String updatePath)
    {
        for (int i = 0; i < deviceArray.size(); i++)
        {
            deviceList[i]->turnOn();
        }
    }

    void turnOffAllDevices(FirebaseJson &roomJson, FirebaseData &fbdo, String updatePath)
    {
        for (int i = 0; i < deviceArray.size(); i++)
        {
            deviceList[i]->turnOff();
        }
    }

    void run()
    {
        for (int i = 0; i < deviceArray.size(); i++)
        {
            
            if (deviceList[i]->type == Device::DeviceType::Type_Fan)
            {
                deviceList[i]->smartToggleFunc(deviceList);
            }
            deviceList[i]->run();
            
        }
    }

    void updateToServer(FirebaseData &fbdo, String updatePath)
    {
        for (int i = 0; i < deviceArray.size(); i++)
        {
            deviceArray.get(data, i);
            FirebaseJson deviceJson;
            data.getJSON(deviceJson);

            deviceList[i]->updateToServer(deviceJson, fbdo, updatePath + devicesPath + String(i));
        }
    }
};
#include <OneWire.h>
#include <DallasTemperature.h>

namespace Device
{
    enum DeviceType
    {
        Type_LED = 1,
        Type_HeatSensor = 2,
        Type_Fan = 3
    };

    class Device
    {
    public:
        FirebaseJsonData data;

        int GPIO;
        int state;
        DeviceType type;

        String GPIOPath;
        String statePath;

        Device(FirebaseJson &deviceJson)
        {
            GPIOPath = "/GPIO";
            statePath = "/state";

            deviceJson.get(data, GPIOPath);
            GPIO = data.to<int>();

            pinMode(GPIO, OUTPUT);

            deviceJson.get(data, statePath);
            state = data.to<int>();
        }

        void turnOn()
        {
            digitalWrite(GPIO, HIGH);
            state = 1;
        }

        void turnOff()
        {
            digitalWrite(GPIO, LOW);
            state = 0;
        }

        virtual void update(FirebaseJson &deviceJson)
        {
            deviceJson.get(data, statePath);
            state = data.to<int>();
        }

        virtual void run()
        {
            if (state == 1)
            {
                turnOn();
            }
            else if (state == 0)
            {
                turnOff();
            }
        }

        virtual void updateToServer(FirebaseJson &deviceJson, FirebaseData &fbdo, String updatePath)
        {
            deviceJson.get(data, statePath);
            if (data.to<int>() != state)
            {
                Firebase.setIntAsync(fbdo, updatePath + statePath, state);
            }
        }

        virtual float getSensorData()
        {
        }

        virtual void smartToggleFunc(Device **deviceList)
        {
        }
    };

    class LED : public Device
    {
    public:
        LED(FirebaseJson &deviceJson) : Device(deviceJson)
        {
            type = DeviceType::Type_LED;
        }
    };

    class HeatSensor : public Device
    {
    public:
        OneWire *oneWire;
        DallasTemperature *sensors;

        float temperature;

        String dataPath;

        HeatSensor(FirebaseJson &deviceJson) : Device(deviceJson)
        {
            dataPath = "/data";

            oneWire = new OneWire(15);
            sensors = new DallasTemperature(oneWire);
            type = DeviceType::Type_HeatSensor;

            sensors->begin();
        }

        void run()
        {
            Device::run();
            if (state == 1)
            {
                readTemperature();
            }
        }

        void readTemperature()
        {
            sensors->requestTemperatures();
            Serial.print("Temperature is: ");
            int temp = sensors->getTempCByIndex(0) * 100;
            temperature = ((float)temp) / 100;
            Serial.println(temperature, 2);
        }

        void updateToServer(FirebaseJson &deviceJson, FirebaseData &fbdo, String updatePath)
        {
            Device::updateToServer(deviceJson, fbdo, updatePath);
            if (state == 1)
            {
                deviceJson.get(data, dataPath + "/temperature");
                if (data.to<float>() != temperature)
                {
                    Firebase.setFloatAsync(fbdo, updatePath + dataPath + "/temperature", temperature);
                }
            }
        }

        float getSensorData()
        {
            return temperature;
        }
    };

    class Fan : public Device
    {
    public:
        int smartToggle;
        int sensorIndex;
        float value;
        bool changeState;

        Fan(FirebaseJson &deviceJson) : Device(deviceJson)
        {
            deviceJson.get(data, "smartToggle");
            smartToggle = data.to<int>();

            deviceJson.get(data, "smartToggleConfig/sensorIndex");
            sensorIndex = data.to<int>();

            deviceJson.get(data, "smartToggleConfig/value");
            value = data.to<float>();

            deviceJson.get(data, "smartToggleConfig/changeState");
            changeState = data.to<bool>();

            type = DeviceType::Type_Fan;
        }

        void update(FirebaseJson &deviceJson)
        {
            Device::update(deviceJson);
            deviceJson.get(data, "smartToggle");
            smartToggle = data.to<int>();
        }

        void smartToggleFunc(Device **deviceList)
        {
            if (smartToggle == 1)
            {
                if (value <= deviceList[sensorIndex]->getSensorData())
                {
                    state = changeState;
                }
                else
                {
                    state = !changeState;
                }
            }
        }
    };
}

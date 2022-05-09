#include "RoomManager.h"

class RootManager
{
public:
    FirebaseData fbdo;
    FirebaseData fbdoUpdate;

    FirebaseJson rootJson;
    FirebaseJsonData data;

    RoomManager *roomManager;
    bool change;

    RootManager()
    {
        fbdo.setBSSLBufferSize(2048, 5120);

        if (Firebase.getJSON(fbdo, "/"))
        {
            rootJson = fbdo.jsonObject();
            roomManager = new RoomManager(rootJson);
        }
        else
        {
            Serial.println(fbdo.errorReason().c_str());
        }

        // Begin streaming for real time update
        if (!Firebase.beginStream(fbdoUpdate, "/"))
        {
            Serial.println(fbdo.errorReason());
        }
    }

    void run()
    {
        updateFromServer(rootJson);

        if (change == true)
        {
            roomManager->update(rootJson);
            roomManager->run(rootJson, fbdo);
            change = false;
        }
    }

    // Get realtime update from Server
    void updateFromServer(FirebaseJson &rootJson)
    {
        while (1)
        {
            if (!Firebase.readStream(fbdoUpdate))
            {
                Serial.println(fbdoUpdate.errorReason());
            }

            if (fbdoUpdate.streamTimeout())
            {
                Serial.println("Stream timeout, resume streaming...");
                Serial.println();
            }

            if (fbdoUpdate.streamAvailable())
            {
                String dataPath = formatDataPath(fbdoUpdate.dataPath());
                /* Serial.println(dataPath); */
                change = true;

                if (dataPath == "/")
                {
                    // If dont know what data have changed then reload whole database
                    reloadDatabase();
                }
                else if (fbdoUpdate.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
                {
                    rootJson.set(dataPath, fbdoUpdate.to<int>());
                }
                else if (fbdoUpdate.dataTypeEnum() == fb_esp_rtdb_data_type_string)
                {
                    rootJson.set(dataPath, fbdoUpdate.to<String>());
                }
            }
            else
            {
                break;
            }
        }
    }

    void reloadDatabase()
    {
        if (Firebase.getJSON(fbdo, "/"))
        {
            rootJson = fbdo.jsonObject();
        }
        else
        {
            Serial.println(fbdo.errorReason().c_str());
        }
    }

    // Format data path to work with FirebaseJson (ex: ../0/.. -> ../[0]/..)
    static String formatDataPath(String dataPath)
    {
        String result = "";
        String token = "";
        for (int i = 0; i <= dataPath.length(); i++)
        {
            if (dataPath.charAt(i) == '/' || i == dataPath.length())
            {
                bool isNumber = true;
                for (int j = 0; j < token.length(); j++)
                {
                    if (!isDigit(token.charAt(j)))
                    {
                        isNumber = false;
                        break;
                    }
                }
                if (token != "")
                {
                    if (isNumber)
                    {
                        result += "[" + String(token) + "]";
                    }
                    else
                    {
                        result += String(token);
                    }
                }
                if (i != dataPath.length())
                {
                    result += "/";
                }
                token = "";
                continue;
            }
            token += dataPath.charAt(i);
        }
        return result;
    }
};

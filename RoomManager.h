#include "Room.h"

// Class to manage a list of Room objects
class RoomManager
{
public:
    FirebaseJsonData data;
    FirebaseJsonArray roomArray;

    Room **roomList;
    bool *open; // An array to keep track of which room is opened
    String serverTime;

    String roomsPath;

    RoomManager(FirebaseJson &rootJson)
    {
        roomsPath = "/rooms/";

        rootJson.get(data, roomsPath);
        data.getArray(roomArray);

        open = new bool[roomArray.size()];
        roomList = new Room *[roomArray.size()];

        for (int i = 0; i < roomArray.size(); i++)
        {
            roomArray.get(data, i);

            FirebaseJson roomJson;
            data.getJSON(roomJson);

            roomList[i] = new Room(roomJson);
            open[i] = false;
        }
    }

    void update(FirebaseJson &rootJson)
    {
        rootJson.get(data, "serverTime");
        serverTime = data.to<String>();

        rootJson.get(data, roomsPath);
        data.getArray(roomArray);

        for (int i = 0; i < roomArray.size(); i++)
        {
            roomArray.get(data, i);

            FirebaseJson roomJson;
            data.getJSON(roomJson);

            roomList[i]->update(roomJson);
        }
    }

    void run(FirebaseJson &rootJson, FirebaseData &fbdo)
    {
        for (int i = 0; i < roomArray.size(); i++)
        {
            roomArray.get(data, i);

            FirebaseJson roomJson;
            data.getJSON(roomJson);

            String startAt = roomList[i]->startAt;
            String endAt = roomList[i]->endAt;

            if (compareTime(startAt, serverTime) >= 0 && compareTime(endAt, serverTime) < 0 && open[i] == false)
            {
                roomList[i]->turnOnAllDevices(roomJson, fbdo, roomsPath + String(i));
                open[i] = true;
            }
            else if ((compareTime(startAt, serverTime) < 0 || compareTime(endAt, serverTime) >= 0) && open[i] == true)
            {
                roomList[i]->turnOffAllDevices(roomJson, fbdo, roomsPath + String(i));
                open[i] = false;
            }
            else
            {
                roomList[i]->run();
            }

            roomList[i]->updateToServer(fbdo, roomsPath + String(i));
        }
    }

    static int compareTime(String &roomTime, String &serverTime)
    {
        int roomHour, roomMinute, roomSecond;
        sscanf(roomTime.c_str(), "%d:%d:%d", &roomHour, &roomMinute, &roomSecond);
        int roomTotalSecond = roomHour * 60 * 60 + roomMinute * 60 + roomSecond;

        int serverHour, serverMinute, serverSecond;
        sscanf(serverTime.c_str(), "%d:%d:%d", &serverHour, &serverMinute, &serverSecond);
        int serverTotalSecond = serverHour * 60 * 60 + serverMinute * 60 + serverSecond;

        return serverTotalSecond - roomTotalSecond;
    }
};
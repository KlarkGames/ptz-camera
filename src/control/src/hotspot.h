#ifndef HOTSPOT_H
#define HOTSPOT_H

class Hotspot
{
public:
    Hotspot();

    bool start(const char* ssid, const char* password);
    bool stop();

    ~Hotspot();

private:
    void *m_client = 0;
    void *m_hotspot = 0;
};

#endif // HOTSPOT_H

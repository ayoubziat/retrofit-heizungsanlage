

struct dataPoint{
  uint32_t value;
  uint16_t address;
  uint8_t length;
  int16_t factor;
  String name;
};


class Optolink {
  public:
    bool print = false;
    unsigned long lastMillis{0};
    uint8_t rxMsgLen{0};
    uint8_t datapointIndex{0};
    HardwareSerial* stream{nullptr};
    struct dataPoint datapoints[10] = {
        {0, 0x00F8, 0x02, -1, "Gerätekennung"}, 
        {0, 0x0802, 0x02, 10, "03_Kesseltemp_(S3)"},
        {0, 0x5502, 0x02, 10, "17_Kesseltemp_Soll"},
        {0, 0x2301, 0x01, -1, "19_Betriebsart" },
        {0, 0x2302, 0x01, -1, "20_Sparbetrieb" },
        {0, 0x2303, 0x01, -1, "21_Partybetrieb"},
        {0, 0x551E, 0x01, -1, "26_Brennerstatus 0..2"},
        {0, 0x2906, 0x01, -1, "32_Heizkreispumpe_A1M1"},
        {0, 0x08A7, 0x04, -1, "Betriebsstunden Stufe 1"},
        {0, 0x2308, 0x01, -1, "Party Temperatur Soll"},
    };
    void clearInputStream(void);
    void debugPrinter(void);
  };
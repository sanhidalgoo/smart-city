#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

class LightSensors {
  private:
    int pin1, pin2;
    
    unsigned long criticalTime;
    unsigned long initTrafficTime;
    
    bool isLate;

  public:
    LightSensors(int p1, int p2, unsigned long criticTime);
    void start();
    bool isDark();
    
};

#endif

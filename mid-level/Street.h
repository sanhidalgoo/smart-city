#ifndef STREET_H
#define STREET_H

class Street {
  private:
    int pin1, pin2, pin3;

    unsigned long criticalTime;
    unsigned long initTrafficTime;

    bool traffic;

  public:
    Street(int p1, int p2, int p3, unsigned long criticTime);
    void reset();
    bool hasCriticalTraffic();

};

#endif
